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
/*! \file hal_rtc.h
 *
 * Real Time Clock (RTC)
 *
 * This module contains more than the hardware abstraction. The real time clock
 * is always running off of the 32.768 kHz crystal.  The micro contains a 
 * calendar function that keeps track of month, date, and year.
 */
/******************************************************************************/

#ifndef HAL_RTC_H
#define HAL_RTC_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned int Year;
	unsigned char Month;
	unsigned char DayOfMonth;
	unsigned char DayOfWeek;
	unsigned char Hour, Minute, Second;
} RTC_DATA;

/*! Initialize the RTC for normal watch operation
 *
 * This function also sets up the static prescale one and 1ppS messages as well
 * as enable the 1 Hz RTC output on P2.4
 *
 * \note This function does not change the value of the RTC
 */
void InitializeRealTimeClock( void );

/*! Sets the RTC 
 *
 * \note There could be a separate struct to abstract this, however the message
 *  data was laid out to exactly match the MSP430 RTC registers.  The asm level
 *  patch functions may not be needed on newer versions of the MSP430
 *
 * \param pRtcData
 */
void halRtcSet(RTC_DATA* pRtcData);

// The exact value is 31.25 mS
#define RTC_TIMER_MS_PER_TICK       31   

// Number of RTC prescale interrupts per second
#define RTC_TICKS_PER_SECOND		128

void SetRTCPrescaleInterrupt(int enable);
int GetRTCPrescaleInterruptEnabled(void);

/*! Get the current structure containing the real time clock parameters.
 *
 * \param pRtcData
 *
 */
void halRtcGet(RTC_DATA* pRtcData);

// Compute day of week (0=Sunday)
int DayOfWeek(int Year, int Month, int Day);

/* TI fix */
extern int SetRTCYEAR(int year); 	
extern int SetRTCMON(int month);
extern int SetRTCDAY(int day);
extern int SetRTCDOW(int dow);
extern int SetRTCHOUR(int hour);
extern int SetRTCMIN(int min);
extern int SetRTCSEC(int sec);

extern int GetRTCTIM0(void); 	
extern int GetRTCTIM1(void); 	
extern int GetRTCDATE(void); 	
extern int GetRTCYEAR(void); 	

extern int GetRTCMON(void);
extern int GetRTCDOW(void);
extern int GetRTCDAY(void);
extern int GetRTCHOUR(void);
extern int GetRTCMIN(void);
extern int GetRTCSEC(void);

#ifdef __cplusplus
}
#endif
#endif /* HAL_RTC_H */
