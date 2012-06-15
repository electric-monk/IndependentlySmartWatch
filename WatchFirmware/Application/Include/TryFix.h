#ifndef __TRYFIX_H__
#define __TRYFIX_H__

#include "Application.h"
#include "Timer.h"

class TryFix : public Application
{
public:
	TryFix();

protected:
	void OnShow(bool popping);
	void OnHide(bool popping);
	void OnButton(BUTTON_TYPE button, BUTTON_EVENT event);
	void OnPaint(Bitmap *destination);

private:
	TimerHandle m_timer;
	int m_count;

	void DrawOne(Bitmap *destination, bool state);
	void DrawTwo(Bitmap *destination, bool state);
};

#endif // __TRYFIX_H__
