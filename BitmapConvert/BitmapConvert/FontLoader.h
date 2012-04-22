#pragma once

#include <vector>
#include <map>

typedef struct {
	unsigned short ch;
	int x, y, width, height;
	int xoffset, yoffset, xadvance;
	int page;
} FontCharacter;

typedef struct {
	unsigned short ch1, ch2;
	int offset;
} FontKerning;

class FontLoader
{
public:
	FontLoader(const TCHAR *filename);
	~FontLoader(void);

	// Glyph images
	const TCHAR* ImageAt(int index);
	int ImageMax(void);

	// Font data
	int LineHeight(void) { return m_height; }
	int Baseline(void) { return m_baseline; }

	// Font entries
	const std::vector<FontCharacter>& Characters() { return m_characters; }
	const std::vector<FontKerning>& Kernings() { return m_kernings; }

private:
	int m_height, m_baseline;
	std::vector<FontCharacter> m_characters;
	std::vector<FontKerning> m_kernings;
	std::map<int, TCHAR*> m_images;
};
