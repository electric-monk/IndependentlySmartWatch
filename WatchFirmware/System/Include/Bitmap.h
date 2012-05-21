#ifndef __BITMAP_H__
#define __BITMAP_H__

#define CHECK_COORD(i, range)	{ if (((i) < 0) || ((i) >= (range))) return; }

#define BLIT_INVERT			0x01
#define BLIT_OR				0x02
#define BLIT_PREINVERT		0x04
#define BLIT_POSTINVERT		0x08

class Bitmap
{
public:
	Bitmap(unsigned char width, unsigned char height, unsigned char span, const unsigned char *data);
	Bitmap(unsigned char width, unsigned char height, unsigned char span, unsigned char *data);
	virtual ~Bitmap();

	int Width(void) { return dataWidth; }
	int Height(void) { return dataHeight; }

	virtual void Begin(void);
	virtual void End(void);

	void Blit(const Bitmap *source, int x, int y, int flags = 0);
	void Blit(const Bitmap *source, int x, int y, int xoffset, int yoffset, int width, int height, int flags = 0);
	void WritePixel(int x, int y, bool set)
	{
		CHECK_COORD(x, dataWidth);
		CHECK_COORD(y, dataHeight);
		if (set) dataBits[(y * dataWidthSpan) + (x >> 3)] |= 1 << (x & 7);
		else dataBits[(y * dataWidthSpan) + (x >> 3)] &= ~(1 << (x & 7));
	}
	void Line(int x1, int y1, int x2, int y2, bool set);
	void LineRect(int x1, int y1, int x2, int y2, bool set);
	void FillRect(int x1, int y1, int x2, int y2, bool set);
	void Circle(int x0, int y0, int radius, bool set);

protected:
	Bitmap();

	unsigned char dataWidth, dataHeight;
	unsigned char dataWidthSpan;
	__attribute__((__packed__)) unsigned char *dataBits;

};

class DynamicBitmap : public Bitmap
{
public:
	DynamicBitmap(int width, int height);
	~DynamicBitmap();
};

#endif // __BITMAP_H__
