#ifndef __NUMENTRY_H__
#define __NUMENTRY_H__

#include "Control.h"

class Font;

class NumberEntry : public Control
{
public:
	NumberEntry();
	~NumberEntry();

	void SetFont(Font *font) { m_font = font; }
	Font *Font(void) { return m_font; }

	void SetFormat(const char *format);

	int ValueAt(int index) { return m_items[index].value; }
	void SetValueAt(int index, int value);
	int ValueCount(void) { return m_count; }

	// For Control
	void Paint(Bitmap *destination, int x, int y, int width, bool selected);
	void OnPress(BUTTON_TYPE button);
	int Height(void);

private:
	typedef enum {
		tCharacter,
		tDigit,
	} TYPE;
	typedef struct {
		TYPE type;
		unsigned char value;
		unsigned char min, max;
	} HOLDER;
	::Font *m_font;
	HOLDER *m_items;
	int m_index, m_count;

	static const char* ParseEntry(const char *input, HOLDER *entry);
	static int CountEntries(const char *input);
	static char GetCharacter(HOLDER *entry);
	void Print(Bitmap *destination, HOLDER *entry, int x, int y, bool inverse);
	int Width(HOLDER *entry);
};

#endif // __NUMENTRY_H__
