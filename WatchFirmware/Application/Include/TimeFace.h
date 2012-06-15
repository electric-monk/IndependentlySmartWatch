#ifndef __TIMEFACE_H__
#define __TIMEFACE_H__

#include "Watch.h"
#include "FixedMaths.h"
#include "Timer.h"

class TimeFace : public WatchFace
{
public:
	TimeFace();
	~TimeFace();

	int SplitHeight(void);
	void OnPaintUpper(Bitmap *destination, int height);
	void OnPaintLower(Bitmap *destination, int y);
	void OnPaintAll(Bitmap *destination);

	void OnShow(int type);
	void OnHide(int type);

	bool AcceptUpper(void);
	bool AcceptLower(void);

private:
	void DrawHand(Bitmap *destination, int cx, int cy, int length, Fixed degrees);

	static int TimerCallback(TimerHandle handle, void *context);

	TimerHandle m_timer;
};

#endif // __TIMEFACE_H__
