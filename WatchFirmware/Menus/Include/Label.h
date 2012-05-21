#ifndef __LABEL_H__
#define __LABEL_H__

#include "Control.h"

class Font;

class Label : public Control
{
public:
	Label();
	~Label();

	void SetText(const char *text, int length = -1);
	const char* Text(void) { return m_text; }

	void SetFont(Font *font) { m_font = font; }
	Font* Font(void) { return m_font; }

	// For Control
	void Paint(Bitmap *destination, int x, int y, int width, bool selected);
	int Height(void);

private:
	char *m_text;
	::Font *m_font;
};

#endif // __LABEL_H__
