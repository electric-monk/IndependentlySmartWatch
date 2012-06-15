#ifndef __COLINDEBUG_H__
#define __COLINDEBUG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "portmacro.h"

extern char goodprintfbuf[100];
void goodsprintf(char *out, char *format, ...);

#ifdef _COLIN_DEBUG

// If this code is enabled for a release build, it'll
// continually reset on "Security violation".

#define goodprintf(format, ...)						\
{													\
	int intr = portSET_INTERRUPT_MASK_FROM_ISR();	\
	goodsprintf(goodprintfbuf, format, ## __VA_ARGS__);	\
	fputs(goodprintfbuf, stderr);					\
	portCLEAR_INTERRUPT_MASK_FROM_ISR(intr);		\
}

#else

#define goodprintf(format, ...)

#endif

#ifdef __cplusplus
}
#endif
#endif
