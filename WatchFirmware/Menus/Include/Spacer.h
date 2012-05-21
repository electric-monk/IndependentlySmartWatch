#ifndef __SPACER_H__
#define __SPACER_H__

#include "Control.h"

class Spacer : public Control
{
public:
	Spacer(int height = 10);

	void SetHeight(int height) { m_height = height; }

	// For Control
	void Paint(Bitmap *destination, int x, int y, int width, bool selected);
	int Height(void);

private:
	int m_height;
};

class LineSpacer : public Spacer
{
public:
	LineSpacer(int height = 10, int linePosition = 5);

	void SetLine(int position) { m_line = position; }
	int Line(void) { return m_line; }

	void SetLeftPadding(int pad) { m_left = pad; }
	int LeftPadding(void) { return m_left; }
	void SetRightPadding(int pad) { m_right = pad; }
	int RightPadding(void) { return m_right; }

	// For Control
	void Paint(Bitmap *destination, int x, int y, int width, bool selected);

private:
	int m_line, m_left, m_right;
};

#endif // __SPACER_H__
