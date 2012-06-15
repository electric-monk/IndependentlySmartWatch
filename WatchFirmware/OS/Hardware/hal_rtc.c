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
/*! \file hal_rtc.c
*
*/
/******************************************************************************/

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "hal_board_type.h"
#include "hal_rtc.h"
#include "hal_lpm.h"
#include "hal_calibration.h"
#include "macro.h"
#include "colindebug.h"

#include "Timer.h"

/** Real Time Clock interrupt Flag definitions */
#define RTC_NO_INTERRUPT      ( 0 )
#define RTC_RDY_IFG           ( 2 )
#define RTC_EV_IFG            ( 4 )
#define RTC_A_IFG             ( 6 )
#define RTC_PRESCALE_ZERO_IFG ( 8 )
#define RTC_PRESCALE_ONE_IFG  ( 10 )

#define RTCCAL_VALUE_MASK ( 0x3f )

void InitializeRealTimeClock( void )
{
	Timer_Initialise();

  // stop it
  RTCCTL01 = RTCHOLD;

  // use calibration data to adjust real time clock frequency
  if ( QueryCalibrationValid() )
  {
    signed char RtcCalibrationValue = GetRtcCalibrationValue();
    
    if ( RtcCalibrationValue < 0 )
    {
      RtcCalibrationValue = -1 * RtcCalibrationValue;
      RTCCTL2 = RtcCalibrationValue & RTCCAL_VALUE_MASK;
    }
    else
    {
      RTCCTL2 = RtcCalibrationValue & RTCCAL_VALUE_MASK;
      /* adjust up */
      RTCCTL2 |= RTCCALS;
    }
    
  }
  
  
  // Set the counter for RTC mode
  RTCCTL01 |= RTCMODE;

  // set 128 Hz rate for prescale 0 interrupt
  RTCPS0CTL |= RT0IP_7;

  // enable 1 pulse per second interrupt using prescale 1
  RTCPS1CTL |= RT1IP_6 | RT1PSIE;

  // 1 Hz calibration output
  RTCCTL23 |= RTCCALF_3;

  // setting the peripheral selection bit makes the other I/O control a don't care
  // P2.4 = 1 Hz RTC calibration output
  // Direction needs to be set as output
  RTC_1HZ_PORT_SEL |= RTC_1HZ_BIT;  
  RTC_1HZ_PORT_DIR |= RTC_1HZ_BIT;  

  // These calls are to asm level patch functions provided by 
  // TI for the MSP430F5438
  SetRTCYEAR((unsigned int)0x07db);
  SetRTCMON((unsigned int)5);
  SetRTCDAY((unsigned int)23);
  SetRTCDOW((unsigned int)5);
  SetRTCHOUR((unsigned int)23);
  SetRTCMIN((unsigned int)58);
  SetRTCSEC((unsigned int)0);


  // Enable the RTC
  RTCCTL01 &= ~RTCHOLD;  
}


typedef union
{
  struct
  {
    unsigned char b0;
    unsigned char b1;
  } byte;

  unsigned int word;

} tWordByteUnion;

void halRtcSet(RTC_DATA* pRtcData)
{
  // Stop the RTC
  RTCCTL01 |= RTCHOLD;    

  // These calls are to asm level patch functions provided by TI for the MSP430F5438
  tWordByteUnion temp;
  temp.byte.b0 = pRtcData->Year & 0xFF;
  temp.byte.b1 = (pRtcData->Year >> 8) & 0xFF;
  SetRTCYEAR((unsigned int)(temp.word));
  SetRTCMON((unsigned int)(pRtcData->Month));
  SetRTCDAY((unsigned int)(pRtcData->DayOfMonth));
  SetRTCDOW((unsigned int)(pRtcData->DayOfWeek));
  SetRTCHOUR((unsigned int)(pRtcData->Hour));
  SetRTCMIN((unsigned int)(pRtcData->Minute));
  SetRTCSEC((unsigned int)(pRtcData->Second));
  
  // Enable the RTC
  RTCCTL01 &= ~RTCHOLD;  
}

void halRtcGet(RTC_DATA* pRtcData)
{
  tWordByteUnion temp;
  temp.word = GetRTCYEAR();
  pRtcData->Year = temp.byte.b0 | (temp.byte.b1 << 8);
  pRtcData->Month = GetRTCMON();
  pRtcData->DayOfMonth = GetRTCDAY(); 
  pRtcData->DayOfWeek = GetRTCDOW();
  pRtcData->Hour = GetRTCHOUR();
  pRtcData->Minute = GetRTCMIN();
  pRtcData->Second = GetRTCSEC();

}

void SetRTCPrescaleInterrupt(int enable)
{
	ENTER_CRITICAL_REGION_QUICK();
	if (enable)
		RTCPS0CTL |= RT0PSIE;
	else
		RTCPS0CTL &= ~RT0PSIE;
	LEAVE_CRITICAL_REGION_QUICK();
}

int GetRTCPrescaleInterruptEnabled(void)
{
	return RTCPS0CTL & RT0PSIE;
}

#pragma vector = RTC_VECTOR
__interrupt void RTC_ISR(void)
{
	unsigned char ExitLpm = 0;

  // compiler intrinsic, value must be even, and in the range of 0 to 10
  switch(__even_in_range(RTCIV,0xa))
  {
  case RTC_NO_INTERRUPT: break;
  case RTC_RDY_IFG:      break;
  case RTC_EV_IFG:       break;
  case RTC_A_IFG:        break;

  case RTC_PRESCALE_ZERO_IFG:	// 128Hz
	  ExitLpm |= Timer_Interrupt(1);
    break;

  case RTC_PRESCALE_ONE_IFG:	// 1Hz
	  ExitLpm |= Timer_Interrupt(0);
    break;
  
  default:
    break;
  }

  if ( ExitLpm )
  {
    EXIT_LPM_ISR();
  }
}

// From Wikipedia
static const unsigned char t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

int DayOfWeek(int Year, int Month, int Day)
{
	if (Month < 3)
		Year--;
	return (Year + Year / 4 - Year / 100 + Year / 400 + t[Month - 1] + Day) % 7;
}
