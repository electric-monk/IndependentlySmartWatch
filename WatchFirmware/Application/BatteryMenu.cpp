#include <string.h>
#include "Battery.h"
#include "colindebug.h"
#include "Label.h"
#include "Spacer.h"
#include "hal_rtc.h"
#include "Resources/MetaWatch_Large_16pt.h"
#include "Resources/MetaWatch_Combined_8pt.h"
#include "BatteryMenu.h"

#define MESSAGE_BATTERY_UPDATE			0x0001

#define GRAPH_SAMPLES					96
#define GRAPH_SAMPLE_TIME				TimerLength(15) * TimerLength(60) * TimerLength(1000)
#define GRAPH_SAMPLE_TICKS				4 // Tick per four samples

static void MakeState(char *buf, Battery_State status)
{
	char *a, *b;

	a = "";
	if (status & Battery_Powered)
		a = " (on power)";
	switch (Battery_State(status))
	{
		case Battery_Precharge:
			b = "Precharge";
			break;
		case Battery_Fast_Charge:
			b = "Fast charge";
			break;
		case Battery_Charge_Done:
			b = "Charged";
			break;
		case Battery_Off:
		default:
			b = "Off";
			break;
	}
	goodsprintf(buf, "%s%s", b, a);
}

BatteryMenu *s_active;
Fixed s_readings[GRAPH_SAMPLES];
int s_lastReading;
Battery_Notify_Handle s_batteryHandle;
TimerHandle s_timerHandle;

#define GRAPH_HEIGHT			30
#define MAX_VOLTAGE				5

class BatteryGraph : public Control
{
public:
	void Paint(Bitmap *destination, int x, int y, int width, bool selected)
	{
		Fixed fx = Fixed(x);
		Fixed fh = Fixed(GRAPH_HEIGHT);
		Fixed fy = Fixed(y) + fh + Fixed(1);
		Fixed sx = Fixed(width) / Fixed(GRAPH_SAMPLES);
		Fixed sy = fh / Fixed(MAX_VOLTAGE);
		Fixed lastX = fx;
		int currentSample = (s_lastReading + 1) % GRAPH_SAMPLES;
		Fixed lastY = fy - (sy * s_readings[currentSample]);
		for (int i = 1; i < GRAPH_SAMPLES; i++)
		{
			Fixed currentX = fx + (Fixed(i) * sx);
			currentSample = (currentSample + 1) % GRAPH_SAMPLES;
			Fixed currentY = fy - (sy * s_readings[currentSample]);
			destination->Line(lastX, lastY, currentX, currentY, true);
			lastX = currentX;
			lastY = currentY;
			if (((s_lastReading + i) % GRAPH_SAMPLE_TICKS) == 0)
				destination->Line(currentX, y + GRAPH_HEIGHT + 2, currentX, y + GRAPH_HEIGHT, true);
		}
		destination->Line(x, y, x + width, y, true);
		destination->Line(x, y + GRAPH_HEIGHT + 2, x + width, y + GRAPH_HEIGHT + 2, true);
	}

	int Height(void)
	{
		return GRAPH_HEIGHT + 2;
	}
};

void BatteryMenu::Initialise(void)
{
	s_active = NULL;
	memset(s_readings, 0, sizeof(s_readings));
	s_lastReading = 0;
	s_timerHandle = Timer_Create(TimerUpdate, NULL, GRAPH_SAMPLE_TIME, true);
}

BatteryMenu::BatteryMenu()
{
	Label *label;
	LineSpacer *spacer;
	BatteryGraph *graph;

	label = new Label();
	label->SetFont(&MetaWatch_Large_16pt);
	label->SetText("Battery");
	label->SetAlignment(caCentre);
	AddControl(label);

	spacer = new LineSpacer(5, 2);
	spacer->SetLeftPadding(10);
	spacer->SetRightPadding(10);
	AddControl(spacer);

	m_voltage = new Label();
	m_voltage->SetFont(&MetaWatch_Combined_8pt);
	m_voltage->SetAlignment(caCentre);
	AddControl(m_voltage);

	m_state = new Label();
	m_state->SetFont(&MetaWatch_Combined_8pt);
	m_state->SetAlignment(caCentre);
	AddControl(m_state);

	graph = new BatteryGraph();
	AddControl(graph);

	UpdateStatus(false);
}

void BatteryMenu::OnMessage(unsigned short message, void *param)
{
	switch (message)
	{
		case MESSAGE_BATTERY_UPDATE:
			UpdateStatus(true);
			break;
		default:
			Menu::OnMessage(message, param);
			break;
	}
}

void BatteryMenu::OnShow(bool popping)
{
	Menu::OnShow(popping);
	if (s_active == NULL)
		s_batteryHandle = Battery_Register(BatteryUpdate, Battery_Notify_Voltage | Battery_Notify_State, NULL);
	s_active = this;
}

void BatteryMenu::OnHide(bool popping)
{
	Menu::OnHide(popping);
	if (s_active == this)
	{
		Battery_Unregister(s_batteryHandle);
		s_active = NULL;
	}
}

void BatteryMenu::UpdateStatus(bool paint)
{
	char buf[30];

	goodsprintf(buf, "%FV", Battery_Voltage());
	m_voltage->SetText(buf);
	MakeState(buf, Battery_Status());
	m_state->SetText(buf);
	if (paint)
		Update();
}

int BatteryMenu::BatteryUpdate(Battery_Notify_Handle handle, void *context, Battery_State newState, unsigned int voltage)
{
	if (s_active != NULL)
	{
		s_active->Post(MESSAGE_BATTERY_UPDATE, NULL);
		return 1;
	}
	return 0;
}

int BatteryMenu::TimerUpdate(TimerHandle handle, void *context)
{
	s_lastReading++;
	if (s_lastReading >= GRAPH_SAMPLES)
		s_lastReading = 0;
	s_readings[s_lastReading] = Battery_Voltage();
	if (s_active != NULL)
	{
		s_active->Post(MESSAGE_BATTERY_UPDATE, NULL);
		return 1;
	}
	return 0;
}
