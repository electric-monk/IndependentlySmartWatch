#ifndef __COLINDEBUG_H__
#define __COLINDEBUG_H__

#include <stdio.h>

extern char goodprintfbuf[100];
void goodsprintf(char *out, char *format, ...);
#define goodprintf(format, ...)						\
{													\
	asm("    dint"); asm("    nop");				\
	goodsprintf(goodprintfbuf, format, ## __VA_ARGS__);	\
	fputs(goodprintfbuf, stderr);					\
	asm("    eint");								\
}

#endif
