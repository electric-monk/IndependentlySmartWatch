#ifndef __FIXEDMATHS_H__
#define __FIXEDMATHS_H__

// From http://wiki.yak.net/675/fixed.h

/*
Copyright (c) 2006 Henry Strickland & Ryan Seto

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

        (* http://www.opensource.org/licenses/mit-license.php *)
*/

typedef long FixedBaseType;

// It'd be good to turn this off to save some Flash, but required just now (for constants)
//#define ENABLE_FLOATS

class Fixed {

private:

	FixedBaseType	g; // the guts

const static int BP= 16;  // how many low bits are right of Binary Point
const static int BP2= BP*2;  // how many low bits are right of Binary Point
const static int BPhalf= BP/2;  // how many low bits are right of Binary Point

#ifdef ENABLE_FLOATS
static double STEP() { return 1.0 / (FixedBaseType(1)<<BP); }  // smallest step we can represent
#endif

public:
	enum FixedRaw { RAW };
	Fixed(FixedRaw, FixedBaseType guts) : g(guts) {}
	FixedBaseType Raw(void) const { return g; }

	Fixed() : g(0) {}
	Fixed(const Fixed& a) : g( a.g ) {}
#ifdef ENABLE_FLOATS
	Fixed(float a) : g( FixedBaseType(a / (float)STEP()) ) {}
	Fixed(double a) : g( FixedBaseType(a / (double)STEP()) ) {}
#endif
	Fixed(int a) : g( FixedBaseType(a) << BP ) {}
	Fixed(long a) : g( FixedBaseType(a) << BP ) {}
	Fixed& operator =(const Fixed& a) { g= a.g; return *this; }
#ifdef ENABLE_FLOATS
	Fixed& operator =(float a) { g= Fixed(a).g; return *this; }
	Fixed& operator =(double a) { g= Fixed(a).g; return *this; }
#endif
	Fixed& operator =(int a) { g= Fixed(a).g; return *this; }
	Fixed& operator =(long a) { g= Fixed(a).g; return *this; }

#ifdef ENABLE_FLOATS
	operator float() { return g * (float)STEP(); }
	operator double() { return g * (double)STEP(); }
#endif
	operator int() { return g>>BP; }
	operator long() { return g>>BP; }

	Fixed operator +() const { return Fixed(RAW,g); }
	Fixed operator -() const { return Fixed(RAW,-g); }

	Fixed operator +(const Fixed& a) const { return Fixed(RAW, g + a.g); }
	Fixed operator -(const Fixed& a) const { return Fixed(RAW, g - a.g); }
#if 1
	// more accurate, using long long
	Fixed operator *(const Fixed& a) const;
//	{
//		long long localG = g;
//		long long remoteG = a.g;
//		long long result = (localG * remoteG) >> BP;
//		return Fixed(RAW, FixedBaseType(result));
//	}
#else
	// faster, but with only half as many bits right of binary point
	Fixed operator *(const Fixed& a) const { return Fixed(RAW, (g>>BPhalf) * (a.g>>BPhalf) ); }
#endif
	Fixed operator /(const Fixed& a) const
	{
		long long localG = g;
		localG <<= BP2;
		long long remoteG = a.g;
		long long result = (localG / remoteG) >> BP;
		return Fixed(RAW, FixedBaseType(result));
	}

#ifdef ENABLE_FLOATS
	Fixed operator +(float a) const { return Fixed(RAW, g + Fixed(a).g); }
	Fixed operator -(float a) const { return Fixed(RAW, g - Fixed(a).g); }
	Fixed operator *(float a) const { return Fixed(RAW, (g>>BPhalf) * (Fixed(a).g>>BPhalf) ); }
	Fixed operator /(float a) const { return Fixed(RAW, FixedBaseType( (((long long)g << BP2) / (long long)(Fixed(a).g)) >> BP) ); }

	Fixed operator +(double a) const { return Fixed(RAW, g + Fixed(a).g); }
	Fixed operator -(double a) const { return Fixed(RAW, g - Fixed(a).g); }
	Fixed operator *(double a) const { return Fixed(RAW, (g>>BPhalf) * (Fixed(a).g>>BPhalf) ); }
	Fixed operator /(double a) const { return Fixed(RAW, FixedBaseType( (((long long)g << BP2) / (long long)(Fixed(a).g)) >> BP) ); }
#endif

	Fixed& operator +=(Fixed a) { *this = *this + a; return *this; }
	Fixed& operator -=(Fixed a) { *this = *this - a; return *this; }
	Fixed& operator *=(Fixed a) { *this = *this * a; return *this; }
	Fixed& operator /=(Fixed a) { *this = *this / a; return *this; }

	Fixed& operator +=(int a) { *this = *this + (Fixed)a; return *this; }
	Fixed& operator -=(int a) { *this = *this - (Fixed)a; return *this; }
	Fixed& operator *=(int a) { *this = *this * (Fixed)a; return *this; }
	Fixed& operator /=(int a) { *this = *this / (Fixed)a; return *this; }

	Fixed& operator +=(long a) { *this = *this + (Fixed)a; return *this; }
	Fixed& operator -=(long a) { *this = *this - (Fixed)a; return *this; }
	Fixed& operator *=(long a) { *this = *this * (Fixed)a; return *this; }
	Fixed& operator /=(long a) { *this = *this / (Fixed)a; return *this; }

#ifdef ENABLE_FLOATS
	Fixed& operator +=(float a) { *this = *this + a; return *this; }
	Fixed& operator -=(float a) { *this = *this - a; return *this; }
	Fixed& operator *=(float a) { *this = *this * a; return *this; }
	Fixed& operator /=(float a) { *this = *this / a; return *this; }

	Fixed& operator +=(double a) { *this = *this + a; return *this; }
	Fixed& operator -=(double a) { *this = *this - a; return *this; }
	Fixed& operator *=(double a) { *this = *this * a; return *this; }
	Fixed& operator /=(double a) { *this = *this / a; return *this; }
#endif

	bool operator ==(const Fixed& a) const { return g == a.g; }
	bool operator !=(const Fixed& a) const { return g != a.g; }
	bool operator <=(const Fixed& a) const { return g <= a.g; }
	bool operator >=(const Fixed& a) const { return g >= a.g; }
	bool operator  <(const Fixed& a) const { return g  < a.g; }
	bool operator  >(const Fixed& a) const { return g  > a.g; }

#ifdef ENABLE_FLOATS
	bool operator ==(float a) const { return g == Fixed(a).g; }
	bool operator !=(float a) const { return g != Fixed(a).g; }
	bool operator <=(float a) const { return g <= Fixed(a).g; }
	bool operator >=(float a) const { return g >= Fixed(a).g; }
	bool operator  <(float a) const { return g  < Fixed(a).g; }
	bool operator  >(float a) const { return g  > Fixed(a).g; }

	bool operator ==(double a) const { return g == Fixed(a).g; }
	bool operator !=(double a) const { return g != Fixed(a).g; }
	bool operator <=(double a) const { return g <= Fixed(a).g; }
	bool operator >=(double a) const { return g >= Fixed(a).g; }
	bool operator  <(double a) const { return g  < Fixed(a).g; }
	bool operator  >(double a) const { return g  > Fixed(a).g; }
#endif
};

#ifdef ENABLE_FLOATS
static inline Fixed operator +(float a, const Fixed& b) { return Fixed(a)+b; }
static inline Fixed operator -(float a, const Fixed& b) { return Fixed(a)-b; }
static inline Fixed operator *(float a, const Fixed& b) { return Fixed(a)*b; }
static inline Fixed operator /(float a, const Fixed& b) { return Fixed(a)/b; }

static inline bool operator ==(float a, const Fixed& b) { return Fixed(a) == b; }
static inline bool operator !=(float a, const Fixed& b) { return Fixed(a) != b; }
static inline bool operator <=(float a, const Fixed& b) { return Fixed(a) <= b; }
static inline bool operator >=(float a, const Fixed& b) { return Fixed(a) >= b; }
static inline bool operator  <(float a, const Fixed& b) { return Fixed(a)  < b; }
static inline bool operator  >(float a, const Fixed& b) { return Fixed(a)  > b; }



static inline Fixed operator +(double a, const Fixed& b) { return Fixed(a)+b; }
static inline Fixed operator -(double a, const Fixed& b) { return Fixed(a)-b; }
static inline Fixed operator *(double a, const Fixed& b) { return Fixed(a)*b; }
static inline Fixed operator /(double a, const Fixed& b) { return Fixed(a)/b; }

static inline bool operator ==(double a, const Fixed& b) { return Fixed(a) == b; }
static inline bool operator !=(double a, const Fixed& b) { return Fixed(a) != b; }
static inline bool operator <=(double a, const Fixed& b) { return Fixed(a) <= b; }
static inline bool operator >=(double a, const Fixed& b) { return Fixed(a) >= b; }
static inline bool operator  <(double a, const Fixed& b) { return Fixed(a)  < b; }
static inline bool operator  >(double a, const Fixed& b) { return Fixed(a)  > b; }
#endif

static inline int& operator +=(int& a, const Fixed& b) { a = (Fixed)a + b; return a; }
static inline int& operator -=(int& a, const Fixed& b) { a = (Fixed)a - b; return a; }
static inline int& operator *=(int& a, const Fixed& b) { a = (Fixed)a * b; return a; }
static inline int& operator /=(int& a, const Fixed& b) { a = (Fixed)a / b; return a; }

static inline long& operator +=(long& a, const Fixed& b) { a = (Fixed)a + b; return a; }
static inline long& operator -=(long& a, const Fixed& b) { a = (Fixed)a - b; return a; }
static inline long& operator *=(long& a, const Fixed& b) { a = (Fixed)a * b; return a; }
static inline long& operator /=(long& a, const Fixed& b) { a = (Fixed)a / b; return a; }

#ifdef ENABLE_FLOATS
static inline float& operator +=(float& a, const Fixed& b) { a = a + b; return a; }
static inline float& operator -=(float& a, const Fixed& b) { a = a - b; return a; }
static inline float& operator *=(float& a, const Fixed& b) { a = a * b; return a; }
static inline float& operator /=(float& a, const Fixed& b) { a = a / b; return a; }

static inline double& operator +=(double& a, const Fixed& b) { a = a + b; return a; }
static inline double& operator -=(double& a, const Fixed& b) { a = a - b; return a; }
static inline double& operator *=(double& a, const Fixed& b) { a = a * b; return a; }
static inline double& operator /=(double& a, const Fixed& b) { a = a / b; return a; }
#endif

// From http://www.ganssle.com/articles/atrig.htm

void StartMaths(void);

Fixed cosine(Fixed x);
Fixed sine(Fixed x);
Fixed tangent(Fixed x);

// Etc.
extern Fixed pi;

#endif // __FIXEDMATHS_H__
