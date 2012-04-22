#include <stdlib.h>
#include "Font.h"

static int CompareChars(const void *a, const void *b)
{
	FontCharacter *chA = (FontCharacter*)a;
	FontCharacter *chB = (FontCharacter*)b;
	return chA->c - chB->c;
}

static int CompareKerns(const void *a, const void *b)
{
	FontKerning *kA = (FontKerning*)a;
	FontKerning *kB = (FontKerning*)b;
	int res = kA->a - kB->a;
	if (res == 0)
		res = kA->b - kB->b;
	else
		res <<= 8;
	return res;
}

class CharacterReader
{
private:
	int utf8Count;
	FontChar next;

public:
	CharacterReader()
	{
		utf8Count = 0;
	}

	// Returns true if a character is ready
	bool HandleByte(char c)
	{
#ifdef ENABLE_UNICODE
		if (c & 0x80)
		{
			if (utf8Count != 0)
			{
				if (c & 0x40)
				{
					// Input contains invalid UTF8
					utf8Count = 0;
					return false;
				}
				next <<= 6;
				next |= c & 0x3F;
				utf8Count--;
				return utf8Count == 0;
			}
			unsigned char mask = 0xFF;
			for (int i = 0x40; (i != 0) && ((c & i) != 0); i >>= 1)
			{
				utf8Count++;
				mask >>= 1;
			}
			if (utf8Count > 6)
			{
				// Input contains invalid UTF8
				utf8Count = 0;
				return false;
			}
			next = c & mask;
			utf8Count--;	// We just ate the first byte
			return false;
		}
		else
		{
			if (utf8Count != 0)
			{
				// Input contains invalid UTF8
				utf8Count = 0;
				// Fall through and return whatever this was, perhaps
			}
			next = c;
			return true;
		}
#else
		next = c;
		return true;
#endif
	}

	FontChar Value(void)
	{
		return next;
	}
};

Font::Font(const FontInformation *info, Bitmap **glyphs, const FontCharacter *characters, const FontKerning *kernings)
{
	fontInformation = info;
	fontGlyphs = glyphs;
	fontCharacters = characters;
	fontKernings = kernings;
}

const FontCharacter* Font::Find(FontChar c)
{
	FontCharacter key;

	key.c = c;
	return (FontCharacter*)bsearch(&key, fontCharacters, fontInformation->charCount, sizeof(FontCharacter), CompareChars);
}

const FontKerning* Font::Find(FontChar a, FontChar b)
{
	FontKerning key;

	if ((a == 0) || (b == 0))
		return NULL;
	key.a = a;
	key.b = b;
	return (FontKerning*)bsearch(&key, fontKernings, fontInformation->kernCount, sizeof(FontKerning), CompareKerns);
}

int Font::Width(const char *str, int len)
{
	int width;
	FontChar last;
	CharacterReader reader;

	width = 0;
	last = 0;
	while ((len != 0) && ((*str) != '\0'))
	{
		bool gotChar = reader.HandleByte(*str);
		str++;
		len--;
		if (!gotChar)
			continue;
		const FontCharacter *ch = Find(reader.Value());
		if (ch == NULL)
		{
			// TODO: Print out 'invalid' glyph
			continue;
		}
		width += ch->advance;
		const FontKerning *kerning = Find(last, ch->c);
		if (kerning != NULL)
			width += kerning->offset;
		last = ch->c;
	}
	return width;
}

int Font::Print(Bitmap *destination, int x, int y, const char *str, int len)
{
	FontChar last;
	CharacterReader reader;

	last = 0;
	destination->Begin();
	while ((len != 0) && ((*str) != '\0'))
	{
		bool gotChar = reader.HandleByte(*str);
		str++;
		len--;
		if (!gotChar)
			continue;
		const FontCharacter *ch = Find(reader.Value());
		if (ch == NULL)
		{
			// TODO: Print out 'invalid' glyph
			continue;
		}
		const FontKerning *kerning = Find(last, ch->c);
		if (kerning != NULL)
			x += kerning->offset;
		destination->Blit(fontGlyphs[ch->image], x + ch->ox, y + ch->oy, ch->bx, ch->by, ch->bw, ch->bh);
		x += ch->advance;
		last = ch->c;
	}
	destination->End();
	return x;
}
