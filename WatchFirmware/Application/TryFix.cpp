#include "Timer.h"
#include "Graphics.h"
#include "TryFix.h"

static int TryFixDraw(TimerHandle handle, void *context)
{
	((TryFix*)context)->Post(MSG_PAINT, FrameBuffer);
	return 1;
}

TryFix::TryFix()
{
	m_count = 0;
}

void TryFix::OnShow(bool popping)
{
	Application::OnShow(popping);
	m_timer = Timer_Create(TryFixDraw, this, 100, true);
}

void TryFix::OnHide(bool popping)
{
	Timer_Destroy(m_timer);
	Application::OnHide(popping);
}

void TryFix::OnButton(BUTTON_TYPE button, BUTTON_EVENT event)
{
	if (event == beDown)
		Application::Pop(this);
	else
		Application::OnButton(button, event);
}

void TryFix::OnPaint(Bitmap *destination)
{
	m_count++;
	if (m_count >= 30)
		m_count = 0;

	if (m_count < 10)
	{
		DrawOne(destination, m_count & 1);
		return;
	}
	if (m_count < 20)
	{
		DrawTwo(destination, m_count & 1);
		return;
	}
	if (m_count & 1)
		DrawTwo(destination, m_count & 2);
	else
		DrawOne(destination, m_count & 2);
}

void TryFix::DrawOne(Bitmap *destination, bool state)
{
	destination->FillRect(0, 0, destination->Width(), destination->Height(), state);
}

void TryFix::DrawTwo(Bitmap *destination, bool state)
{
	bool b = state;
	int w = destination->Width();
	int h = destination->Height();
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			destination->WritePixel(x, y, b);
			b = !b;
		}
	}
}
