#include <string.h>
#include "Label.h"
#include "Font.h"

Label::Label()
{
	m_text = NULL;
	m_font = NULL;
}

Label::~Label()
{
	if (m_text != NULL)
		delete[] m_text;
}

void Label::SetText(const char *text, int length)
{
	if (m_text != NULL)
		delete[] m_text;
	m_text = NULL;
	if (text != NULL)
	{
		if (length == -1)
			length = strlen(text);
		m_text = new char[length + 1];
		strcpy(m_text, text);
	}
}

void Label::Paint(Bitmap *destination, int x, int y, int width, bool selected)
{
	switch (Alignment())
	{
		case caLeft:
		default:
			break;
		case caRight:
			x += width - m_font->Width(m_text, -1);
			break;
		case caCentre:
			x += (width - m_font->Width(m_text, -1)) / 2;
			break;
	}
	m_font->Print(destination, x, y, m_text, -1);
}

int Label::Height(void)
{
	return m_font->Height();
}
