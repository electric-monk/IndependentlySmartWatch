#include <FreeRTOS.h>
#include <task.h>
#include "hal_board_type.h"
#include "hal_clock_control.h"
#include "hal_lpm.h"
#include <btstack/hal_uart_dma.h>
#include "colindebug.h"

#define BUFFER_SIZE			100

#define UART_ENABLE_RX()	{BT_CTRL_POUT &= ~BT_RTS_PIN;}
#define UART_DISABLE_RX()	{BT_CTRL_POUT |= BT_RTS_PIN;}

static void DummyHandler(void)
{
}

static void (*rx_handler)(void) = DummyHandler;
static uint8_t *rx_buffer = NULL;
static uint16_t rx_length = 0;

static void (*tx_handler)(void) = DummyHandler;
static uint8_t *tx_buffer = NULL;
static uint16_t tx_length = 0;

static void (*cts_handler)(void) = DummyHandler;

void hal_uart_dma_init(void)
{
	// Configure Bluetooth SHUTDOWN pin (active low)
	BT_RST_PDIR |= BT_RST_PIN;
	BT_RST_POUT &= ~BT_RST_PIN;	// Low - stop chip

	// Configure port pins as UART I/O
	BT_COMM_SEL |= BT_TX_PIN | BT_RX_PIN;
	BT_COMM_PDIR |= BT_TX_PIN;
	BT_COMM_PDIR &= ~BT_RX_PIN;

	// Configure RS232 control lines
	BT_CTRL_SEL &= ~(BT_CTS_PIN | BT_RTS_PIN);
	BT_CTRL_PDIR |= BT_RTS_PIN;
	BT_CTRL_PDIR &= ~BT_CTS_PIN;
	BT_CTRL_POUT |= BT_RTS_PIN;	// High - not ready to receive

	// Enable ACLK (32kHz clock to the Bluetooth chip)
	P11SEL |= BIT0;
	P11DIR |= BIT0;

	BT_RST_POUT |= BT_RST_PIN;	// High - start chip

	// Wait a second
	vTaskDelay(1000 / portTICK_RATE_MS);

	// Configure UART
	BT_UART_CTL1 |= UCSWRST;	// Reset
	BT_UART_CTL0 = UCMODE_0;
	BT_UART_CTL0 &= ~UC7BIT;	// 8-bit characters
	BT_UART_CTL1 |= UCSSEL__SMCLK;
	BT_UART_CTL1 &= ~UCSWRST;	// Run UART

//	hal_uart_dma_set_baud(115200);
}

void hal_uart_dma_set_block_received( void (*block_handler)(void))
{
	if (block_handler == NULL)
		rx_handler = DummyHandler;
	else
		rx_handler = block_handler;
}

void hal_uart_dma_set_block_sent( void (*block_handler)(void))
{
	if (block_handler == NULL)
		tx_handler = DummyHandler;
	else
		tx_handler = block_handler;
}

void hal_uart_dma_set_csr_irq_handler( void (*cts_irq_handler)(void))
{
	BT_CTRL_IE &= ~BT_CTS_PIN;
	if (cts_irq_handler == NULL)
	{
		cts_handler = DummyHandler;
	}
	else
	{
		cts_handler = cts_irq_handler;
		BT_CTRL_IFG = 0;
		BT_CTRL_IV = 0;
		BT_CTRL_IES &= ~BT_CTS_PIN;	// IRQ on rising edge
		BT_CTRL_IE |= BT_CTS_PIN;	// Enable interrupt
	}
}

static const struct {
	uint32_t baud;
	unsigned short selector;
	unsigned char modulation;
} SupportedBauds[] = {
#if 1
		// 16MHz SMCLK
	{ 4000000,	2,   0 }, // wrong?
	{ 3000000,	3,   3 },
	{ 2400000,	6,   5 },
	{ 2000000,	8,   0 },
	{ 1000000,	16,  0 },
	{ 921600,	17,  7 },
	{ 115200,	138, 7 },
	{ 57600,	277, 7 },
#endif
#if 0
		// 8MHz SMCLK
	{ 1000000,	 8,  0 },
	{ 460800,	17,  3 },
	{ 230400,	34,  6 },
	{ 115200,	69,  4 },
	{ 57600,   138,  7 },
	{ 38400,   208,  3 },
	{ 19200,   416,  6 },
	{ 9600,    833,  2 },
#endif
};

int  hal_uart_dma_set_baud(uint32_t baud)
{
	int i, find;

	find = -1;
	for (i = 0; i < (sizeof(SupportedBauds) / sizeof(SupportedBauds[0])); i++)
	{
		if (SupportedBauds[i].baud == baud)
		{
			find = i;
			break;
		}
	}
	if (find == -1)
		return -1;

	BT_UART_CTL1 |= UCSWRST;	// Reset UART

	BT_UART_BR0 = (unsigned char)(SupportedBauds[find].selector & 0xFF);
	BT_UART_BR1 = (unsigned char)((SupportedBauds[find].selector & 0xFF00) >> 8);
	BT_UART_MCTL = SupportedBauds[find].modulation << 1;

	BT_UART_CTL1 &= ~UCSWRST;	// Resume UART

	return 0;
}

void hal_uart_dma_send_block(const uint8_t *buffer, uint16_t length)
{
	EnableSmClkUser(BT_UART_USER);

	BT_UART_IE &= ~UCTXIE;	// Disable TX interrupt

	tx_buffer = (uint8_t*)buffer;
	tx_length = length;

	BT_UART_IE |= UCTXIE;	// Enable TX interrupt
}

void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len)
{
	EnableSmClkUser(BT_UART_USER);

	BT_UART_IE &= ~UCRXIE;	// Disable RX interrupt

	rx_buffer = buffer;
	rx_length = len;

	BT_UART_IE |= UCRXIE;	// Enable RX interrupt

	UART_ENABLE_RX();
}

void hal_uart_dma_set_sleep(uint8_t sleep)
{
	if (sleep)
		DisableSmClkUser(BT_UART_USER);
	else
		EnableSmClkUser(BT_UART_USER);
}

#pragma vector=BT_UART_VECTOR
__interrupt void isrBluetoothUART(void)
{
	int exitLpm = 0;

	switch (BT_UART_IV)
	{
		case 2: // RX
			if (rx_length == 0)
			{	// Sanity check?
				UART_DISABLE_RX();
				BT_UART_IE &= ~UCRXIE;
				return;
			}
			*(rx_buffer++) = BT_UART_RXBUF;
			if ((--rx_length) == 0)
			{
				UART_DISABLE_RX();
				BT_UART_IE &= ~UCRXIE;
				rx_handler();
				exitLpm = 1;
			}
			break;

		case 4: // TX
			if (tx_length == 0)
			{	// Sanity check?
				BT_UART_IE &= ~UCTXIE;
				return;
			}
//			if (BT_CTRL_POUT & BT_CTS_PIN)
//			{
//				BT_UART_IE &= ~UCTXIE;
//				return;
//			}
			BT_UART_TXBUF = *(tx_buffer++);
			if ((--tx_length) == 0)
			{
				BT_UART_IE &= ~UCTXIE;
				tx_handler();
				exitLpm = 1;
			}
			break;

		default:
			break;
	}

	if (exitLpm)
		EXIT_LPM_ISR();
}

#pragma vector=BT_CTRL_VECTOR
__interrupt void isrBluetoothPort(void)
{
	BT_CTRL_IV = 0;
//	if (tx_length != 0)
//		BT_UART_IE |= UCTXIE;
	cts_handler();
	EXIT_LPM_ISR();
}
