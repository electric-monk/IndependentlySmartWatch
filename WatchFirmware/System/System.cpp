#include "FreeRTOS.h"
#include "System.h"
#include "Graphics.h"
#include "Resources/MetaWatch_Large_16pt.h"
#include "Resources/MetaWatch_Large_8pt.h"
#include "Resources/MetaWatch_Small_8pt.h"
#include "Resources/neko.h"

#include "hal_rtc.h"
#include "task.h"

#include "Timer.h"

void* operator new(size_t size)
{
	return pvPortMalloc(size);
}

void* operator new[](size_t size)
{
	return operator new(size);
}

void operator delete(void *p)
{
	vPortFree(p);
}

void operator delete[](void *p)
{
	operator delete (p);
}

#define TASK_STACK_DEPTH	   	(configMINIMAL_STACK_SIZE + 160)
#define TASK_PRIORITY   		(tskIDLE_PRIORITY + 1)

static xTaskHandle TaskHandle;

static int TimerTaunt(TimerHandle handle, void *context)
{
	xTaskResumeFromISR(TaskHandle);
//	vTaskJobAdd();
	return 1;
}

static char buf[20];

extern "C" void goodsprintf(char*,char*,...);
static void MainTask(void *parameter)
{
	Timer_Create(TimerTaunt, NULL, 1000, 1);
	while (1)
	{
		vTaskSuspend(NULL);
		RTC_DATA temp;
		halRtcGet(&temp);
		goodsprintf(buf, "%i:%2i:%2i", temp.Hour, temp.Minute, temp.Second);
		FrameBuffer->Begin();
		int w = MetaWatch_Large_8pt.Width(buf, -1);
		int h = MetaWatch_Large_8pt.Height();
		FrameBuffer->FillRect(50, 55, 50 + w + 10, 55 + h, false);
		MetaWatch_Large_8pt.Print(FrameBuffer, 50, 55,buf, -1);
		FrameBuffer->End();
//		vTaskJobComplete();
	}
}

void InitialiseSystem(void)
{
	GraphicsInit();

	FrameBuffer->Begin();
	FrameBuffer->Circle(96/2, 96/2, 50, true);
	FrameBuffer->Line(0, 0, 95, 95, true);
	FrameBuffer->Line(0, 95, 95, 0, false);
	int x = MetaWatch_Large_8pt.Print(FrameBuffer, 0, 0, "T", -1);
	MetaWatch_Small_8pt.Print(FrameBuffer, x, MetaWatch_Large_8pt.Baseline() - MetaWatch_Small_8pt.Baseline(), "ESTING", -1);
	MetaWatch_Large_16pt.Print(FrameBuffer, 5, MetaWatch_Large_8pt.Height(), "Hello!", -1);
	char buf[10];
	extern unsigned short reason;
	goodsprintf(buf, "Reason %i", reason);
	MetaWatch_Large_8pt.Print(FrameBuffer, 0, MetaWatch_Large_8pt.Height() + MetaWatch_Large_16pt.Height(), buf, -1);
	FrameBuffer->Blit(&neko, 50, MetaWatch_Large_8pt.Height() + MetaWatch_Large_16pt.Height() + 5, BLIT_INVERT);
	FrameBuffer->End();

	xTaskCreate(MainTask, "Main", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, &TaskHandle);
}
