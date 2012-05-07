#include "hal_board_type.h"
#include "hal_lcd.h"
#include "hal_clock_control.h"
#include "hal_dma.h"

#define SPI_PRESCALE_L   0x10
#define SPI_PRESCALE_H   0x00

unsigned short LcdDmaBusy;

void StartLcd(void)
{
	/* LCD 5.0 V SUPPLY */
	LCD_5V_PDIR |= LCD_5V_BIT;
	LCD_5V_POUT |= LCD_5V_BIT;

	CONFIG_LCD_PINS();

	/*
	 * configure the MSP430 SPI peripheral for use with Lcd
	 */

	/* Put state machine in reset while it is configured */
	LCD_SPI_UCBxCTL1 |= UCSWRST;

	/*
	 * 3-pin, 8-bit SPI master, Clock polarity low
	 * Clock phase set, least sig bit first
	 * SMCLK is the clock source
	 * set the clock prescaler
	 */
	LCD_SPI_UCBxCTL0 |= UCMST+ UCCKPH + UCSYNC;

	LCD_SPI_UCBxCTL1 |= UCSSEL_2;
	LCD_SPI_UCBxBR0 = SPI_PRESCALE_L;
	LCD_SPI_UCBxBR1 = SPI_PRESCALE_H;

	/* remove reset */
	LCD_SPI_UCBxCTL1 &= ~UCSWRST;

	LcdDmaBusy = 0;

	SetDMAHandler(2, LcdDMAComplete);
}

void WriteLcd(unsigned char* pData,unsigned int Size, int async)
{
  EnableSmClkUser(LCD_USER);
  LCD_CS_ASSERT();

  LcdDmaBusy = 1;

  /* USCIB0 TXIFG is the DMA trigger
   * DMACTL1 controls dma2 and [dma3]
   */
  DMACTL1 = DMA2TSEL_19;

  __data16_write_addr((unsigned short) &DMA2SA, (unsigned long)pData);
  __data16_write_addr((unsigned short) &DMA2DA, (unsigned long)&LCD_SPI_UCBxTXBUF);

  DMA2SZ = Size;

  /*
   * single transfer, increment source address, source byte and dest byte,
   * level sensitive, enable interrupt, clear interrupt flag
   */
  DMA2CTL = DMADT_0 + DMASRCINCR_3 + DMASBDB + DMALEVEL + DMAIE;

  /* start the transfer */
  DMA2CTL |= DMAEN;

  if (!async)
	  WaitForLcd();
}

void ClearLcd(int async)
{
	unsigned char command[LCD_CLEAR_CMD_SIZE] = {LCD_CLEAR_CMD, 0, 0};
	WriteLcd(command, sizeof(command), async);
}

void WaitForLcd(void)
{
	int enabled = (_get_SR_register() & 0x08) != 0;
	if (enabled)
	{
		while (LcdDmaBusy);
	}
	else
	{
		LcdDMAComplete(0);
	}
}

int LcdDMAComplete(int channel)
{
	/* wait for shift to complete ( ~3 us ) */
	while( (LCD_SPI_UCBxSTAT & 0x01) != 0 );

	/* now the chip select can be deasserted */
	LCD_CS_DEASSERT();
	DisableSmClkUser(LCD_USER);

	LcdDmaBusy = 0;

	return 0;
}
