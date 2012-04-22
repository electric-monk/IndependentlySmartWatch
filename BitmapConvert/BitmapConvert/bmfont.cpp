#include "StdAfx.h"
#include <algorithm>
#include "bmfont.h"

#pragma warning(disable: 4996)

#define BUF_LENGTH		1024

#define IS_WHITESPACE(x)			(((x) == ' ') || ((x) == '\t'))

typedef enum {
	btUnknown,
	btINFO,
	btCOMMON,
	btPAGE,
	btCHAR,
	btKERNING,
} BMFTYPE;

static BMFTYPE GetBMFType(const TCHAR *buffer)
{
#define CHECK(x)		if (_tcsnicmp(buffer, _T(#x) _T(" "), _tcslen(_T(#x) _T(" "))) == 0) return bt##x
	CHECK(INFO);
	CHECK(COMMON);
	CHECK(PAGE);
	CHECK(CHAR);
	CHECK(KERNING);
#undef CHECK
	return btUnknown;
}

static bool FindParameter(const TCHAR *data, const TCHAR *parameter, TCHAR *value)
{
	const TCHAR *found;

	found = _tcsstr(data, parameter);
	if (found == NULL)
		return false;
	found += _tcslen(parameter);
	while (IS_WHITESPACE(*found))
		found++;
	if ((*found) != '=')
		return false;
	found++;
	while (IS_WHITESPACE(*found))
		found++;
	if ((*found) == '\"')
	{
		found++;
		while (((*found) != '\"') && ((*found) != '\0'))
			(*(value++)) = (*(found++));
		if ((*found) == '\0')
			return false;
	}
	else
	{
		while (!IS_WHITESPACE(*found) && ((*found) != '\0'))
			(*(value++)) = (*(found++));
	}
	(*value) = '\0';
	return true;
}

static bool ParseCommon(const TCHAR *data, FONTDATA *info)
{
	TCHAR temp[100];

	if (!FindParameter(data, _T("lineHeight"), temp))
		return false;
	info->lineHeight = _ttoi(temp);
	if (!FindParameter(data, _T("base"), temp))
		return false;
	info->base = _ttoi(temp);
	if (!FindParameter(data, _T("scaleW"), temp))
		return false;
	info->width = _ttoi(temp);
	if (!FindParameter(data, _T("scaleH"), temp))
		return false;
	info->height = _ttoi(temp);
	return true;
}

static bool ParsePage(const TCHAR *data, FONTDATA *info)
{
	TCHAR filename[100], temp[100];
	int id;

	if (!FindParameter(data, _T("id"), temp))
		return false;
	id = _ttoi(temp);
	if (!FindParameter(data, _T("file"), filename))
		return false;
	// Now, load the texture
	_tcscpy(info->bitmaps[id].data, filename);
	return true;
}

static bool ParseCharacter(const TCHAR *data, FONTDATA *info, FONTCHARACTER *output)
{
	TCHAR temp[100];
	int x, y, w, h;

	if (!FindParameter(data, _T("id"), temp))
		return false;
	output->ch = _ttoi(temp);
	if (!FindParameter(data, _T("x"), temp))
		return false;
	x = _ttoi(temp);
	if (!FindParameter(data, _T("y"), temp))
		return false;
	y = _ttoi(temp);
	if (!FindParameter(data, _T("width"), temp))
		return false;
	w = _ttoi(temp);
	if (!FindParameter(data, _T("height"), temp))
		return false;
	h = _ttoi(temp);
	if (!FindParameter(data, _T("xoffset"), temp))
		return false;
	output->xoffset = _ttoi(temp);
	if (!FindParameter(data, _T("yoffset"), temp))
		return false;
	output->yoffset = _ttoi(temp);
	if (!FindParameter(data, _T("xadvance"), temp))
		return false;
	output->xadvance = _ttoi(temp);
	if (!FindParameter(data, _T("page"), temp))
		return false;
	output->page = _ttoi(temp);
	// Calculate
	output->x1 = float(x) / float(info->width);
	output->y1 = float(y) / float(info->height);
	output->x2 = float(x + w) / float(info->width);
	output->y2 = float(y + h) / float(info->height);
	// Done
	return true;
}

static bool ParseKerning(const TCHAR *data, FONTKERNING *output)
{
	TCHAR temp[100];

	if (!FindParameter(data, _T("first"), temp))
		return false;
	output->first = _ttoi(temp);
	if (!FindParameter(data, _T("second"), temp))
		return false;
	output->second = _ttoi(temp);
	if (!FindParameter(data, _T("amount"), temp))
		return false;
	output->amount = _ttoi(temp);
	return true;
}

static bool CompareChars(FONTCHARACTER a, FONTCHARACTER b)
{
	return a.ch < b.ch;
}

static bool CompareKerns(FONTKERNING a, FONTKERNING b)
{
	if (a.first == b.first)
		return a.second < b.second;
	else
		return a.first < b.first;
}

bool ParseFile(const TCHAR *filename, FONTDATA *data)
{
	FILE *input;
	TCHAR buf[BUF_LENGTH];
	BMFTYPE type;

	input = _tfopen(filename, _T("rt"));
	if (input == NULL)
	{
		_ftprintf(stderr, _T("Unable to open '%s'\n"), filename);
		return false;
	}
	while (_fgetts(buf, BUF_LENGTH, input) != NULL)
	{
		type = GetBMFType(buf);
		switch (type)
		{
			case btCOMMON:
				if (!ParseCommon(buf, data))
				{
					_ftprintf(stderr, _T("Unable to parse 'common' field\n"));
					return false;
				}
				break;

			case btPAGE:
				if (!ParsePage(buf, data))
				{
					_ftprintf(stderr, _T("Unable to parse 'page' field\n"));
					return false;
				}
				break;

			case btCHAR:
				{
					FONTCHARACTER character;

					if (ParseCharacter(buf, data, &character))
						data->characters.push_back(character);
				}
				break;

			case btKERNING:
				{
					FONTKERNING kerning;

					if (ParseKerning(buf, &kerning))
						data->kerning.push_back(kerning);
				}
				break;

			default:
				break;
		}
	}
	fclose(input);
	std::sort(data->characters.begin(), data->characters.end(), CompareChars);
	std::sort(data->kerning.begin(), data->kerning.end(), CompareKerns);
	return true;
}
