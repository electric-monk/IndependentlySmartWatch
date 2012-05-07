//==============================================================================
//  Copyright 2011 Meta Watch Ltd. - http://www.MetaWatch.org/
// 
//  Licensed under the Meta Watch License, Version 1.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.MetaWatch.org/licenses/license-1.0.html
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//==============================================================================

/******************************************************************************/
/*! \file hal_lpm.c
 *
 */
/******************************************************************************/

#include "hal_board_type.h"
#include "hal_rtos_timer.h"
#include "hal_lpm.h"
#include "HAL_UCS.h"
#include "macro.h"
#include "portmacro.h"
#include "hal_clock_control.h"

unsigned short LPMcurrent = LPM3_bits;

unsigned short sleep = 0, wake = 0;

void MSP430_LPM_ENTER(void)
{
#ifdef LPM_ENABLED
    
  /* Turn off the watchdog timer */
  WDTCTL = WDTPW | WDTHOLD;

  /*
   * Enter a critical section to do so that we do not get switched out by the
   * OS in the middle of stopping the OS Scheduler.
   */
  __disable_interrupt();
  __no_operation();
  DisableRtosTick();

  // Select a nice mode
  int smclk = 1;//IsSmClkInUse();
  if (smclk)
  {
	  LPMcurrent = LPM0_bits;
  }
  else
  {
	  LPMcurrent = LPM3_bits;
  }
  MCLK_DIV(2);	// Errata PMM11

  sleep++;
  if (sleep == 5)
  {
	  __no_operation();
  }
  //__enable_interrupt();
  int state = LPMcurrent | GIE;
  _bis_SR_register(state);
  __no_operation();
  wake++;

//  if (!smclk)
  {
	  /* errata PMM11 - wait to put MCLK into normal mode */
	  __delay_cycles(100);
	  MCLK_DIV(1);
  }
  
  /* Generate a vTickIsr by setting the flag to trigger an interrupt
   * You can't call vTaskIncrementTick and vTaskSwitchContext from within a
   * task so do it with an ISR.  We need to cause an OS tick here so that tasks
   * blocked on an event sent by an ISR will run.  FreeRTOS queues them for
   * the next system tick.
   */
  EnableRtosTick();
  RTOS_TICK_SET_IFG();
  __no_operation();
  

#endif  
  
}



void SoftwareReset(void)
{
  /* let the uart drain */
  __delay_cycles(100000);
  
  PMMCTL0 = PMMPW | PMMSWBOR;
}

/******************************************************************************/

unsigned char nvRstNmiConfiguration;

unsigned char QueryRstPinEnabled(void)
{
  return ( nvRstNmiConfiguration == RST_PIN_ENABLED );  
}

void ConfigureResetPinFunction(unsigned char Control)
{
  switch (Control)
  {
  case RST_PIN_ENABLED:
    nvRstNmiConfiguration = RST_PIN_ENABLED;
    SFRRPCR |= SYSRSTRE;
    SFRRPCR &= ~SYSNMI;
    break;
    
  case RST_PIN_DISABLED:
  default:
    /* enable nmi functionality but don't enable the interrupt */
    nvRstNmiConfiguration = RST_PIN_DISABLED;
    SFRRPCR &= ~SYSRSTRE;
    SFRRPCR |= SYSNMI;
    break;
  
  }
}

void EnableRstPin(void)
{
  ConfigureResetPinFunction(RST_PIN_ENABLED);  
}

void DisableRstPin(void)
{
  ConfigureResetPinFunction(RST_PIN_DISABLED);  
}
