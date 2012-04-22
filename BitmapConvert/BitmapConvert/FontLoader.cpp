#include "StdAfx.h"
#include "FontLoader.h"
#include "bmfont.h"

FontLoader::FontLoader(const TCHAR *filename)
{
	FONTDATA data;

	if (!ParseFile(filename, &data))
		throw "Can't read font file";

	m_height = data.lineHeight;
	m_baseline = data.base;

	for (std::vector<FONTCHARACTER>::iterator it = data.characters.begin(); it != data.characters.end(); it++)
	{
		FontCharacter newChar;

		newChar.ch = (*it).ch;
		// This all looks kind of odd because it's originally from some OpenGL font code.
		newChar.x = (int)(((*it).x1 * data.width) + 0.5);
		newChar.y = (int)(((*it).y1 * data.height) + 0.5);
		newChar.width = (int)((((*it).x2 - (*it).x1) * data.width) + 0.5);
		newChar.height = (int)((((*it).y2 - (*it).y1) * data.height) + 0.5);
		newChar.xoffset = (*it).xoffset;
		newChar.yoffset = (*it).yoffset;
		newChar.xadvance = (*it).xadvance;
		newChar.page = (*it).page;

		m_characters.push_back(newChar);
	}

	for (std::vector<FONTKERNING>::iterator it = data.kerning.begin(); it != data.kerning.end(); it++)
	{
		FontKerning newKern;

		newKern.ch1 = (*it).first;
		newKern.ch2 = (*it).second;
		newKern.offset = (*it).amount;

		m_kernings.push_back(newKern);
	}

	for (std::map<int, FontFilename>::iterator it = data.bitmaps.begin(); it != data.bitmaps.end(); it++)
		m_images[it->first] = _tcsdup(it->second.data);
}

FontLoader::~FontLoader(void)
{
	for (std::map<int, TCHAR*>::iterator it = m_images.begin(); it != m_images.end(); it++)
		free(it->second);
}

const TCHAR* FontLoader::ImageAt(int index)
{
	std::map<int, TCHAR*>::iterator it = m_images.find(index);
	if (it == m_images.end())
		return NULL;
	return it->second;
}

int FontLoader::ImageMax(void)
{
	int max = -1;
	for (std::map<int, TCHAR*>::iterator it = m_images.begin(); it != m_images.end(); it++)
	{
		if (it->first > max)
			max = it->first;
	}
	return max;
}
