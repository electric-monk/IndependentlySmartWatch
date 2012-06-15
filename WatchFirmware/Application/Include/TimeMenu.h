#ifndef __TIMEMENU_H__
#define __TIMEMENU_H__

#include "Menu.h"

class NumberEntry;

class TimeMenu : public Menu
{
public:
	TimeMenu();

protected:
	void OnShow(bool popping);
	void OnHide(bool popping);

private:
	NumberEntry *timeEntry, *dateEntry;
};

#endif // __TIMEMENU_H__
