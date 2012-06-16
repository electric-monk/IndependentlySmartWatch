#ifndef __BATTERY_H__
#define __BATTERY_H__
#ifdef __cplusplus
#include "FixedMaths.h"
extern "C" {
#endif

typedef unsigned short Battery_State;
typedef unsigned short Battery_Notify_Handle;

// Callback is called from an ISR, so be careful! return non-zero to wake up OS
typedef int (*Battery_Callback)(Battery_Notify_Handle handle, void *context, Battery_State newState, unsigned int voltage);

// Values match hal_battery.h values for speed
#define Battery_Precharge		0x00
#define Battery_Fast_Charge		0x02
#define Battery_Charge_Done		0x01
#define Battery_Off				0x03	// Off, fault or sleep
// Masked flags
#define Battery_Powered			0x04
#define Battery_Low				0x08	// Battery low warning
#define Battery_LowBluetooth	0x10	// Battery too low for Bluetooth

// Convenience macros
#define Battery_State(x)		((x) & 0x3)

// Notification mask
#define Battery_Notify_Voltage	0x01
#define Battery_Notify_State	0x02

void Battery_Initialise(void);
Battery_Notify_Handle Battery_Register(Battery_Callback callback, unsigned int notify, void *context);
void Battery_Unregister(Battery_Notify_Handle handle);

Battery_State Battery_Status(void);
unsigned int Battery_MilliVoltage(void);

#ifdef __cplusplus
}
Fixed Battery_Voltage(void);
#endif
#endif
