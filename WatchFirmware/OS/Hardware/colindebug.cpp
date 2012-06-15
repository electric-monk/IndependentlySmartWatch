#include <stdarg.h>
#include "FixedMaths.h"

char goodprintfbuf[100];

static void printint(char **out, unsigned long value, int precision)
{
	unsigned long msb = value / 10;
	precision -= 1;
	if (msb == 0)
	{
		while (precision > 0)
		{
			*((*out)++) = '0';
			precision--;
		}
	}
	else
	{
		printint(out, msb, precision);
	}
	*((*out)++) = '0' + (value % 10);
}

static void printhex(char **out, unsigned long value, int precision)
{
	const char *hexy = "0123456789ABCDEF";
	unsigned long msb = value >> 8;
	precision -= 2;
	if (msb == 0)
	{
		while (precision > 0)
		{
			*((*out)++) = '0';
			precision--;
		}
	}
	else
	{
		printhex(out, msb, precision);
	}
	*((*out)++) = hexy[(value & 0xF0) >> 4];
	*((*out)++) = hexy[value & 0x0F];
}

static void goodvsprintf(char *out, char *format, va_list params)
{
	int lastPercent = 0;
	int precision;

	while (*format != '\0')
	{
		if (lastPercent)
		{
			switch (*format)
			{
				case 'u':
				{
					unsigned int bob = va_arg(params, unsigned int);
					printint(&out, bob, precision);
				}
					break;
				case 'i':
				{
					signed int bob = va_arg(params, signed int);
					if (bob < 0)
					{
						*(out++) = '-';
						bob = -bob;
					}
					printint(&out, bob, precision);
				}
					break;
				case 'l':
				{
					signed long bob = va_arg(params, signed long);
					if (bob < 0)
					{
						*(out++) = '-';
						bob = -bob;
					}
					printint(&out, bob, precision);
				}
					break;
				case 'x':
					printhex(&out, va_arg(params, unsigned int), precision);
					break;
				case 's':
				{
					char *ns = va_arg(params, char*);
					while (*ns != '\0')
						*(out++) = *(ns++);
				}
					break;
				case '%':
					*(out++) = '%';
					break;
				case 'F':
				{
					Fixed f = va_arg(params, Fixed);
					if (f < Fixed(0))
					{
						*(out++) = '-';
						f = -f;
					}
					long i = (long)f;
					f -= Fixed(i);
					printint(&out, i, 1);
					f *= Fixed(100);
					*(out++) = '.';
					printint(&out, (long)f, 2);
				}
					break;
				default:
					// Handle format info
					precision *= 10;
					precision += *format - '0';
					lastPercent++;
					break;
			}
			lastPercent--;
		}
		else
		{
			if (*format == '%')
			{
				lastPercent++;
				precision = 0;
			}
			else
				*(out++) = *format;
		}
		format++;
	}
	*out = '\0';
}

extern "C" void goodsprintf(char *out, char *format, ...)
{
	va_list params;

	va_start(params, format);
	goodvsprintf(out, format, params);
	va_end(params);
}
