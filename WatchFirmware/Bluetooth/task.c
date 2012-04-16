#include <FreeRTOS.h>
#include <task.h>
#include "bttask.h"
#include <btstack/run_loop.h>
#include <btstack/hci_cmds.h>
#include <btstack/sdp_util.h>
#include <string.h>

#include "btstack/config.h"
#include "btstack/bt_control_cc256x.h"
#include "btstack/hci.h"
#include "btstack/remote_device_db.h"
#include "btstack/btstack_memory.h"
#include "btstack/l2cap.h"
#include "btstack/sdp.h"
#include "btstack/rfcomm.h"

#include "colindebug.h"

#define TASK_STACK_DEPTH	   	(configMINIMAL_STACK_SIZE + 160)
#define TASK_PRIORITY   		(tskIDLE_PRIORITY + 1)

static xTaskHandle TaskHandle;

static void PacketHandler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
	switch (packet_type)
	{
		case HCI_EVENT_PACKET:
			switch (packet[0])
			{
				case BTSTACK_EVENT_STATE:
					if (packet[2] == HCI_STATE_WORKING)
					{
						goodprintf("Setting Bluetooth name!\n");
						hci_send_cmd(&hci_write_local_name, "FunkWatch 9000");
					}
					break;
				case HCI_EVENT_COMMAND_COMPLETE:
					if (COMMAND_COMPLETE_EVENT(packet, hci_write_local_name))
					{
						goodprintf("Doing something!\n");
						//hci_discoverable_control(1);
						hci_send_cmd(&hci_inquiry, HCI_INQUIRY_LAP, 15, 0);
					}
					break;
				case HCI_EVENT_INQUIRY_RESULT:
					goodprintf("Device type 0x%02x%02x @ %02x:%02x:%02x:%02x:%02x:%02x\n",
							packet[12], packet[13],
							packet[5], packet[4], packet[3], packet[2], packet[1], packet[0]);
					break;
				case HCI_EVENT_INQUIRY_COMPLETE:
					goodprintf("Enquiry complete.");
					break;
			}
			break;
	}
}

static uint8_t   sppServiceBuffer[100];

static void BluetoothTask(void *parameter)
{
	// Pre-initialise
	btstack_memory_init();
	run_loop_init(RUN_LOOP_EMBEDDED);

	// Start up
	hci_init(
				hci_transport_h4_dma_instance(),
				hci_uart_config_cc256x_instance(),
				bt_control_cc256x_instance(),
				&remote_device_db_memory);

    // use eHCILL
    bt_control_cc256x_enable_ehcill(1);

    // init L2CAP
    l2cap_init();
    l2cap_register_packet_handler(PacketHandler);

    // try RS232
    rfcomm_init();
    rfcomm_register_packet_handler(PacketHandler);
    rfcomm_register_service_internal(NULL, 1, 100);

    // try SDP
    sdp_init();
    memset(sppServiceBuffer, 0, sizeof(sppServiceBuffer));
    service_record_item_t *serviceRecordItem = (service_record_item_t*)sppServiceBuffer;
    sdp_create_spp_service( (uint8_t*) &serviceRecordItem->service_record, 1, "SPP Interface");
    sdp_register_service_internal(NULL, serviceRecordItem);

	SetBluetoothPower(1);

    // go!
    run_loop_execute();
}

void InitialiseBluetooth(void)
{
	xTaskCreate(BluetoothTask, "Bluetooth", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, &TaskHandle);
}

void SetBluetoothPower(int on)
{
	hci_power_control(on ? HCI_POWER_ON : HCI_POWER_OFF);
}
