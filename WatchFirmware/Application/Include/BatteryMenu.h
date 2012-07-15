#ifndef __BATTERYMENU_H__
#define __BATTERYMENU_H__

#include "Menu.h"
#include "Battery.h"
#include "Timer.h"

class Label;

class BatteryMenu : public Menu
{
public:
	static void Initialise(void);

	BatteryMenu();

protected:
	void OnMessage(unsigned short message, void *param);
	void OnShow(bool popping);
	void OnHide(bool popping);

private:
	Label *m_voltage, *m_state;

	void UpdateStatus(bool paint);

	static int BatteryUpdate(Battery_Notify_Handle handle, void *context, Battery_State newState, unsigned int voltage);
	static int TimerUpdate(TimerHandle handle, void *context);
};

#endif // __BATTERYMENU_H__
