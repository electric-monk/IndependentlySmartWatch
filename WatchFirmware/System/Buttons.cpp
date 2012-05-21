#include <string.h>
#include "hal_lpm.h"
#include "Buttons.h"
#include "Timer.h"
#include "Application.h"

#define BUTTON_DEBOUNCE_COUNTER		50		// milliseconds
#define BUTTON_DEBOUNCE_COUNT		100		// Should be divisible by above
#define BUTTON_LONG_PRESS			250		// Should be divisible by above

static struct {
	unsigned char activated;
	unsigned short startTime, releaseTime;
} s_buttonStates[NUMBER_OF_BUTTONS];
static unsigned short s_buttonsActive;

extern "C" void Buttons_Initialise(void)
{
	memset(s_buttonStates, 0, sizeof(s_buttonStates));
	s_buttonsActive = 0;
	CONFIGURE_BUTTON_PINS();
}

static int timerButtons(TimerHandle timer, void *context)
{
	int result = 0;
	int i;

	for (i = 0; i < NUMBER_OF_BUTTONS; i++)
	{
		// Is button activated?
		if (!s_buttonStates[i].activated)
			continue;
		// Is the button currently pressed?
		if (BUTTON_PORT_IN & (1 << i))
		{
			// Pressed
			if (s_buttonStates[i].startTime != 0)
			{
				if ((s_buttonStates[i].startTime--) == 0)
					result |= Application::GenerateEvent((BUTTON_TYPE)i, beHold);
			}
			s_buttonStates[i].releaseTime = BUTTON_DEBOUNCE_COUNT / BUTTON_DEBOUNCE_COUNTER;
		}
		else
		{
			if ((s_buttonStates[i].releaseTime--) == 0)
			{
				result |= Application::GenerateEvent((BUTTON_TYPE)i, beUp);
				s_buttonsActive--;
				s_buttonStates[i].activated = 0;
			}
		}
	}
	if (s_buttonsActive == 0)
		Timer_Destroy(timer);
	return result;
}

#pragma vector=BUTTON_PORT_VECTOR
__interrupt void isrButtons(void)
{
	unsigned char isrTriggers = BUTTON_PORT_IFG;
	int result = 0;
	int i;

	for (i = 0; i < NUMBER_OF_BUTTONS; i++)
	{
		// Did this button trigger the interrupt?
		if (!(isrTriggers & (1 << i)))
			continue;
		// Is this button in a debouncing period?
		if (!s_buttonStates[i].activated)
		{
			// Activate debouncing and long press mechanism
			s_buttonStates[i].activated = 1;
			s_buttonStates[i].startTime = BUTTON_LONG_PRESS / BUTTON_DEBOUNCE_COUNTER;
			s_buttonStates[i].releaseTime = BUTTON_DEBOUNCE_COUNT / BUTTON_DEBOUNCE_COUNTER;
			if (s_buttonsActive == 0)
				Timer_Create(timerButtons, NULL, BUTTON_DEBOUNCE_COUNTER, 1);
			s_buttonsActive++;
			// Trigger event
			result |= Application::GenerateEvent((BUTTON_TYPE)i, beDown);
		}
	}
	BUTTON_PORT_IFG = 0;
	if (result)
		EXIT_LPM_ISR();
}
