#include <string.h>
#include "macro.h"
#include "Application.h"
#include "Graphics.h"
#include "Buttons.h"

#define APPLICATION_MAXIMUM			10

typedef struct {
	unsigned short message;
	Application *target;
	void *parameter;
} AppQueueItem;

#define APPLICATION_QUEUE_LEN		10
#define APPLICATION_QUEUE_SIZE		sizeof(AppQueueItem)
#define TASK_STACK_DEPTH	   		(configMINIMAL_STACK_SIZE + 160)
#define TASK_PRIORITY   			(tskIDLE_PRIORITY + 1)

// Runtime data
static Application *s_applicationStack[APPLICATION_MAXIMUM];
static signed char s_applicationStackLevel;
static xQueueHandle s_applicationQueue;
static xTaskHandle s_applicationTask;

void Application::Initialise(void)
{
	memset(s_applicationStack, 0, sizeof(s_applicationStack));
	s_applicationStackLevel = -1;
	s_applicationQueue = xQueueCreate(APPLICATION_QUEUE_LEN, APPLICATION_QUEUE_SIZE);
	xTaskCreate(MainTask, "Application", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, &s_applicationTask);
	Buttons_Initialise();
}

int Application::GenerateEvent(BUTTON_TYPE button, BUTTON_EVENT event)
{
	AppQueueItem message;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	message.message = MSG_BUTTON;
	message.target = NULL;
	message.parameter = (void*)(((event & 0x3) << 3) | (button & 0x7));
	xQueueSendFromISR(s_applicationQueue, &message, &xHigherPriorityTaskWoken);
	return 1;
}

Application* Application::Current(void)
{
	return s_applicationStack[s_applicationStackLevel];
}

xQueueHandle Application::Queue(void)
{
	return s_applicationQueue;
}

xTaskHandle Application::Task(void)
{
	return s_applicationTask;
}

void Application::Push(Application *toShow)
{
	ENTER_CRITICAL_REGION_QUICK();
	if (s_applicationStackLevel >= 0)
		Application::Current()->Post(MSG_HIDE, (void*)false);
	s_applicationStack[++s_applicationStackLevel] = toShow;
	toShow->Post(MSG_SHOW, (void*)false);
	LEAVE_CRITICAL_REGION_QUICK();
}

void Application::Pop(Application *toHide)
{
	ENTER_CRITICAL_REGION_QUICK();
	if (s_applicationStackLevel < 0)
	{
		// Error!
		goto failed;
	}
	if (s_applicationStack[s_applicationStackLevel] != toHide)
	{
		// Error!
		goto failed;
	}
	s_applicationStack[s_applicationStackLevel--] = NULL;
	toHide->Post(MSG_HIDE, (void*)true);
	if (s_applicationStackLevel >= 0)
		Application::Current()->Post(MSG_SHOW, (void*)true);
failed:
	LEAVE_CRITICAL_REGION_QUICK();
}

Application::Application()
{
	m_active = false;
	m_paintpending = false;
}

Application::~Application()
{
	// Nothing to do, just needs to be virtual
}

void Application::Post(unsigned short message, void *param)
{
	AppQueueItem packet;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (message == MSG_PAINT)
	{
		if (m_paintpending)
			return;
		else
			m_paintpending = true;
	}
	packet.message = message;
	packet.target = this;
	packet.parameter = param;
	xQueueSendFromISR(s_applicationQueue, &packet, &xHigherPriorityTaskWoken);
}

void Application::MainTask(void*)
{
	AppQueueItem message;

	for (;;)
	{
		if (pdTRUE == xQueueReceive(s_applicationQueue, &message, portMAX_DELAY))
		{
			if (message.target == NULL)
				message.target = Application::Current();
			message.target->OnMessage(message.message, message.parameter);
		}
	}
}

void Application::OnMessage(unsigned short message, void *param)
{
	switch (message)
	{
		case MSG_SHOW:
			OnShow((bool)param);
			break;
		case MSG_HIDE:
			OnHide((bool)param);
			break;
		case MSG_BUTTON:
		{
			int data = (int)param;
			OnButton((BUTTON_TYPE)(data & 0x7), (BUTTON_EVENT)((data >> 3) & 0x3));
		}
			break;
		case MSG_PAINT:
			if (m_active)
			{
				Bitmap *render = (Bitmap*)param;
				render->Begin();
				OnPaint(render);
				render->End();
			}
			m_paintpending = false;
			break;
	}
}

void Application::OnShow(bool popping)
{
	// Delay, in case any initialisation is needed?
	m_active = true;
	Post(MSG_PAINT, FrameBuffer);
}

void Application::OnHide(bool popping)
{
	m_active = false;
}

void Application::OnPaint(Bitmap *destination)
{
	// Nothing to do by default
}

void Application::OnButton(BUTTON_TYPE button, BUTTON_EVENT event)
{
	// Nothing to do by default
}
