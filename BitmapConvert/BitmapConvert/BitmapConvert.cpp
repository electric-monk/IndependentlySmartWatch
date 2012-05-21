// BitmapConvert.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <IL/il.h>
#include <string.h>
#include "FontLoader.h"

#pragma comment(lib, "DevIL.lib")
#pragma warning(disable: 4996)

static bool GetPixel(int x, int y)
{
	unsigned char value;

	// Not very efficient, but this is just an offline tool
	if (ilGetInteger(IL_IMAGE_ORIGIN) == IL_ORIGIN_LOWER_LEFT)
		y = (ilGetInteger(IL_IMAGE_HEIGHT) - 1) - y;
	ilCopyPixels(x, y, 0, 1, 1, 1, IL_LUMINANCE, IL_UNSIGNED_BYTE, &value);
	return value > 128;
}

static bool IsRowEmpty(int width, int y)
{
	for (int x = 0; x < width; x++)
	{
		if (GetPixel(x, y))
			return false;
	}
	return true;
}

static bool IsColumnEmpty(int x, int height)
{
	for (int y = 0; y < height; y++)
	{
		if (GetPixel(x, y))
			return false;
	}
	return true;
}

static void SetBufferPixel(unsigned char *buffer, int spanWidth, int x, int y)
{
	buffer[(spanWidth * y) + (x / 8)] |= 1 << (x & 7);
}

static void ConvertImage(const TCHAR *image, const TCHAR *buffername, FILE *output, bool minimise, bool isPublic)
{
	ILuint ilImage;
	int width, height;
	int widthSpan;
	unsigned char *buf;

	ilGenImages(1, &ilImage);
	ilBindImage(ilImage);
	ilLoadImage(image);
	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	if (minimise)
	{
		while (IsRowEmpty(width, height - 1))
			height--;
		while (IsColumnEmpty(width - 1, height))
			width--;
	}
	widthSpan = width / 8;
	if ((width % 8) != 0)
		widthSpan++;
	int dataLength = widthSpan * height;
	buf = new unsigned char [dataLength];
	memset(buf, 0, dataLength);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			if (GetPixel(x, y))
				SetBufferPixel(buf, widthSpan, x, y);
	ilDeleteImages(1, &ilImage);
	// Generate bitmap array
	_ftprintf(output, _T("static const unsigned char bitmap_%s[]={"), buffername);
	for (int i = 0; i < dataLength; i++)
	{
		if ((i % 20) == 0)
			_ftprintf(output, _T("\n\t"));
		_ftprintf(output, _T("0x%.2x,"), buf[i]);
	}
	_ftprintf(output, _T("\n};\n"));
	delete[] buf;
	// Generate class definition
	_ftprintf(output, _T("%sBitmap %s(%i, %i, %i, bitmap_%s);\n\n"),
		isPublic ? _T("") : _T("static "),
		buffername, width, height, widthSpan, buffername);
}

static void MakeFontImageName(TCHAR *output, const TCHAR *buffername, int index)
{
	_stprintf(output, _T("bitmap_%s_%i"), buffername, index);
}

static void ConvertFont(const TCHAR *font, const TCHAR *buffername, FILE *output)
{
	FontLoader data(font);
	int bitmapRange = data.ImageMax();
	// Generate bitmaps
	for (int i = 0; i <= bitmapRange; i++)
	{
		const TCHAR *bitmapFilename = data.ImageAt(i);
		if (bitmapFilename == NULL)
			continue;
		TCHAR name[100];
		MakeFontImageName(name, buffername, i);
		ConvertImage(bitmapFilename, name, output, true, false);
	}
	// Generate array
	_ftprintf(output, _T("static Bitmap *glyphs_%s[] = {\n"), buffername);
	for (int i = 0; i <= bitmapRange; i++)
	{
		const TCHAR *bitmapFilename = data.ImageAt(i);
		if (bitmapFilename == NULL)
		{
			_ftprintf(output, _T("\tNULL,\n"));
			continue;
		}
		TCHAR name[100];
		MakeFontImageName(name, buffername, i);
		_ftprintf(output, _T("\t&%s,\n"), name);
	}
	_ftprintf(output, _T("};\n\n"));
	// Generate character data
	_ftprintf(output, _T("static const FontCharacter chars_%s[] = {\n"), buffername);
	bool emittedUnicode = false;
	for (std::vector<FontCharacter>::const_iterator it = data.Characters().begin(); it != data.Characters().end(); it++)
	{
		if (((*it).ch > 255) && (!emittedUnicode))
		{
			emittedUnicode = true;
			_ftprintf(output, _T("#ifdef ENABLE_UNICODE\n"));
		}
		_ftprintf(output, _T("\t{ %i, %i, %i, %i, %i, %i, %i, %i, %i },\n"),
			(*it).ch, (*it).x, (*it).y, (*it).width, (*it).height,
			(*it).xoffset, (*it).yoffset, (*it).xadvance, (*it).page);
	}
	if (emittedUnicode)
		_ftprintf(output, _T("#endif // ENABLE_UNICODE\n"));
	_ftprintf(output, _T("};\n\n"));
	// Generate kerning data
	_ftprintf(output, _T("static const FontKerning kerns_%s[] = {\n"), buffername);
	emittedUnicode = false;
	for (std::vector<FontKerning>::const_iterator it = data.Kernings().begin(); it != data.Kernings().end(); it++)
	{
		if (((*it).ch1 > 255) || ((*it).ch2 > 255))
		{
			if (!emittedUnicode)
			{
				emittedUnicode = true;
				_ftprintf(output, _T("#ifdef ENABLE_UNICODE\n"));
			}
		}
		else
		{
			if (emittedUnicode)
			{
				_ftprintf(output, _T("#endif // ENABLE_UNICODE\n"));
				emittedUnicode = false;
			}
		}
		_ftprintf(output, _T("\t{ %i, %i, %i },\n"),
			(*it).ch1, (*it).ch2, (*it).offset);
	}
	if (emittedUnicode)
		_ftprintf(output, _T("#endif // ENABLE_UNICODE\n"));
	_ftprintf(output, _T("};\n\n"));
	// Generate font information structure
//	_ftprintf(output, _T("static const FontInformation info_%s = { %i, %i, %i, %i };\n\n"), buffername,
//		data.LineHeight(), data.Baseline(), data.Characters().size(), data.Kernings().size());
	_ftprintf(output, _T("static const FontInformation info_%s = { %i, %i, sizeof(chars_%s) / sizeof(chars_%s[0]), sizeof(kerns_%s) / sizeof(kerns_%s[0]) };\n\n"), buffername,
		data.LineHeight(), data.Baseline(), buffername, buffername, buffername, buffername);
	// Generate font definition
	_ftprintf(output, _T("BitmapFont %s(&info_%s, &glyphs_%s[0], &chars_%s[0], &kerns_%s[0]);\n\n"),
		buffername, buffername, buffername, buffername, buffername);
}

static void GenerateHeader(FILE *output, const TCHAR *basename, const TCHAR *sourcefile)
{
	_ftprintf(output, _T("// %s\n// Generated from %s\n// This is a generated file. Do not modify manually.\n\n"), basename, sourcefile);
}

static FILE* GenerateSourceFile(const TCHAR *basename, const TCHAR *sourcefile)
{
	TCHAR filename[100];
	FILE *file;

	_stprintf(filename, _T("%s.cpp"), basename);
	file = _tfopen(filename, _T("wt"));
	GenerateHeader(file, filename, sourcefile);
	_ftprintf(file, _T("#include \"%s.h\"\n\n"), basename);
	return file;
}

static void GenerateHeaderFile(const TCHAR *basename, const TCHAR *sourcefile, const TCHAR *exportType, const TCHAR *exportName)
{
	TCHAR filename[100];
	FILE *file;

	_stprintf(filename, _T("%s.h"), basename);
	file = _tfopen(filename, _T("wt"));
	GenerateHeader(file, filename, sourcefile);
	_ftprintf(file, _T("#ifndef __generated_%s_%s\n#define __generated_%s_%s\n\n"), exportType, basename, exportType, basename);
	_ftprintf(file, _T("#include \"%s.h\"\n\n"), exportType);
	_ftprintf(file, _T("extern %s %s;\n\n"), exportType, exportName);
	_ftprintf(file, _T("#endif\n"));
	fclose(file);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ilInit();
	if (argc != 2)
	{
		_ftprintf(stderr, _T("This tool converts images and 'font' files into a format usable by the\n"));
		_ftprintf(stderr, _T("equivalent 'Font' and 'Bitmap' classes on the device. To make a .fnt file\n"));
		_ftprintf(stderr, _T("to input into this utility, use BMfont at\n"));
		_ftprintf(stderr, _T("http://www.angelcode.com/products/bmfont/\n\n"));
		_ftprintf(stderr, _T("Usage: %s <filename>\n\n"), argv[0]);
		return -1;
	}

	TCHAR filename[100], *type;
	_tcscpy(filename, argv[1]);

	int i = _tcslen(argv[1]);
	while ((i > 0) && (argv[1][i - 1] != '.'))
		i--;

	filename[i - 1] = '\0';
	FILE *source = GenerateSourceFile(filename, argv[1]);
	if (_tcsicmp(argv[1] + i, _T("fnt")) == 0)
	{
		type = _T("BitmapFont");
		// Convert font
		ConvertFont(argv[1], filename, source);
	}
	else
	{
		type = _T("Bitmap");
		// Convert image
		ConvertImage(argv[1], filename, source, false, true);
	}
	fclose(source);
	GenerateHeaderFile(filename, argv[1], type, filename);
	return 0;
}
