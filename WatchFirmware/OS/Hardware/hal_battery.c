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
/*! \file hal_battery.c
*
*/
/******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "hal_board_type.h"
#include "hal_battery.h"

void ConfigureBatteryPins(void)
{
  // enable the resistor on the interface pins to the battery charger
  BAT_CHARGE_REN |= BAT_CHARGE_OPEN_DRAIN_BITS;
  
  // Start with these are pull downs so we use less current
  BAT_CHARGE_OUT &= ~BAT_CHARGE_OPEN_DRAIN_BITS;
  
  // Set these bits a inputs
  BAT_CHARGE_DIR &= ~BAT_CHARGE_OPEN_DRAIN_BITS;
  
  // charge enable output
  BAT_CHARGE_DIR |= BAT_CHARGE_ENABLE_PIN;
  
  BATTERY_CHARGE_DISABLE();

}
