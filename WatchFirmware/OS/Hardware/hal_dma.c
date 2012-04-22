#include <stdio.h>
#include <FreeRTOS.h>
#include "hal_board_type.h"
#include "hal_dma.h"

static DMACallback dmaCallbacks[3];

static void DummyCallback(int channel)
{
	// Do nothing
}

void InitialiseDMA(void)
{
	dmaCallbacks[0] = DummyCallback;
	dmaCallbacks[1] = DummyCallback;
	dmaCallbacks[2] = DummyCallback;
}

void SetDMAHandler(int channel, DMACallback callback)
{
	portDISABLE_INTERRUPTS();
	if (callback == NULL)
		dmaCallbacks[channel] = DummyCallback;
	else
		dmaCallbacks[channel] = callback;
	portENABLE_INTERRUPTS();
}

#pragma vector=DMA_VECTOR
__interrupt void isrDMA(void)
{
	switch (__even_in_range(DMAIV, 16))
	{
		case DMAIV_DMA0IFG:
			dmaCallbacks[0](0);
			break;
		case DMAIV_DMA1IFG:
			dmaCallbacks[1](1);
			break;
		case DMAIV_DMA2IFG:
			dmaCallbacks[2](2);
			break;
		default:
			break;
	}
}
