#ifndef __SELECTOR_H__
#define __SELECTOR_H__

#include "Control.h"

class Font;

class Selector : public Control
{
public:
	Selector();
	~Selector();

	void SetFont(Font *font) { m_font = font; }
	Font *Font(void) { return m_font; }

	void Add(int value, const char *text);
	void Remove(int value);
	void Select(int value);
	int Selected(void);

	// For Control
	void Paint(Bitmap *destination, int x, int y, int width, bool selected);
	void OnPress(BUTTON_TYPE button);
	int Height(void);

private:
	class Entry {
	public:
		int value;
		char *text;
		Entry *last, *next;

		Entry(Entry **root, int value, const char *label);
		~Entry();

	private:
		Entry **m_root;
	};

	::Font *m_font;
	Entry *m_entries;
	Entry *m_selected;
};

#endif // __SELECTOR_H__
