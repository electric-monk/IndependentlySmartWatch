#include <string.h>
#include "Graphics.h"
#include "Bitmap.h"

Bitmap::Bitmap(unsigned char width, unsigned char height, unsigned char span, unsigned char *data)
{
	dataWidth = width;
	dataHeight = height;
	dataWidthSpan = span;
	dataBits = data;
}

Bitmap::Bitmap(unsigned char width, unsigned char height, unsigned char span, const unsigned char *data)
{
	// This constructor is only for in-ROM bitmaps, as it does not enforce the const-ness of the bitmap.
	dataWidth = width;
	dataHeight = height;
	dataWidthSpan = span;
	dataBits = (unsigned char*)data;
}

Bitmap::~Bitmap()
{
	// Nothing to do
}

Bitmap::Bitmap()
{
	// No init required
}

void Bitmap::Begin(void)
{
	// Nothing to do
}

void Bitmap::End(void)
{
	// Nothing to do
}

void Bitmap::Blit(const Bitmap *destination, int x, int y, int flags)
{
	Blit(destination, x, y, 0, 0, dataWidth, dataHeight, flags);
}

static __inline unsigned char ReadBitmap(const unsigned char *bits, int pixelOffset)
{
	if (pixelOffset == 0)
		return *bits;
	return (*bits >> pixelOffset) | (*(bits + 1) << (8 - pixelOffset));
}

static __inline void WriteBitmap(unsigned char *bits, int pixelOffset, unsigned char data, int bitsCount, int flags)
{
	unsigned char mask;

	if (flags & BLIT_INVERT)
		data = ~data;

	// Compute mask
	mask = 0;
	while (bitsCount > 0)
	{
		mask = (mask << 1) | 1;
		if (mask & 0x80)
			break;
		bitsCount--;
	}

	// Only the data we need
	data &= mask;

	// Shortcut
	if (pixelOffset == 0)
	{
		if (!(flags & BLIT_OR))
			*bits &= ~mask;
		else if (flags & BLIT_PREINVERT)
			*bits ^= mask;
		*bits = (*bits & ~mask) | data;
		if (flags & BLIT_POSTINVERT)
			*bits ^= mask;
		return;
	}

	// Mask in
	if (!(flags & BLIT_OR))
		*bits &= ~(mask << pixelOffset);
	else if (flags & BLIT_PREINVERT)
		*bits ^= mask << pixelOffset;
	*bits |= data << pixelOffset;
	if (flags & BLIT_POSTINVERT)
		*bits ^= mask << pixelOffset;
	bits++;
	int antiPixelOffset = 8 - pixelOffset;
	if (!(flags & BLIT_OR))
		*bits &= ~(mask >> antiPixelOffset);
	else if (flags & BLIT_PREINVERT)
		*bits ^= mask >> antiPixelOffset;
	*bits |= data >> antiPixelOffset;
	if (flags & BLIT_POSTINVERT)
		*bits ^= mask >> antiPixelOffset;
}

void Bitmap::Blit(const Bitmap *source, int x, int y, int xoffset, int yoffset, int width, int height, int flags)
{
	// Validate the input
	if (x < 0)
	{
		xoffset -= x;
		x = 0;
	}
	if (y < 0)
	{
		yoffset -= y;
		y = 0;
	}
	if (xoffset < 0)
	{
		x -= xoffset;
		xoffset = 0;
	}
	if (yoffset < 0)
	{
		y -= yoffset;
		yoffset = 0;
	}
	if ((xoffset + width) > source->dataWidth)
		width = source->dataWidth - xoffset;
	if ((yoffset + height) > source->dataHeight)
		height = source->dataHeight - yoffset;
	if ((x + width) > dataWidth)
		width = dataWidth - x;
	if ((y + height) > dataHeight)
		height = dataHeight - y;
	if ((width <= 0) || (height <= 0))
		return;
	if ((x >= dataWidth) || (y >= dataHeight))
		return;

	// Calculate source copy
	int sourceDataWidthSpan = source->dataWidthSpan;
	unsigned char *bitmapRow = source->dataBits + (yoffset * sourceDataWidthSpan) + (xoffset >> 3);
	int destDataWidthSpan = dataWidthSpan;
	unsigned char *screenRow = dataBits + (y * destDataWidthSpan) + (x >> 3);

	// Do the copy
	Begin();
	for (int cy = 0; cy < height; cy++)
	{
		unsigned char *bitmapColumn = bitmapRow;
		unsigned char *screenColumn = screenRow;
		for (int cx = 0; cx < width; cx += 8)
		{
			unsigned char data = ReadBitmap(bitmapColumn, xoffset & 7);
			WriteBitmap(screenColumn, x & 7, data, width - cx, flags);

			bitmapColumn++;
			screenColumn++;
		}
		bitmapRow += sourceDataWidthSpan;
		screenRow += destDataWidthSpan;
	}
	End();
}

void Bitmap::LineRect(int x1, int y1, int x2, int y2, bool set)
{
	Line(x1, y1, x2, y1, set);
	Line(x1, y1, x1, y2, set);
	Line(x2, y1, x2, y2, set);
	Line(x1, y2, x2, y1, set);
}

void Bitmap::FillRect(int x1, int y1, int x2, int y2, bool set)
{
	int x, y, width, height;

	if (x1 > x2)
	{
		x = x2;
		width = x1 - x2;
	}
	else
	{
		x = x1;
		width = x2 - x1;
	}
	if (y1 > y2)
	{
		y = y2;
		height = y1 - y2;
	}
	else
	{
		y = y1;
		height = y2 - y1;
	}

	// Validate the input
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if ((x + width) > dataWidth)
		width = dataWidth - x;
	if ((y + height) > dataHeight)
		height = dataHeight - y;
	if ((width <= 0) || (height <= 0))
		return;

	// Calculate source copy
	unsigned char *screenRow = dataBits + (y * dataWidthSpan) + (x >> 3);
	unsigned char data = set ? 0xFF : 0x00;

	// Do the copy
	Begin();
	for (int cy = 0; cy < height; cy++)
	{
		unsigned char *screenColumn = screenRow;
		for (int cx = 0; cx < width; cx += 8)
		{
			WriteBitmap(screenColumn, x & 7, data, width - cx, 0);

			screenColumn++;
		}
		screenRow += dataWidthSpan;
	}
	End();
}

// Bresenham Algorithm (from Wikipedia)
void Bitmap::Line(int x1, int y1, int x2, int y2, bool set)
{
	int dx, dy, sx, sy, err;

	dx = x2 - x1;
	if (dx < 0) dx = -dx;
	dy = y2 - y1;
	if (dy < 0) dy = -dy;

	if (x1 < x2) sx = 1;
	else sx = -1;
	if (y1 < y2) sy = 1;
	else sy = -1;

	err = dx - dy;

	Begin();
	while (true) {
		WritePixel(x1, y1, set);
		if ((x1 == x2) && (y1 == y2))
			break;
		int e2 = 2 * err;
		if (e2 > -dy) {
			err = err - dy;
			x1 = x1 + sx;
		}
		if (e2 < dx) {
			err = err + dx;
			y1 = y1 + sy;
		}
	}
	End();
}

// Midpoint circle algorithm (from Wikipedia)
void Bitmap::Circle(int x0, int y0, int radius, bool set)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	WritePixel(x0, y0 + radius, set);
	WritePixel(x0, y0 - radius, set);
	WritePixel(x0 + radius, y0, set);
	WritePixel(x0 - radius, y0, set);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		WritePixel(x0 + x, y0 + y, set);
		WritePixel(x0 - x, y0 + y, set);
		WritePixel(x0 + x, y0 - y, set);
		WritePixel(x0 - x, y0 - y, set);
		WritePixel(x0 + y, y0 + x, set);
		WritePixel(x0 - y, y0 + x, set);
		WritePixel(x0 + y, y0 - x, set);
		WritePixel(x0 - y, y0 - x, set);
	}
}

DynamicBitmap::DynamicBitmap(int width, int height)
{
	dataWidth = width;
	dataHeight = height;

	dataWidthSpan = dataWidth / 8;
	if ((dataWidth % 8) != 0)
		dataWidthSpan++;

	int totalMemory = dataWidthSpan * dataHeight;
	dataBits = new unsigned char[totalMemory];
	memset(dataBits, 0, totalMemory);
}

DynamicBitmap::~DynamicBitmap()
{
	delete[] dataBits;
}
