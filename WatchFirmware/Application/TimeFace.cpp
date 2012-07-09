#include "TimeFace.h"
#include "Resources/MetaWatch_Combined_8pt.h"
#include "Resources/MetaWatch_Small_8pt.h"
#include "hal_rtc.h"
#include "Battery.h"
#include "Adc.h"
#include "colindebug.h"

static const char *daysOfWeek[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

static Fixed GetRadians(int value, int range)
{
	return Fixed(2) * pi * Fixed(value) / Fixed(range);
}

TimeFace::TimeFace()
{
	m_timer = NULL;
}

TimeFace::~TimeFace()
{
	if (m_timer != NULL)
		Timer_Destroy(m_timer);
}

int TimeFace::SplitHeight(void)
{
	return 1;
}

void TimeFace::OnPaintUpper(Bitmap *destination, int height)
{
// TODO
}

void TimeFace::OnPaintLower(Bitmap *destination, int y)
{
// TODO
}

void TimeFace::OnPaintAll(Bitmap *destination)
{
	int w = destination->Width();
	int h = destination->Height();
	int cx = w / 2;
	int cy = h / 2;

	destination->FillRect(0, 0, w, h, false);

	{
		char buf[10];
		int y = cy + 10;
		MetaWatch_Combined_8pt.Print(destination, 20, y, daysOfWeek[GetRTCDOW()]);
		goodsprintf(buf, "%i/%2i",
				GetRTCDAY(),
				GetRTCMON()
				);
		y += MetaWatch_Combined_8pt.Height();
		MetaWatch_Small_8pt.Print(destination, 20, y, buf);
		goodsprintf(buf, "%4i",
				GetRTCYEAR()
				);
		y += MetaWatch_Small_8pt.Height();
		MetaWatch_Small_8pt.Print(destination, 20, y, buf);
	}

	// Clock markings
	for (int i = 0; i < 12; i++)
		DrawHand(destination, cx, cy, htTick, GetRadians(i, 12));

	int hour = GetRTCHOUR() % 12;
	int minute = GetRTCMIN();
	int second = GetRTCSEC();

	// Hour
	int hourFull = (hour * 60) + minute;
	DrawHand(destination, cx, cy, htHour, GetRadians(hourFull, 12 * 60));

	// Minute
	int minuteFull = (minute * 60) + second;
	DrawHand(destination, cx, cy, htMinute, GetRadians(minuteFull, 60 * 60));

	// Second
	DrawHand(destination, cx, cy, htSecond, GetRadians(second, 60));
}

void TimeFace::OnShow(int type)
{
	WatchFace::OnShow(type);
	if (Visible() && (m_timer == NULL))
		m_timer = Timer_Create(TimerCallback, this, 1000, true);
}

void TimeFace::OnHide(int type)
{
	WatchFace::OnHide(type);
	if ((!Visible()) && (m_timer != NULL))
	{
		Timer_Destroy(m_timer);
		m_timer = NULL;
	}
}

bool TimeFace::AcceptUpper(void)
{
	return !(Visible() & SHOW_UPPER);
}

bool TimeFace::AcceptLower(void)
{
	return !(Visible() & SHOW_LOWER);
}

#define PRECOMPUTE_ANGLE(ang)		Fixed cosang = cosine(ang); Fixed sinang = sine(ang)
#define COMPUTE_X(x, y)				(-((Fixed(x) * cosang) - (Fixed(y) * sinang)))
#define COMPUTE_Y(x, y)				(-((Fixed(y) * cosang) + (Fixed(x) * sinang)))

void TimeFace::DrawHand(Bitmap *destination, int cx, int cy, HandType hand, Fixed degrees)
{
	PRECOMPUTE_ANGLE(degrees);
	bool drawBall = false;
	bool generateArrow = false;
	int lineCount = 0;
	struct {
		int x1, y1, x2, y2;
	} line[4];
	int max = destination->Width();
	line[0].x1 = line[0].x2 = 0;
	switch (hand)
	{
		case htHour:
			line[0].y1 = 0;
			line[0].y2 = max / 4;
			lineCount = 1;
			generateArrow = true;
			break;
		case htMinute:
			line[0].y1 = 0;
			line[0].y2 = (max / 2) - 10;
			lineCount = 1;
			generateArrow = true;
			break;
		case htSecond:
			line[0].y1 = -7;
			line[0].y2 = max / 2;
			lineCount = 1;
			drawBall = true;
			break;
		case htTick:
			line[0].y2 = max / 2;
			line[0].y1 = line[0].y2 - 5;
			lineCount = 1;
			break;
	}
	if (generateArrow)
	{
		line[1].y1 = line[1].y2 = line[0].y2;
		line[1].x1 = -3;
		line[1].x2 = 3;
		line[2].x1 = line[1].x1; line[2].y1 = line[1].y1;
		line[3].x1 = line[1].x2; line[3].y1 = line[1].y2;
		line[2].x2 = line[3].x2 = 0;
		line[2].y2 = line[3].y2 = line[1].y1 + 3;
		lineCount = 4;
	}
	Fixed fcx = cx; Fixed fcy = cy;
	for (int i = 0; i < lineCount; i++)
	{
		Fixed x1 = COMPUTE_X(line[i].x1, line[i].y1) + fcx;
		Fixed y1 = COMPUTE_Y(line[i].x1, line[i].y1) + fcy;
		Fixed x2 = COMPUTE_X(line[i].x2, line[i].y2) + fcx;
		Fixed y2 = COMPUTE_Y(line[i].x2, line[i].y2) + fcy;
		destination->Line(x1, y1, x2, y2, true);
	}
	if (drawBall)
	{
		Fixed x = COMPUTE_X(line[0].x1, line[0].y1) + fcx;
		Fixed y = COMPUTE_Y(line[0].x1, line[0].y1) + fcy;
		destination->Circle(x, y, 3, true);
	}
}

int TimeFace::TimerCallback(TimerHandle handle, void *context)
{
	TimeFace *face = (TimeFace*)context;
	face->Owner()->Repaint();
	return 1;
}
