/*
 * main.c
 */
#include "FreeRTOS.h"
#include "task.h"

#include "hal_lpm.h"
#include "hal_board_type.h"
#include "hal_calibration.h"
#include "hal_miscellaneous.h"
#include "hal_rtc.h"
#include "hal_accelerometer.h"
#include "hal_vibe.h"
#include "hal_lcd.h"
#include "hal_dma.h"

#include "bttask.h"

#include "System.h"
#include "Adc.h"
#include "Battery.h"

#include <stdio.h>

#include "colindebug.h"

void* malloc(size_t _size)
{
	return pvPortMalloc(_size);
}

void free(void *p)
{
	vPortFree(p);
}

static void ConfigureHardware(void)
{
  SetAllPinsToOutputs();

  SetupClockAndPowerManagementModule();

  SetupAclkToRadio();

  BLUETOOTH_SIDEBAND_CONFIG();

  DISABLE_LCD_LED();

  CONFIG_SRAM_PINS();
  APPLE_CONFIG();

  /* the accelerometer may not be used so configure its pins here */
  CONFIG_ACCELEROMETER_PINS();
  CONFIG_ACCELEROMETER_PINS_FOR_SLEEP();

}
unsigned short reason;
void main(void)
{
	/* Turn off the watchdog timer */
	WDTCTL = WDTPW + WDTHOLD;

	/* clear reason for reset */
	reason = SYSRSTIV;
	SYSRSTIV = 0;

	/* disable DMA during read-modify-write cycles */
	DMACTL4 = DMARMWDIS;

	InitializeCalibrationData();

	ConfigureHardware();

	InitialiseDMA();
	InitializeRealTimeClock();
	Adc_Initialise();
	Battery_Initialise();
	InitAccelerometerPeripheral();
	SetupVibrationMotorTimerAndPwm();
	InitialiseBluetooth();
	StartLcd();

	InitialiseSystem();
	// TODO: Initiate tasks (actual interface)
	goodprintf("Startup: %i bytes free, restart reason %04x\n", (int)xPortGetFreeHeapSize(), (int)reason);
#ifdef _COLIN_DEBUG
	ENABLE_LCD_LED();
#endif
	vTaskStartScheduler();
}

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	goodprintf("Task '%s' stack overflow\n", pcTaskName);
}

void vApplicationMallocFailedHook(size_t xWantedSize)
{
	goodprintf("Malloc failed for %i bytes\n", xWantedSize);
}

void vApplicationIdleHook(void)
{
	if (!vTasksReady())
	{
	    /* Call MSP430 Utility function to enable low power mode 3.     */
		/* Put OS and Processor to sleep. Will need an interrupt        */
		/* to wake us up from here.   */
#ifdef _COLIN_DEBUG
		DISABLE_LCD_LED();
#endif
		MSP430_LPM_ENTER();
#ifdef _COLIN_DEBUG
		ENABLE_LCD_LED();
#endif
	}
}
