#ifndef __BUTTONS_H__
#define __BUTTONS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "hal_board_type.h"

typedef enum {
	beDown,
	beUp,
	beHold,
} BUTTON_EVENT;

typedef enum {
	btLeftTop = SW_F_INDEX,
	btLeftCentre = SW_E_INDEX,
	btLeftBottom = SW_D_INDEX,
	btRightTop = SW_A_INDEX,
	btRightCentre = SW_B_INDEX,
	btRightBottom = SW_C_INDEX,
} BUTTON_TYPE;

// The callback function that uses the above enums could be made a configurable
// callback if someone wants to be able to customise button handling, like in
// the other functionality. Currently it's implemented in the Application class.

void Buttons_Initialise(void);

#ifdef __cplusplus
}
#endif
#endif
