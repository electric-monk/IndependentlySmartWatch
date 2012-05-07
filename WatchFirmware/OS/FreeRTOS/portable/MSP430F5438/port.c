/*
    FreeRTOS V7.1.0 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the MSP430X port.
 *----------------------------------------------------------*/

/* Constants required for hardware setup.  The tick ISR runs off the ACLK,
not the MCLK. */
#define portACLK_FREQUENCY_HZ			( ( portTickType ) 32768 )
#define portINITIAL_CRITICAL_NESTING	( ( unsigned short ) 10 )
#define portFLAGS_INT_ENABLED			( ( portSTACK_TYPE ) 0x08 )

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;

/* Each task maintains a count of the critical section nesting depth.  Each
time a critical section is entered the count is incremented.  Each time a
critical section is exited the count is decremented - with interrupts only
being re-enabled if the count is zero.

usCriticalNesting will get set to zero when the scheduler starts, but must
not be initialised to zero as this will cause problems during the startup
sequence. */
volatile portSTACK_TYPE usCriticalNesting = portINITIAL_CRITICAL_NESTING;
/*-----------------------------------------------------------*/


/*
 * Sets up the periodic ISR used for the RTOS tick.  This uses timer 0, but
 * could have alternatively used the watchdog timer or timer 1.
 */
//void vPortSetupTimerInterrupt( void );
/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See the header file portable.h.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
unsigned short *pusTopOfStack;

	/*
		Place a few bytes of known values on the bottom of the stack.
		This is just useful for debugging and can be included if required.

	*/
		*pxTopOfStack = ( portSTACK_TYPE ) 0x11111111;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0x22222222;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0x33333333;
		pxTopOfStack--;

	/* Data types are need either 16 bits or 32 bits depending on the data 
	and code model used. */
	pusTopOfStack = ( unsigned short * ) pxTopOfStack;
	if( sizeof( pxCode ) == sizeof( unsigned short ) )
	{
//		*pusTopOfStack = ( unsigned short ) pxCode;
		*(pusTopOfStack--) = ( unsigned short ) (unsigned long)pxCode;
		*(pusTopOfStack--) = portFLAGS_INT_ENABLED;
	}
	else
	{
		/* Make room for a 20 bit value stored as a 32 bit value. */
		*(pusTopOfStack--) = (unsigned short)(((unsigned long)pxCode) & 0xFFFF);
		*(pusTopOfStack--) = (unsigned short)(portFLAGS_INT_ENABLED & 0x01FF) | ((((unsigned long)pxCode) & 0xF0000) >> 4);
		pusTopOfStack--;	// Going to be 32-bit...
		//pulTopOfStack = ( unsigned long * ) pusTopOfStack;
		//*pulTopOfStack = ( unsigned long ) pxCode;
	}

//	pusTopOfStack--;
//	*pusTopOfStack = portFLAGS_INT_ENABLED;
//	pusTopOfStack -= ( sizeof( portSTACK_TYPE ) / 2 );
	
	/* From here on the size of stacked items depends on the memory model. */
	pxTopOfStack = ( portSTACK_TYPE * ) pusTopOfStack;

	/* Next the general purpose registers. */
	#ifdef PRELOAD_REGISTER_VALUES
		*pxTopOfStack = ( portSTACK_TYPE ) 0xffffffff;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0xeeeeeeee;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0xdddddddd;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) pvParameters;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0xbbbbbbbb;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0xaaaaaaaa;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0x99999999;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0x88888888;
		pxTopOfStack--;	
		*pxTopOfStack = ( portSTACK_TYPE ) 0x55555555;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0x66666666;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0x55555555;
		pxTopOfStack--;
		*pxTopOfStack = ( portSTACK_TYPE ) 0x44444444;
		pxTopOfStack--;
	#else
		pxTopOfStack -= 3;
		*pxTopOfStack = ( portSTACK_TYPE ) pvParameters;
		pxTopOfStack -= 9;
	#endif

	/* A variable is used to keep track of the critical section nesting.
	This variable has to be stored as part of the task context and is
	initially set to zero. */
	*(pxTopOfStack--) = ( portSTACK_TYPE ) portNO_CRITICAL_SECTION_NESTING;

	/* Return a pointer to the top of the stack we have generated so this can
	be stored in the task control block for the task. */
	return pxTopOfStack + 1;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the MSP430 port will get stopped.  If required simply
	disable the tick interrupt here. */
}
/*-----------------------------------------------------------*/
