#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "hal_lcd.h"

#ifdef __cplusplus

#include "Bitmap.h"

extern Bitmap *FrameBuffer;

extern "C" {
#endif

void GraphicsInit(void);

#ifdef __cplusplus
}
#endif

#endif // __GRAPHICS_H__
