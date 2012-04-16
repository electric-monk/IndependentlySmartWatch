;
;/*
;    FreeRTOS V7.1.0 - Copyright (C) 2011 Real Time Engineers Ltd.
;	
;
;    ***************************************************************************
;     *                                                                       *
;     *    FreeRTOS tutorial books are available in pdf and paperback.        *
;     *    Complete, revised, and edited pdf reference manuals are also       *
;     *    available.                                                         *
;     *                                                                       *
;     *    Purchasing FreeRTOS documentation will not only help you, by       *
;     *    ensuring you get running as quickly as possible and with an        *
;     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
;     *    the FreeRTOS project to continue with its mission of providing     *
;     *    professional grade, cross platform, de facto standard solutions    *
;     *    for microcontrollers - completely free of charge!                  *
;     *                                                                       *
;     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
;     *                                                                       *
;     *    Thank you for using FreeRTOS, and thank you for your support!      *
;     *                                                                       *
;    ***************************************************************************
;
;
;    This file is part of the FreeRTOS distribution.
;
;    FreeRTOS is free software; you can redistribute it and/or modify it under
;    the terms of the GNU General Public License (version 2) as published by the
;    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
;    >>>NOTE<<< The modification to the GPL is included to allow you to
;    distribute a combined work that includes FreeRTOS without being obliged to
;    provide the source code for proprietary components outside of the FreeRTOS
;    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
;    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
;    more details. You should have received a copy of the GNU General Public
;    License and the FreeRTOS license exception along with FreeRTOS; if not it
;    can be viewed here: http://www.freertos.org/a00114.html and also obtained
;    by writing to Richard Barry, contact details for whom are available on the
;    FreeRTOS WEB site.
;
;    1 tab == 4 spaces!
;
;    http://www.FreeRTOS.org - Documentation, latest information, license and
;    contact details.
;
;    http://www.SafeRTOS.com - A version that is certified for use in safety
;    critical systems.
;
;    http://www.OpenRTOS.com - Commercial support, development, porting,
;    licensing and training services.
;*/

; * The definition of the "register test" tasks, as described at the top of
; * main.c

	.include data_model.h

	.cdecls C,LIST,"msp430f5438a.h"

	.global vTaskIncrementTick
	.global vTaskSwitchContext
	.global SetupRtosTimer
	.global pxCurrentTCB
	.global usCriticalNesting
	.global RtosTickEnabled
	.global RtosTickCount

	.global TA0CCR0
	.global TA0R

	.def vPortPreemptiveTickISR
;	.def vPortCooperativeTickISR
	.def vPortYield
	.def xPortStartScheduler

;-----------------------------------------------------------

portSAVE_CONTEXT .macro

	bicx.w  #0x00f0, 0(SP)	; Clear SP
	;Save the remaining registers.
	pushm_x	#12, r15
	mov.w	&usCriticalNesting, r14
	push_x r14
	mov_x	&pxCurrentTCB, r12
	mov_x	sp, 0( r12 )
	.endm
;-----------------------------------------------------------
		
portRESTORE_CONTEXT .macro

	mov_x	&pxCurrentTCB, r12
	mov_x	@r12, sp
	pop_x	r15
	mov.w	r15, &usCriticalNesting
	popm_x	#12, r15
	reti
	.endm
;-----------------------------------------------------------

;*
;* The RTOS tick ISR.
;*
;* If the cooperative scheduler is in use this simply increments the tick
;* count.
;*
;* If the preemptive scheduler is in use a context switch can also occur.
;*/
	
	.text
	.align 2
	
metaWatchSpecialTickISR: .asmfunc
	; The sr is not saved in portSAVE_CONTEXT() because vPortYield() needs
	;to save it manually before it gets modified (interrupts get disabled).
	;push.w sr
	tst.b &RtosTickEnabled
	jne	vPortPreemptiveTickISR
	;pop.w sr
	reti
	.endasmfunc

	.align 2

vPortPreemptiveTickISR: .asmfunc
	
	portSAVE_CONTEXT
				
	; More metawatch nonsense
	movx.w &TA0CCR0,&TA0R
	add.w &RtosTickCount,&TA0CCR0
	; End of nonsense

	call_x	#vTaskIncrementTick
	call_x	#vTaskSwitchContext
		
	portRESTORE_CONTEXT
	.endasmfunc
;-----------------------------------------------------------

;	.align 2
	
;vPortCooperativeTickISR: .asmfunc
	
	; The sr is not saved in portSAVE_CONTEXT() because vPortYield() needs
	;to save it manually before it gets modified (interrupts get disabled).
;	push.w sr
;	portSAVE_CONTEXT
				
;	call_x	#vTaskIncrementTick
		
;	portRESTORE_CONTEXT

;	.endasmfunc
;-----------------------------------------------------------

;
; Manual context switch called by the portYIELD() macro.
;

	.align 2

vPortYield: .asmfunc
	; Colin fix - Fake the interrupt
	.if $DEFINED( __LARGE_CODE_MODEL__ )
	push.w sr	; Temporarily
	dint
	nop
	bicx.w #0xFE00, 0(SP)	; Clear power mode flags
	swpbx.w 4(SP)
	rlax.w 4(SP)
	rlax.w 4(SP)
	rlax.w 4(SP)
	rlax.w 4(SP)
	bis.w 0(SP), 4(SP)
	incdx.a SP	; 'pop' saved SR
	.else
	push.w sr	; Placeholder for SR
	dint
	nop
	.endif
	pushx.a r15
	mov.w 6(SP), r15
	mov.w 4(SP), 6(SP)
	mov.w r15, 4(SP)
	popx.a r15
	portSAVE_CONTEXT

	; The sr needs saving before it is modified.
	;push.w	sr
	
	; Now the SR is stacked we can disable interrupts.
	;dint
	;nop
				
	; Save the context of the current task.
	;portSAVE_CONTEXT

	; Select the next task to run.
	call_x	#vTaskSwitchContext		

	; Restore the context of the new task.
	portRESTORE_CONTEXT
	.endasmfunc
;-----------------------------------------------------------


;
; Start off the scheduler by initialising the RTOS tick timer, then restoring
; the context of the first task.
;

	.align 2
	
xPortStartScheduler: .asmfunc

	; Setup the hardware to generate the tick.  Interrupts are disabled
	; when this function is called.
	call_x	#SetupRtosTimer

	; Restore the context of the first task that is going to run.
	portRESTORE_CONTEXT
	.endasmfunc
;-----------------------------------------------------------
      		
     .sect	TIMER0_A0_VECTOR
     .short	metaWatchSpecialTickISR

	.end
		
