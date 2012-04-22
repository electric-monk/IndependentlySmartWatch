#pragma once

#include <vector>
#include <map>

typedef struct {
	unsigned short ch;
	unsigned short page;
	float x1, y1, x2, y2;
	int xoffset, yoffset;
	int xadvance;
} FONTCHARACTER;

typedef struct {
	unsigned short first, second;
	int amount;
} FONTKERNING;

class FontFilename
{
public:
	TCHAR data[100];
};

typedef struct FONTDATA {
	int lineHeight, base;
	int width, height;
	std::vector<FONTCHARACTER> characters;
	std::vector<FONTKERNING> kerning;
	std::map<int, FontFilename> bitmaps;
} FONTDATA;

bool ParseFile(const TCHAR *filename, FONTDATA *data);
