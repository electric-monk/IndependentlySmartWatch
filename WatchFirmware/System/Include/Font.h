#ifndef __FONT_H__
#define __FONT_H__

#include "Bitmap.h"

//#define ENABLE_UNICODE

#ifdef ENABLE_UNICODE
typedef unsigned short FontChar;
#else
typedef unsigned char FontChar;
#endif

typedef struct {
	unsigned char lineHeight;
	unsigned char baseline;
	FontChar charCount;	// Hopefully will hold only as most as the character it represents. No support for per-
	unsigned short kernCount;
} FontInformation;

typedef struct {
	FontChar c;
	unsigned short bx, by, bw, bh;
	signed char ox, oy, advance;
	unsigned char image;
} FontCharacter;

typedef struct {
	FontChar a, b;
	signed char offset;
} FontKerning;

class Font
{
public:
	virtual ~Font();

	virtual int Width(const char *str, int len = -1) = 0;
	virtual int Height(void) = 0;
	virtual int Baseline(void)  = 0;

	virtual int Print(Bitmap *destination, int x, int y, const char *str, int len = -1, bool inverse = false) = 0;
};

class BitmapFont : public Font
{
private:
	const FontInformation *fontInformation;
	Bitmap **fontGlyphs;
	const FontCharacter *fontCharacters;
	const FontKerning *fontKernings;

	const FontCharacter* Find(FontChar c);
	const FontKerning* Find(FontChar a, FontChar b);

public:
	BitmapFont(const FontInformation *info, Bitmap **glyphs, const FontCharacter *characters, const FontKerning *kernings);

	int Width(const char *str, int len = -1);
	int Height(void);
	int Baseline(void);

	int Print(Bitmap *destination, int x, int y, const char *str, int len = -1, bool inverse = false);
};

class CompositeFont : public Font
{
public:
	// Note: This class is currently not UTF8-capable

	CompositeFont(Font *uppercase, Font *lowercase, bool followBaseline);

	int Width(const char *str, int len = -1);
	int Height(void);
	int Baseline(void);

	int Print(Bitmap *destination, int x, int y, const char *str, int len = -1, bool inverse = false);

private:
	Font *m_upper, *m_lower;
	bool m_followBaseline;
};

#endif // __FONT_H__
