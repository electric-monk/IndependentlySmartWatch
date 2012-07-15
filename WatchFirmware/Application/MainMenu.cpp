#include "MainMenu.h"
#include "Label.h"
#include "Button.h"
#include "Spacer.h"
#include "TimeMenu.h"
#include "BatteryMenu.h"

#include "Resources/MetaWatch_Large_16pt.h"
#include "Resources/MetaWatch_Combined_8pt.h"

#define MESSAGE_TIME_SETUP			0x0001
#define MESSAGE_BATTERY				0x0002

MainMenu::MainMenu()
{
	Label *label;
	LineSpacer *spacer;
	Button *button;

	label = new Label();
	label->SetFont(&MetaWatch_Large_16pt);
	label->SetText("Settings");
	label->SetAlignment(caCentre);
	AddControl(label);

	spacer = new LineSpacer(5, 2);
	spacer->SetLeftPadding(10);
	spacer->SetRightPadding(10);
	AddControl(spacer);

	button = new Button();
	button->SetFont(&MetaWatch_Combined_8pt);
	button->SetText("Time & Date");
	button->SetTarget(this);
	button->SetMessage(MESSAGE_TIME_SETUP);
	AddControl(button);

	button = new Button();
	button->SetFont(&MetaWatch_Combined_8pt);
	button->SetText("Battery");
	button->SetTarget(this);
	button->SetMessage(MESSAGE_BATTERY);
	AddControl(button);
}

void MainMenu::OnMessage(unsigned short message, void *param)
{
	switch (message)
	{
		default:
			Menu::OnMessage(message, param);
			break;
		case MESSAGE_TIME_SETUP:
			Application::Push(new TimeMenu());
			break;
		case MESSAGE_BATTERY:
			Application::Push(new BatteryMenu());
			break;
	}
}

void MainMenu::OnHide(bool popping)
{
	Menu::OnHide(popping);
	if (popping)
		delete this;
}
