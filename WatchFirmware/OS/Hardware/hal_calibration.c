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
/*! \file hal_calibration.c
 *
 */
/******************************************************************************/

#include "hal_board_type.h"
#include "hal_calibration.h"

/*! Calibration data structure
 *
 * \note This must match what is done in the Calibration firmware
 *
 * \param FlashRevision 
 * \param batteryCal Valid range is 0-254
 * \param xtalCap Valid range is 0 to 3
 * \param rtcCal Valid range is +/-63 
 */
struct stCalibrationData {
  unsigned int FlashRevision;
  
  unsigned char batteryCal;
  
  unsigned char xtalCap;           
  
  signed char rtcCal;            

} __attribute__((__packed__));

typedef struct stCalibrationData tCalibrationData;

/*! Location of this value is in information flash */
static const tCalibrationData CalibrationData;
#pragma DATA_SECTION(CalibrationData, ".infoA")

static unsigned char ValidCalibration;

void InitializeCalibrationData(void)
{
  ValidCalibration = 1;
  
  if (   CalibrationData.FlashRevision == 0x0000
      || CalibrationData.FlashRevision == 0xFFFF )
  {
    ValidCalibration = 0;  
  }
  
  if ( CalibrationData.batteryCal > 254 )
  {
    ValidCalibration = 0;  
  }
  
  if ( CalibrationData.xtalCap > 3 )
  {
    ValidCalibration = 0;  
  }
  
  if ( CalibrationData.rtcCal > 63 || CalibrationData.rtcCal < -63 )
  {
    ValidCalibration = 0;
  }
  
}

unsigned char QueryCalibrationValid(void)
{
  return ValidCalibration;  
}

unsigned char GetBatteryCalibrationValue(void)
{
  return CalibrationData.batteryCal; 
}

unsigned char GetXtalCalibrationValue(void)
{
  /* we know the value is valid so shift it to the right position */
  return (CalibrationData.xtalCap << 2);  
}

signed char GetRtcCalibrationValue(void)
{
  return CalibrationData.rtcCal;  
}
