#include "msp430.h"
#include "hal_digital_v2_defs.h"

#ifndef LPM_ENABLED
#define LPM_ENABLED
#endif

void TaskDelayLpmDisable(void);
void TaskDelayLpmEnable(void);
