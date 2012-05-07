#include "hal_board_type.h"
#include "colindebug.h"

#define EMPTY_ISR(isr)					\
	__interrupt void EMPTY_##isr##_ISR(void)	\
	{ ReportISR(isr); }

static void ReportISR(int id)
{
	goodprintf("Unhandled interrupt %i\n", id);
}

#pragma vector=PORT2_VECTOR
EMPTY_ISR(PORT2_VECTOR);
#pragma vector=USCI_B3_VECTOR
EMPTY_ISR(USCI_B3_VECTOR);
#pragma vector=USCI_A3_VECTOR
EMPTY_ISR(USCI_A3_VECTOR);
#pragma vector=USCI_B2_VECTOR
EMPTY_ISR(USCI_B2_VECTOR);
#pragma vector=USCI_A2_VECTOR
EMPTY_ISR(USCI_A2_VECTOR);
#pragma vector=ADC12_VECTOR
EMPTY_ISR(ADC12_VECTOR);
#pragma vector=USCI_B0_VECTOR
EMPTY_ISR(USCI_B0_VECTOR);
#pragma vector=USCI_A0_VECTOR
EMPTY_ISR(USCI_A0_VECTOR);
#pragma vector=WDT_VECTOR
EMPTY_ISR(WDT_VECTOR);
#pragma vector=TIMER0_B1_VECTOR
EMPTY_ISR(TIMER0_B1_VECTOR);
#pragma vector=TIMER0_B0_VECTOR
EMPTY_ISR(TIMER0_B0_VECTOR);
#pragma vector=UNMI_VECTOR
EMPTY_ISR(UNMI_VECTOR);
#pragma vector=SYSNMI_VECTOR
EMPTY_ISR(SYSNMI_VECTOR);
/*
#pragma vector=(40 * 1u)
EMPTY_ISR(40);
#pragma vector=(39 * 1u)
EMPTY_ISR(39);
#pragma vector=(38 * 1u)
EMPTY_ISR(38);
#pragma vector=(37 * 1u)
EMPTY_ISR(37);
#pragma vector=(36 * 1u)
EMPTY_ISR(36);
#pragma vector=(35 * 1u)
EMPTY_ISR(35);
#pragma vector=(34 * 1u)
EMPTY_ISR(34);
#pragma vector=(33 * 1u)
EMPTY_ISR(33);
#pragma vector=(32 * 1u)
EMPTY_ISR(32);
#pragma vector=(31 * 1u)
EMPTY_ISR(31);
#pragma vector=(30 * 1u)
EMPTY_ISR(30);
#pragma vector=(29 * 1u)
EMPTY_ISR(29);
#pragma vector=(28 * 1u)
EMPTY_ISR(28);
#pragma vector=(27 * 1u)
EMPTY_ISR(27);
#pragma vector=(26 * 1u)
EMPTY_ISR(26);
#pragma vector=(25 * 1u)
EMPTY_ISR(25);
#pragma vector=(24 * 1u)
EMPTY_ISR(24);
#pragma vector=(23 * 1u)
EMPTY_ISR(23);
#pragma vector=(22 * 1u)
EMPTY_ISR(22);
#pragma vector=(21 * 1u)
EMPTY_ISR(21);
#pragma vector=(20 * 1u)
EMPTY_ISR(20);
#pragma vector=(19 * 1u)
EMPTY_ISR(19);
#pragma vector=(18 * 1u)
EMPTY_ISR(18);
#pragma vector=(17 * 1u)
EMPTY_ISR(17);
#pragma vector=(16 * 1u)
EMPTY_ISR(16);
#pragma vector=(15 * 1u)
EMPTY_ISR(15);
#pragma vector=(14 * 1u)
EMPTY_ISR(14);
#pragma vector=(13 * 1u)
EMPTY_ISR(13);
#pragma vector=(12 * 1u)
EMPTY_ISR(12);
#pragma vector=(11 * 1u)
EMPTY_ISR(11);
#pragma vector=(10 * 1u)
EMPTY_ISR(10);
#pragma vector=(9 * 1u)
EMPTY_ISR(9);
#pragma vector=(8 * 1u)
EMPTY_ISR(8);
#pragma vector=(7 * 1u)
EMPTY_ISR(7);
#pragma vector=(6 * 1u)
EMPTY_ISR(6);
#pragma vector=(5 * 1u)
EMPTY_ISR(5);
#pragma vector=(4 * 1u)
EMPTY_ISR(4);
#pragma vector=(3 * 1u)
EMPTY_ISR(3);
#pragma vector=(2 * 1u)
EMPTY_ISR(2);
#pragma vector=(1 * 1u)
EMPTY_ISR(1);
#pragma vector=(0 * 1u)
EMPTY_ISR(0);
*/
