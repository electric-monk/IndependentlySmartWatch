#include <FreeRTOS.h>
#include <string.h>
#include "hal_dma.h"
#include "Graphics.h"

typedef struct
{
	unsigned char Command;
	tLcdLine FrameBuffer[96];
} __attribute__((__packed__)) FullLcdCommand;

class FrameBufferBitmap : public Bitmap
{
private:
	static FrameBufferBitmap instance;

	FullLcdCommand LcdFramebuffer;
	unsigned short GraphicsDrawing, ScreenRevision, LastRevision;

	static void CheckScreen(int channel)
	{
		LcdDMAComplete(channel);
		if (instance.ScreenRevision != instance.LastRevision)
			instance.BlitScreen();
	}

	void BlitScreen(void)
	{
		LastRevision = ScreenRevision;
		WriteLcd((unsigned char*)&LcdFramebuffer, (int)sizeof(LcdFramebuffer), 1);
	}

	void Setup(void)
	{
		dataWidth = sizeof(LcdFramebuffer.FrameBuffer[0].Data) * 8;
		dataHeight = sizeof(LcdFramebuffer.FrameBuffer) / sizeof(LcdFramebuffer.FrameBuffer[0]);
		dataWidthSpan = sizeof(tLcdLine);
		dataBits = LcdFramebuffer.FrameBuffer[0].Data;

		GraphicsDrawing = 0;
		ScreenRevision = 0;
		LastRevision = 0;

		ClearLcd(1);

		memset(&LcdFramebuffer, 0, sizeof(LcdFramebuffer));
		LcdFramebuffer.Command = LCD_WRITE_CMD;
		for (int i = 0; i < dataHeight; i++)
			LcdFramebuffer.FrameBuffer[i].Row = FIRST_LCD_LINE_OFFSET + i;

		WaitForLcd();

		SetDMAHandler(2, FrameBufferBitmap::CheckScreen);
	}

public:
	static FrameBufferBitmap* Initialise(void)
	{
		instance.Setup();
		return &instance;
	}

	void Begin(void)
	{
		GraphicsDrawing++;
	}

	void End(void)
	{
		if ((--GraphicsDrawing) == 0)
		{
			ScreenRevision++;
			portDISABLE_INTERRUPTS();
			if (!LcdDmaBusy)
				BlitScreen();
			portENABLE_INTERRUPTS();
		}
	}
};

FrameBufferBitmap FrameBufferBitmap::instance;

Bitmap *FrameBuffer;

extern "C" void GraphicsInit(void)
{
	FrameBuffer = FrameBufferBitmap::Initialise();
}
