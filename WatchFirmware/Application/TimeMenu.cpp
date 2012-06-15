#include "TimeMenu.h"
#include "Label.h"
#include "Button.h"
#include "Spacer.h"
#include "NumEntry.h"
#include "hal_rtc.h"

#include "Resources/MetaWatch_Large_16pt.h"
#include "Resources/MetaWatch_Large_8pt.h"

static void SetTime(NumberEntry *timeEditor)
{
	timeEditor->SetRangeValueAt(0, 2, GetRTCHOUR());
	timeEditor->SetRangeValueAt(3, 2, GetRTCMIN());
	timeEditor->SetRangeValueAt(6, 2, GetRTCSEC());
}

static void SetDate(NumberEntry *dateEditor)
{
	dateEditor->SetRangeValueAt(0, 2, GetRTCDAY());
	dateEditor->SetRangeValueAt(3, 2, GetRTCMON());
	dateEditor->SetRangeValueAt(6, 4, GetRTCYEAR());
}

static void SaveTime(NumberEntry *timeEditor)
{
	SetRTCHOUR(timeEditor->RangeValueAt(0, 2));
	SetRTCMIN(timeEditor->RangeValueAt(3, 2));
	SetRTCSEC(timeEditor->RangeValueAt(6, 2));
}

static void SaveDate(NumberEntry *dateEditor)
{
	int d = dateEditor->RangeValueAt(0, 2);
	int m = dateEditor->RangeValueAt(3, 2);
	int y = dateEditor->RangeValueAt(6, 4);

	SetRTCYEAR(y);
	SetRTCMON(m);
	SetRTCDAY(d);
	SetRTCDOW(DayOfWeek(y, m, d));
}

TimeMenu::TimeMenu()
{
	Label *label;
	LineSpacer *spacer;

	label = new Label();
	label->SetFont(&MetaWatch_Large_16pt);
	label->SetText("Time & Date");
	label->SetAlignment(caCentre);
	AddControl(label);

	spacer = new LineSpacer(5, 2);
	spacer->SetLeftPadding(10);
	spacer->SetRightPadding(10);
	AddControl(spacer);

	// Time editor
	timeEntry = new NumberEntry();
	timeEntry->SetFont(&MetaWatch_Large_8pt);
	timeEntry->SetFormat("[0-2][0-9]:[0-5][0-9]:[0-5][0-9]"); // TODO: Change with time format?
	timeEntry->SetAlignment(caCentre);
	AddControl(timeEntry);

	// Date editor
	dateEntry = new NumberEntry();
	dateEntry->SetFont(&MetaWatch_Large_8pt);
	dateEntry->SetFormat("[0-3][0-9]:[0-1][0-9]:[0-2][0-9][0-9][0-9]"); // TODO: Change with date format?
	dateEntry->SetAlignment(caCentre);
	AddControl(dateEntry);
}

void TimeMenu::OnShow(bool popping)
{
	Menu::OnShow(popping);
	SetTime(timeEntry);
	SetDate(dateEntry);
}

void TimeMenu::OnHide(bool popping)
{
	Menu::OnHide(popping);
	SaveTime(timeEntry);
	SaveDate(dateEntry);
	if (popping)
		delete this;
}
