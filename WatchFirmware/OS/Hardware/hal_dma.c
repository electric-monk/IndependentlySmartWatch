#include <stdio.h>
#include <FreeRTOS.h>
#include "hal_board_type.h"
#include "hal_dma.h"
#include "hal_lpm.h"
#include "macro.h"
#include "colindebug.h"

static DMACallback dmaCallbacks[3];

static int DummyCallback(int channel)
{
	return 0; // Don't wake up
}

void InitialiseDMA(void)
{
	dmaCallbacks[0] = DummyCallback;
	dmaCallbacks[1] = DummyCallback;
	dmaCallbacks[2] = DummyCallback;
}

void SetDMAHandler(int channel, DMACallback callback)
{
	ENTER_CRITICAL_REGION_QUICK();
	if (callback == NULL)
		dmaCallbacks[channel] = DummyCallback;
	else
		dmaCallbacks[channel] = callback;
	LEAVE_CRITICAL_REGION_QUICK();
}

#pragma vector=DMA_VECTOR
__interrupt void isrDMA(void)
{
	int index;
	switch (__even_in_range(DMAIV, 16))
	{
		case DMAIV_DMA0IFG:
			index = 0;
			break;
		case DMAIV_DMA1IFG:
			index = 1;
			break;
		case DMAIV_DMA2IFG:
			index = 2;
			break;
		default:
			return;	// Don't continue
	}
	if (dmaCallbacks[index](index))
	{
		EXIT_LPM_ISR();
	}
}
