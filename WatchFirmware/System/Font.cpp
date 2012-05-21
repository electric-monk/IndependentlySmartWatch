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

Font::~Font()
{
}

BitmapFont::BitmapFont(const FontInformation *info, Bitmap **glyphs, const FontCharacter *characters, const FontKerning *kernings)
{
	fontInformation = info;
	fontGlyphs = glyphs;
	fontCharacters = characters;
	fontKernings = kernings;
}

const FontCharacter* BitmapFont::Find(FontChar c)
{
	FontCharacter key;

	key.c = c;
	return (FontCharacter*)bsearch(&key, fontCharacters, fontInformation->charCount, sizeof(FontCharacter), CompareChars);
}

const FontKerning* BitmapFont::Find(FontChar a, FontChar b)
{
	FontKerning key;

	if ((a == 0) || (b == 0))
		return NULL;
	key.a = a;
	key.b = b;
	return (FontKerning*)bsearch(&key, fontKernings, fontInformation->kernCount, sizeof(FontKerning), CompareKerns);
}

int BitmapFont::Width(const char *str, int len)
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

int BitmapFont::Height(void)
{
	return fontInformation->lineHeight;
}

int BitmapFont::Baseline(void)
{
	return fontInformation->baseline;
}

int BitmapFont::Print(Bitmap *destination, int x, int y, const char *str, int len, bool inverse)
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
		destination->Blit(fontGlyphs[ch->image], x + ch->ox, y + ch->oy, ch->bx, ch->by, ch->bw, ch->bh, inverse ? BLIT_POSTINVERT : 0);
		x += ch->advance;
		last = ch->c;
	}
	destination->End();
	return x;
}

static bool IsUpperFont(char c)
{
	return !((c >= 'a') && (c <= 'z'));
}

static void MakeUpper(char *buf, const char *input, int len)
{
	while (len != 0)
	{
		char c = *(input++);
		if ((c >= 'a') && (c <= 'z'))
			c -= 0x20;
		*(buf++) = c;
		len--;
	}
}

static int CountLength(const char *str, int max)
{
	if (max == 0)
		return 0;
	if ((*str) == '\0')
		return 0;
	if (max == 1)
		return 1;
	bool state = IsUpperFont(*str);
	int count = 1;
	max--;
	str++;
	while ((*str) != '\0')
	{
		if (IsUpperFont(*str) != state)
			break;
		str++;
		count++;
		max--;
		if (max == 0)
			break;
	}
	return count;
}

CompositeFont::CompositeFont(Font *uppercase, Font *lowercase, bool followBaseline)
{
	m_upper = uppercase;
	m_lower = lowercase;
	m_followBaseline = followBaseline;
}

int CompositeFont::Width(const char *str, int len)
{
	int total;
	int runLength;

	total = 0;
	while ((len != 0) && ((*str) != '\0'))
	{
		runLength = CountLength(str, len);
		total += (IsUpperFont(*str) ? m_upper : m_lower)->Width(str, runLength);
		len -= runLength;
		str += runLength;
	}
	return total;
}

int CompositeFont::Height(void)
{
	return m_upper->Height();
}

int CompositeFont::Baseline(void)
{
	return m_upper->Baseline();
}

int CompositeFont::Print(Bitmap *destination, int x, int y, const char *str, int len, bool inverse)
{
	int runLength;
	int yLow;

	yLow = y;
	if (m_followBaseline)
		yLow += m_upper->Baseline() - m_lower->Baseline();
	while ((len != 0) && ((*str) != '\0'))
	{
		runLength = CountLength(str, len);
		if (IsUpperFont(*str))
		{
			x = m_upper->Print(destination, x, y, str, runLength, inverse);
			len -= runLength;
			str += runLength;
		}
		else
		{
			char temp[10];

			while (runLength > 0)
			{
				int copy = runLength;
				if (copy > sizeof(temp))
					copy = sizeof(temp);
				MakeUpper(temp, str, copy);
				x = m_lower->Print(destination, x, yLow, temp, copy, inverse);
				len -= copy;
				str += copy;
				runLength -= copy;
			}
		}
	}
	return x;
}
