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
private:
	const FontInformation *fontInformation;
	Bitmap **fontGlyphs;
	const FontCharacter *fontCharacters;
	const FontKerning *fontKernings;

	const FontCharacter* Find(FontChar c);
	const FontKerning* Find(FontChar a, FontChar b);

public:
	Font(const FontInformation *info, Bitmap **glyphs, const FontCharacter *characters, const FontKerning *kernings);

	int Width(const char *str, int len = -1);
	int Height(void) { return fontInformation->lineHeight; }
	int Baseline(void) { return fontInformation->baseline; }

	int Print(Bitmap *destination, int x, int y, const char *str, int len = -1);
};

#endif // __FONT_H__
