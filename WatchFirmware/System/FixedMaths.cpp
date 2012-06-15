#include "FixedMaths.h"
#include "colindebug.h"

Fixed pi;

// From http://www.ganssle.com/articles/atrig.htm

static Fixed cos_p0;
static Fixed cos_p1;
static Fixed cos_p2;
static Fixed cos_p3;
static Fixed cos_p4;
static Fixed cos_p5;
static Fixed cos_pi2;

#ifdef ENABLE_FLOATS
#define DISPLAY_VALUES
#else
#define USE_FIXED_CONSTANTS
#endif

Fixed Fixed::operator *(const Fixed& a) const
{
	long long localG = g;
	long long remoteG = a.g;
	long long result = (localG * remoteG) >> BP;
	return Fixed(RAW, FixedBaseType(result));
}

void StartMaths(void)
{
#ifdef USE_FIXED_CONSTANTS
	pi = Fixed(Fixed::RAW, 205887);	// 3.14
	cos_p0 = Fixed(Fixed::RAW, 65536);	// 1.00
	cos_p1 = Fixed(Fixed::RAW, -32768);	// -0.50
	cos_p2 = Fixed(Fixed::RAW, 2730);	// 0.04
	cos_p3 = Fixed(Fixed::RAW, -91);	// -0.00
	cos_p4 = Fixed(Fixed::RAW, 1);	// 0.00
	cos_p5 = Fixed(Fixed::RAW, 0);	// 0.00
	cos_pi2 = Fixed(Fixed::RAW, 102943);	// 1.57
#else // USE_FIXED_CONSTANTS
	pi = 3.1415926535897932f;
	cos_p0 = 0.999999999781f;
	cos_p1 = -0.499999993585f;
	cos_p2 = 0.041666636258f;
	cos_p3 = -0.0013888361399f;
	cos_p4 = 0.00002476016134f;
	cos_p5 = -0.00000026051495f;
	cos_pi2 = pi / Fixed(2);
#ifdef DISPLAY_VALUES
#define REPORT(x)		goodprintf("\t" ## #x ## " = Fixed(Fixed::RAW, %l);\t// %F\n", x.Raw(), x)
	REPORT(pi);
	REPORT(cos_p0);
	REPORT(cos_p1);
	REPORT(cos_p2);
	REPORT(cos_p3);
	REPORT(cos_p4);
	REPORT(cos_p5);
	REPORT(cos_pi2);
	extern void ReportFixedAdcs(void);
	ReportFixedAdcs();
#endif // DISPLAY_VALUES
#endif // USE_FIXED_CONSTANTS
}

Fixed cosine(Fixed x)
{
	Fixed absx;
	if (x < Fixed(0))
		absx = -x;
	else
		absx = x;

	Fixed part = absx / cos_pi2;
	int quad = (int)part;
	Fixed frac = part - Fixed(quad);
	quad %= 4;

	Fixed t;
	switch (quad)
	{
		case 0:
			t = frac * cos_pi2;
			break;
		case 1:
			t = (Fixed(1) - frac) * cos_pi2;
			break;
		case 2:
			t = frac * cos_pi2;
			break;
		case 3:
			t = (frac - Fixed(1)) * cos_pi2;
			break;
	}
	t = t * t;

	Fixed y = cos_p0 + (cos_p1 * t) + (cos_p2 * t * t) + (cos_p3 * t * t * t) + (cos_p4 * t * t * t * t) + (cos_p5 * t * t * t * t * t);
	if ((quad == 1) || (quad == 2))
		return -y;
	else
		return y;
}

Fixed sine(Fixed x)
{
	return cosine(cos_pi2 - x);
}

Fixed tangent(Fixed x)
{
	return sine(x) / cosine(x);
}
