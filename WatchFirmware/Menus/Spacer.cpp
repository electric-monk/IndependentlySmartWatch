#include "Bitmap.h"
#include "Spacer.h"

Spacer::Spacer(int height)
{
	m_height = height;
}

void Spacer::Paint(Bitmap *destination, int x, int y, int width, bool selected)
{
	// Nothing to do!
}

int Spacer::Height(void)
{
	return m_height;
}

LineSpacer::LineSpacer(int height, int linePosition)
:Spacer(height)
{
	m_line = linePosition;
	m_left = 0;
	m_right = 0;
}

void LineSpacer::Paint(Bitmap *destination, int x, int y, int width, bool selected)
{
	// No need to waste time calling base class, it does nothing
	destination->Line(x + m_left, y + m_line, x + width - m_right, y + m_line, true);
}
