#include "TimeFace.h"
#include "Resources/MetaWatch_Small_8pt.h"
#include "hal_rtc.h"
#include "colindebug.h"

static const char *labels[] = {
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRI",
	"SAT",
};

static void GetDateString(char *buf)
{
	goodsprintf(buf, "%s %i/%2i/%4i",
			labels[GetRTCDOW()],
			GetRTCDAY(),
			GetRTCMON(),
			GetRTCYEAR()
			);
}

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
		char buf[20];
		GetDateString(buf);
		MetaWatch_Small_8pt.Print(destination, 10, cy + 10, buf);

		goodsprintf(buf, "%i:%2i:%2i", GetRTCHOUR(), GetRTCMIN(), GetRTCSEC());
		MetaWatch_Small_8pt.Print(destination, 10, 10, buf);
	}

	int hour = GetRTCHOUR() % 12;
	int minute = GetRTCMIN();
	int second = GetRTCSEC();

	// Hour
	int hourFull = (hour * 60) + minute;
	DrawHand(destination, cx, cy, w / 4, GetRadians(hourFull, 12 * 60));

	// Minute
	int minuteFull = (minute * 60) + second;
	DrawHand(destination, cx, cy, (w / 2) - 10, GetRadians(minuteFull, 60 * 60));

	// Second
	DrawHand(destination, cx, cy, w / 2, GetRadians(second, 60));
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

void TimeFace::DrawHand(Bitmap *destination, int cx, int cy, int length, Fixed degrees)
{
	Fixed fl = length;
	int lx = Fixed(cx) + fl * sine(degrees);
	int ly = Fixed(cy) - fl * cosine(degrees);
	destination->Line(cx, cy, lx, ly, true);
}

int TimeFace::TimerCallback(TimerHandle handle, void *context)
{
	TimeFace *face = (TimeFace*)context;
	face->Owner()->Repaint();
	return 1;
}
