#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "Buttons.h"

enum {
	MSG_BUTTON = 0xF000,
	MSG_SHOW,
	MSG_HIDE,
	MSG_PAINT,
};

class Bitmap;

class Application
{
public:
	static void Initialise(void);
	static int GenerateEvent(BUTTON_TYPE button, BUTTON_EVENT event);
	static Application* Current(void);
	static xQueueHandle Queue(void);
	static xTaskHandle Task(void);

	static void Push(Application *toShow);
	static void Pop(Application *toHide);

	Application();
	virtual ~Application();

	void Post(unsigned short message, void *param);

protected:
	virtual void OnMessage(unsigned short message, void *param);
	virtual void OnShow(bool popping);
	virtual void OnHide(bool popping);
	virtual void OnButton(BUTTON_TYPE button, BUTTON_EVENT event);
	virtual void OnPaint(Bitmap *destination);

private:
	static void MainTask(void*);

	bool m_active, m_paintpending;
};

#endif // __APPLICATION_H__
