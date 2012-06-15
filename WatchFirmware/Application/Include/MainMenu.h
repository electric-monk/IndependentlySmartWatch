#ifndef __MAINMENU_H__
#define __MAINMENU_H__

#include "Menu.h"

class MainMenu : public Menu
{
public:
	MainMenu();

protected:
	void OnMessage(unsigned short message, void *param);
	void OnHide(bool popping);
};

#endif // __MAINMENU_H__
