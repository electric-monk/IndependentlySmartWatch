#include <string.h>
#include "hal_board_type.h"
#include "hal_battery.h"
#include "Adc.h"
#include "Timer.h"
#include "Battery.h"
#include "macro.h"

// From Metawatch
#define DEFAULT_LOW_BATTERY_WARNING_LEVEL    3500
#define DEFAULT_LOW_BATTERY_BTOFF_LEVEL      3300

#define BATTERY_UPDATE			8000	// milliseconds
#define BATTERY_MAX_NOTIFY		2

typedef struct {
	unsigned short notify;
	Battery_Callback callback;
	void *context;
} BATTERY_NOTIFICATION;

static Battery_State s_currentState;
static unsigned int s_currentVoltage;
static BATTERY_NOTIFICATION s_notifications[BATTERY_MAX_NOTIFY];

static int adcBattery(Adc_Request_Type request, void *context, unsigned int value)
{
	Battery_State newState = (Battery_State)context;
	if (Adc_Battery_Average_Available())
	{
		value = Adc_Battery_Average();
		if (!(newState & Battery_Powered))
		{
			if (value < DEFAULT_LOW_BATTERY_BTOFF_LEVEL)
				newState |= Battery_LowBluetooth;
			if (value < DEFAULT_LOW_BATTERY_WARNING_LEVEL)
				newState |= Battery_Low;
		}
	}
	unsigned int changes = 0;
	if (newState != s_currentState)
	{
		s_currentState = newState;
		changes |= Battery_Notify_State;
	}
	if (value != s_currentVoltage)
	{
		s_currentVoltage = value;
		changes |= Battery_Notify_Voltage;
	}
	int result = 0;
	if (changes != 0)
	{
		// Generate notifications
		for (int i = 0; i < BATTERY_MAX_NOTIFY; i++)
		{
			if (s_notifications[i].notify & changes)
				result |= s_notifications[i].callback(i + 1, s_notifications[i].context, newState, value);
		}
	}
	return result;
}

static int timerChargingControl(TimerHandle timer, void *context)
{
	Timer_Destroy(timer);

	// Get reading
	unsigned char BatteryCharging = BAT_CHARGE_IN;

	// Work out if external power is connected
	bool powerGood = !(BatteryCharging & BAT_CHARGE_PWR_GOOD);

	// Work out battery state
	BatteryCharging &= BAT_CHARGE_STAT1 | BAT_CHARGE_STAT2;
	unsigned int powerState = BatteryCharging >> 3;

	if (powerState == BATTERY_CHARGE_OFF_FAULT_SLEEP)
	{
		BATTERY_CHARGE_DISABLE();
		BAT_CHARGE_OUT &= ~BAT_CHARGE_OPEN_DRAIN_BITS;
	}

	// Work out new state
	Battery_State newState = powerState;
	if (powerGood)
		newState |= Battery_Powered;

	// Launch ADC cycle
	Adc_Request(adcBatteryLevel, adcBattery, (void*)newState);

	return 0;
}

static int timerBattery(TimerHandle timer, void *context)
{
	// Initiate battery charging control
	BAT_CHARGE_OUT |= BAT_CHARGE_OPEN_DRAIN_BITS;
	BATTERY_CHARGE_ENABLE();

	// Fire off another timer, to wait a while
	Timer_Create(timerChargingControl, NULL, 50, 0);

	return 0;
}

extern "C" void Battery_Initialise(void)
{
	s_currentState = Battery_Off;
	s_currentVoltage = 0;
	memset(s_notifications, 0, sizeof(s_notifications));
	ConfigureBatteryPins();
	Timer_Create(timerBattery, NULL, BATTERY_UPDATE, 1);
}

extern "C" Battery_Notify_Handle Battery_Register(Battery_Callback callback, unsigned int notify, void *context)
{
	if (notify == 0)
		return 0;
	Battery_Notify_Handle result = 0;
	ENTER_CRITICAL_REGION_QUICK();
	for (int i = 0; i < BATTERY_MAX_NOTIFY; i++)
	{
		if (s_notifications[i].notify == 0)
		{
			result = i + 1;
			break;
		}
	}
	if (result != 0)
	{
		s_notifications[result - 1].notify = notify;
		s_notifications[result - 1].callback = callback;
		s_notifications[result - 1].context = context;
	}
	LEAVE_CRITICAL_REGION_QUICK();
	return result;
}

extern "C" void Battery_Unregister(Battery_Notify_Handle handle)
{
	if (handle == 0)
		return;
	ENTER_CRITICAL_REGION_QUICK();
	s_notifications[handle - 1].notify = 0;
	s_notifications[handle - 1].callback = NULL;
	s_notifications[handle - 1].context = NULL;
	LEAVE_CRITICAL_REGION_QUICK();
}

extern "C" Battery_State Battery_Status(void)
{
	return s_currentState;
}

extern "C" unsigned int Battery_MilliVoltage(void)
{
	return s_currentVoltage;
}

Fixed Battery_Voltage(void)
{
	return Fixed((long)s_currentVoltage) / Fixed(1000);
}
