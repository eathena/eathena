#ifndef __BASEMATH_H__
#define __BASEMATH_H__

#include "basetypes.h"
#include "basebooltype.h"
#include "basebits.h"
#include "basepair.h"

///////////////////////////////////////////////////////////////////////////////
// irregular part and missing functions
#if defined(_MSC_VER) || defined(__BORLANDC__)
///////////////////////////////////////////////////////////////////////////////

inline int isfinite(double x)							{ return _finite(x); }
inline int isnan(double x)								{ return _isnan(x); }

inline float hypotf(float x, float y)					{ return (float)_hypot((float)x,(float)y); }
inline double hypot(double x, double y)					{ return _hypot(x,y); }
inline long double hypotl(long double x, long double y)	{ return (long double)_hypot((long double)x,(long double)y); }

///////////////////////////////////////////////////////////////////////////////
#elif defined(__sun__)
///////////////////////////////////////////////////////////////////////////////

#include <ieeefp.h>
#define fpclassify fpclass
inline int isfinite(double x)		{ return finite(x); }
inline int isnan(float x)			{ return isnanf(x); }
inline int isnan(double x)			{ return isnand(x); }

#if defined(__GNUC__) 
// sunmath.h does not exist for gcc
// manually rearrange the math_iso.h things [marc]
extern "C" {
float __acosf(float);
float __asinf(float);
float __atanf(float);
float __atan2f(float, float);
float __ceilf(float);
float __cosf(float);
float __coshf(float);
float __expf(float);
float __fabsf(float);
float __floorf(float);
float __fmodf(float, float);
float __frexpf(float, int *);
float __ldexpf(float, int);
float __logf(float);
float __log10f(float);
float __modff(float, float *);
float __powf(float, float);
float __sinf(float);
float __sinhf(float);
float __sqrtf(float);
float __tanf(float);
float __tanhf(float);

long double __acosl(long double);
long double __asinl(long double);
long double __atanl(long double);
long double __atan2l(long double, long double);
long double __ceill(long double);
long double __cosl(long double);
long double __coshl(long double);
long double __expl(long double);
long double __fabsl(long double);
long double __floorl(long double);
long double __fmodl(long double, long double);
long double __frexpl(long double, int *);
long double __ldexpl(long double, int);
long double __logl(long double);
long double __log10l(long double);
long double __modfl(long double, long double *);
long double __powl(long double, long double);
long double __sinl(long double);
long double __sinhl(long double);
long double __sqrtl(long double);
long double __tanl(long double);
long double __tanhl(long double);
}// extern "C"

extern "C" {
inline float acosf(float x)								{ return __acosf(x); }
inline float asinf(float x)								{ return __asinf(x); }
inline float atanf(float x)								{ return __atanf(x); }
inline float atan2f(float x, float y)					{ return __atan2f(x,y); }
inline float ceilf(float x)								{ return __ceilf(x); }
inline float cosf(float x)								{ return __cosf(x); }
inline float coshf(float x)								{ return __coshf(x); }
inline float expf(float x)								{ return __expf(x); }
inline float fabsf(float x)								{ return __fabsf(x); }
inline float floorf(float x)							{ return __floorf(x); }
inline float fmodf(float x, float y)					{ return __fmodf(x,y); }
inline float frexpf(float x, int *e)					{ return __frexpf(x,e); }
inline float hypotf(float x, float y)					{ return (float)hypot((float)x, (float)y); }
inline float ldexpf(float v, int e)						{ return __ldexpf(v,e); }
inline float logf(float x)								{ return __logf(x); }
inline float log10f(float x)							{ return __log10f(x); }
inline float modff(float x, float *i)					{ return __modff(x,i); }
inline float powf(float x, float y)						{ return __powf(x,y); }
inline float sinf(float x)								{ return __sinf(x); }
inline float sinhf(float x)								{ return __sinhf(x); }
inline float sqrtf(float x)								{ return __sqrtf(x); }
inline float tanf(float x)								{ return __tanf(x); }
inline float tanhf(float x)								{ return __tanhf(x); }

inline long double acosl(long double x)					{ return __acosl(x); }
inline long double asinl(long double x)					{ return __asinl(x); }
inline long double atanl(long double x)					{ return __atanl(x); }
inline long double atan2l(long double x, long double y)	{ return __atan2l(x,y); }
inline long double ceill(long double x)					{ return __ceill(x); }
inline long double cosl(long double x)					{ return __cosl(x); }
inline long double coshl(long double x)					{ return __coshl(x); }
inline long double expl(long double x)					{ return __expl(x); }
inline long double fabsl(long double x)					{ return __fabsl(x); }
inline long double floorl(long double x)				{ return __floorl(x); }
inline long double fmodl(long double x, long double y)	{ return __fmodl(x,y); }
inline long double frexpl(long double x, int *e)		{ return __frexpl(x,e); }
inline long double hypotl(long double x, long double y)	{ return (long double)hypot((double)x, (double)y); }
inline long double ldexpl(long double v, int e)			{ return __ldexpl(v,e); }
inline long double logl(long double x)					{ return __logl(x); }
inline long double log10l(long double x)				{ return __log10l(x); }
inline long double modfl(long double x, long double *i)	{ return __modfl(x,i); }
inline long double powl(long double x, long double y)	{ return __powl(x,y); }
inline long double sinl(long double x)					{ return __sinl(x); }
inline long double sinhl(long double x)					{ return __sinhl(x); }
inline long double sqrtl(long double x)					{ return __sqrtl(x); }
inline long double tanl(long double x)					{ return __tanl(x); }
inline long double tanhl(long double x)					{ return __tanhl(x); }
}// extern "C"

#else//!defined(__GNUC__) 
// use sunmath for native compilers
#include <sunmath.h>
#endif// !defined(__GNUC__) 

///////////////////////////////////////////////////////////////////////////////
#elif defined(__alpha) && !defined (__linux)
///////////////////////////////////////////////////////////////////////////////

#include <nan.h>

///////////////////////////////////////////////////////////////////////////////
#elif defined(__CYGWIN__) 
///////////////////////////////////////////////////////////////////////////////

#undef isfinite
#if defined(fpclassify)
#define isfinite(x) (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#else
#define isfinite(x) ((x) - (x) == 0)
#endif// !defined(fpclassify)

#undef isnan
#if !defined(fpclassify)
#define isnan(x) ((x) != (x))
#else
#define isnan(x) (fpclassify(x) == FP_NAN)
#endif// !defined(fpclassify)

// cygwin has no standard long double math functions
// manually add replacements
extern "C" {
inline long double acosl(long double x)					{ return (long double)acos((double)x); }
inline long double asinl(long double x)					{ return (long double)asin((double)x); }
inline long double atanl(long double x)					{ return (long double)atan((double)x); }
inline long double atan2l(long double x, long double y)	{ return (long double)atan2((double)x,(double)y); }
inline long double ceill(long double x)					{ return (long double)ceil((double)x); }
inline long double cosl(long double x)					{ return (long double)cos((double)x); }
inline long double coshl(long double x)					{ return (long double)cosh((double)x); }
inline long double expl(long double x)					{ return (long double)exp((double)x); }
inline long double fabsl(long double x)					{ return (long double)fabs((double)x); }
inline long double floorl(long double x)				{ return (long double)floor((double)x); }
inline long double fmodl(long double x, long double y)	{ return (long double)fmod((double)x,(double)y); }
inline long double frexpl(long double x, int *e)		{ return (long double)frexpl((double)x,e); }
inline long double hypotl(long double x, long double y)	{ return (long double)hypot((double)x, (double)y); }
inline long double ldexpl(long double v, int e)			{ return (long double)ldexp((double)v,(double)e); }
inline long double logl(long double x)					{ return (long double)log((double)x); }
inline long double log10l(long double x)				{ return (long double)log10((double)x); }
inline long double modfl(long double x, long double *i)	{ double tmp; double ret= modf((double)x,&tmp); *i=(long double)tmp; return (long double)ret; }
inline long double powl(long double x, long double y)	{ return (long double)pow((double)x,(double)y); }
inline long double sinl(long double x)					{ return (long double)sin((double)x); }
inline long double sinhl(long double x)					{ return (long double)sinh((double)x); }
inline long double sqrtl(long double x)					{ return (long double)sqrt((double)x); }
inline long double tanl(long double x)					{ return (long double)tan((double)x); }
inline long double tanhl(long double x)					{ return (long double)tanh((double)x); }
}// extern "C"

///////////////////////////////////////////////////////////////////////////////
#else// others
///////////////////////////////////////////////////////////////////////////////

// When isfinite is not available, try to use one of the alternatives, or bail out.
#if !defined(isfinite)
#if defined(fpclassify)
#define isfinite(x) (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#else
#define isfinite(x) ((x) - (x) == 0)
#endif// !defined(fpclassify)
#endif// !defined(isfinite)

#if !defined(isnan)
#if !defined(fpclassify)
#define isnan(x) ((x) != (x))
#else
#define isnan(x) (fpclassify(x) == FP_NAN)
#endif// !defined(fpclassify)
#endif// !defined(isnan)

///////////////////////////////////////////////////////////////////////////////
#endif//
///////////////////////////////////////////////////////////////////////////////


NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
//@{
/// check for void
template <typename T>
struct is_void
{
	enum _dummy { Result = false };
	typedef bool_false Type;
};

template<>
struct is_void<void>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
};
//@}

///////////////////////////////////////////////////////////////////////////////
//@{
/// check for integer types
template <typename T>
struct is_integral
{
	enum _dummy { Result = false };
	typedef bool_false Type;
	typedef bool_false integral;
	typedef bool_false issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<bool>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_false issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<char>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_true issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<signed char>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_true issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<unsigned char>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_false issigned;
	typedef bool_true isunsigned;
};

#if !defined(WCHAR_T_IS_USHORT)
template<>
struct is_integral<wchar_t>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_false issigned;
	typedef bool_true isunsigned;
};
#endif

template<>
struct is_integral<short>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_true issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<unsigned short>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_false issigned;
	typedef bool_true isunsigned;
};

template<>
struct is_integral<int>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_true issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<unsigned int>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_false issigned;
	typedef bool_true isunsigned;
};

template<>
struct is_integral<long>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_true issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<unsigned long>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_false issigned;
	typedef bool_true isunsigned;
};

template<>
struct is_integral<sint64>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_true issigned;
	typedef bool_false isunsigned;
};

template<>
struct is_integral<uint64>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true integral;
	typedef bool_false issigned;
	typedef bool_true isunsigned;
};
//@}

///////////////////////////////////////////////////////////////////////////////
//@{
/// check for rational types
template <typename T>
struct is_rational
{
	enum _dummy { Result = false };
	typedef bool_false Type;
	typedef bool_false rational;
};

template<>
struct is_rational<float>
{
	enum _dummy { Result = true };
	typedef bool_false Type;
	typedef bool_true rational;
};

template<>
struct is_rational<double>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true rational;
};

template<>
struct is_rational<long double>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true rational;
};
//@}

template<typename T>
struct is_float
{
	enum _dummy { Result = false };
	typedef bool_false Type;
};
template<>
struct is_float<float>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
};
template<typename T>
struct is_double
{
	enum _dummy { Result = false };
	typedef bool_false Type;
};
template<>
struct is_double<double>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
};
template<typename T>
struct is_longdouble
{
	enum _dummy { Result = false };
	typedef bool_false Type;
};
template<>
struct is_longdouble<long double>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
};

///////////////////////////////////////////////////////////////////////////////
///
template<typename T>
struct is_arithmetic
{
	typedef typename logic_or<typename is_integral<T>::Type, typename is_rational<T>::Type>::Type Type;
	enum _dummy { Result = Type::Result };
};

///////////////////////////////////////////////////////////////////////////////
//@{
/// check for signed type
template<typename T>
struct is_signed
{
	enum _dummy { Result = false };
	typedef bool_false Type;
	typedef bool_false issigned;
};
template<>
struct is_signed<signed char>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true issigned;
};
template<>
struct is_signed<signed short>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true issigned;
};
template<>
struct is_signed<signed int>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true issigned;
};
template<>
struct is_signed<signed long>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true issigned;
};
template<>
struct is_signed<sint64>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true issigned;
};
//@}

///////////////////////////////////////////////////////////////////////////////
//@{
/// check for unsigned types
template<typename T>
struct is_unsigned
{
	enum _dummy { Result = false };
	typedef bool_false Type;
	typedef bool_false isunsigned;
};
template<>
struct is_unsigned<unsigned char>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true isunsigned;
};
template<>
struct is_unsigned<unsigned short>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true isunsigned;
};
template<>
struct is_unsigned<unsigned int>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true isunsigned;
};
template<>
struct is_unsigned<unsigned long>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true isunsigned;
};
template<>
struct is_unsigned<uint64>
{
	enum _dummy { Result = true };
	typedef bool_true Type;
	typedef bool_true isunsigned;
};
//@}


///////////////////////////////////////////////////////////////////////////////
//@{
/// return an appropeate signed type
template<typename T>
struct signed_type
{
	typedef T Type;
};
template<>
struct signed_type<unsigned char>
{
	typedef signed char Type;
};
template<>
struct signed_type<unsigned short>
{
	typedef signed short Type;
};
template<>
struct signed_type<unsigned int>
{
	typedef signed int Type;
};
template<>
struct signed_type<unsigned long>
{
	typedef signed long Type;
};
template<>
struct signed_type<uint64>
{
	typedef sint64 Type;
};
//@}

///////////////////////////////////////////////////////////////////////////////
//@{
/// return an appropeate unsigned type
template<typename T>
struct unsigned_type
{
	typedef T Type;
};
template<>
struct unsigned_type<signed char>
{
	typedef unsigned char Type;
};
template<>
struct unsigned_type<signed short>
{
	typedef unsigned short Type;
};
template<>
struct unsigned_type<signed int>
{
	typedef unsigned int Type;
};
template<>
struct unsigned_type<signed long>
{
	typedef unsigned long Type;
};
template<>
struct unsigned_type<sint64>
{
	typedef uint64 Type;
};
//@}


///////////////////////////////////////////////////////////////////////////////
/// functional conversion overloads.
/// to change signed types to the appropriate unsigned
inline unsigned char to_unsigned(char t)
{
	return (unsigned char)(t);
}
inline unsigned char to_unsigned(unsigned char t)
{
	return (unsigned char)(t);
}
// UCT2
inline unsigned short to_unsigned(short t)
{
	return (unsigned short)(t);
}
inline unsigned short to_unsigned(unsigned short t)
{
	return (unsigned short)(t);
}
// others, just to be complete
inline unsigned int to_unsigned(int t)
{
	return (unsigned int)(t);
}
inline unsigned int to_unsigned(unsigned int t)
{
	return (unsigned int)(t);
}
inline unsigned long to_unsigned(long t)
{
	return (unsigned long)(t);
}
inline unsigned long to_unsigned(unsigned long t)
{
	return (unsigned long)(t);
}
inline uint64 to_unsigned(int64 t)
{
	return (uint64)(t);
}
inline uint64 to_unsigned(uint64 t)
{
	return (uint64)(t);
}
inline float to_unsigned(float t)
{
	return t;
}
inline double to_unsigned(double t)
{
	return t;
}
inline long double to_unsigned(long double t)
{
	return t;
}
///////////////////////////////////////////////////////////////////////////////
/// functional conversion overloads.
/// to change unsigned types to the appropriate signed
inline signed char to_signed(char t)
{
	return (signed char)(t);
}
inline signed char to_signed(unsigned char t)
{
	return (signed char)(t);
}
// UCT2
inline signed short to_signed(short t)
{
	return (signed short)(t);
}
inline signed short to_signed(unsigned short t)
{
	return (signed short)(t);
}
// others, just to be complete
inline signed int to_signed(int t)
{
	return (signed int)(t);
}
inline signed int to_signed(unsigned int t)
{
	return (signed int)(t);
}
inline signed long to_signed(long t)
{
	return (signed long)(t);
}
inline signed long to_signed(unsigned long t)
{
	return (signed long)(t);
}
inline sint64 to_signed(int64 t)
{
	return (sint64)(t);
}
inline sint64 to_signed(uint64 t)
{
	return (sint64)(t);
}
inline float to_signed(float t)
{
	return t;
}
inline double to_signed(double t)
{
	return t;
}
inline long double to_signed(long double t)
{
	return t;
}
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// relops.
//template<typename T> inline bool operator!=(const T& x, const T& y)	{ return !(x == y); }
//template<typename T> inline bool operator> (const T& x, const T& y)	{ return  (y <  x); }
//template<typename T> inline bool operator<=(const T& x, const T& y)	{ return !(y <  x); }
//template<typename T> inline bool operator>=(const T& x, const T& y)	{ return !(x <  y); }


///////////////////////////////////////////////////////////////////////////////
/// note that these are defines and do not stop at namespace borders

/// definition of PI.
#ifndef PI
#define PI 3.14159265358979323846264338327950288L
#endif
/// natural logarithm of 10.
#ifndef LN10
#define LN10 2.3025850929940456840179914546844L
#endif

///////////////////////////////////////
namespace math {
///////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// epsilon for real comparison.
const float REAL_EPSILON		=((float)1e-9);


///////////////////////////////////////////////////////////////////////////////
/// radiant to degree
template<typename T>
inline T to_degrees(T radians) { return radians*((T)57.2957795130823208767981548141052); }
///////////////////////////////////////////////////////////////////////////////
/// degree to radiant
template<typename T>
inline T to_radians(T degrees) { return degrees*((T)0.0174532925199432957692369076848861); }


///////////////////////////////////////////////////////////////////////////////
/// templates for absolute value.
/// use existing functions, otherwise do comparison and negation
template<typename T> inline T abs(const T& x)							{ return (x<T()) ? -x : x; }
template<> inline int abs<int>(const int& x)							{ return ::abs(x); }
template<> inline long abs<long>(const long& x)							{ return ::labs(x); }
template<> inline float abs<float>(const float& x)						{ return ::fabsf(x); }
template<> inline double abs<double>(const double& x)					{ return ::fabs(x); }
template<> inline long double abs<long double>(const long double& x)	{ return ::fabsl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for arcus cosine.
inline float acos(const float x)										{ return ::acosf(x); }
inline double acos(const double x)										{ return ::acos(x); }
inline long double acos(const long double x)							{ return ::acosl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for arcus sine
inline float asin(const float x)										{ return ::asinf(x); }
inline double asin(const double x)										{ return ::asin(x); }
inline long double asin(const long double x)							{ return ::asinl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for arcus tangent and tangent2
inline float atan(const float x)										{ return ::atanf(x); }
inline double atan(const double x)										{ return ::atan(x); }
inline long double atan(const long double x)							{ return ::atanl(x); }
inline float atan(const float x, const float y)							{ return ::atan2f(x,y); }
inline double atan(const double x, const double y)						{ return ::atan2(x,y); }
inline long double atan(const long double x, const long double y)		{ return ::atan2l(x,y); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for ceiling
inline float ceil(const float x)										{ return ::ceilf(x); }
inline double ceil(const double x)										{ return ::ceil(x); }
inline long double ceil(const long double x)							{ return ::ceill(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for cosine
inline float cos(const float x)											{ return ::cosf(x); }
inline double cos(const double x)										{ return ::cos(x); }
inline long double cos(const long double x)								{ return ::cosl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for hyperbolic cosine 
inline float cosh(const float x)										{ return ::coshf(x); }
inline double cosh(const double x)										{ return ::cosh(x); }
inline long double cosh(const long double x)							{ return ::coshl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for exponent naturalis
inline double exp(const int x)											{ return ::exp((double)x); }
inline float exp(const float x)											{ return ::expf(x); }
inline double exp(const double x)										{ return ::exp(x); }
inline long double exp(const long double x)								{ return ::expl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for exponent base10
inline uint64 exp10(const uint x)										{ return pow10(x); }
inline float exp10(const float x)										{ return ::expf(x*LN10); }
inline double exp10(const double x)										{ return ::exp(x*LN10); }
inline long double exp10(const long double x)							{ return ::expl(x*LN10); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for floor
inline float floor(const float x)										{ return ::floorf(x); }
inline double floor(const double x)										{ return ::floor(x); }
inline long double floor(const long double x)							{ return ::floorl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for fmod
inline float fmod(const float x, const float y)							{ return ::fmodf(x,y); }
inline double fmod(const double x, const double y)						{ return ::fmod(x,y); }
inline long double fmod(const long double x, const long double y)		{ return ::fmodl(x,y); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for frexp. note that exponent is given as reference, not as pointer
inline float frexp(const float x, int& y)								{ return ::frexpf(x,&y); }
inline double frexp(const double x, int& y)								{ return ::frexp(x,&y); }
inline long double frexp(const long double x, int& y)					{ return ::frexpl(x,&y); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for frexp. returning a pair
template<typename T> inline pair<T,int> frexp(const T& x)				{ int itmp; pair<T,int> ptmp( frexp<T>(x,&itmp),0); ptmp.second=itmp; return ptmp; }

///////////////////////////////////////////////////////////////////////////////
/// overloads for ldexp
inline float ldexp(const float x, const int y)							{ return ::ldexpf(x,y); }
inline double ldexp(const double x, const int y)						{ return ::ldexp(x,y); }
inline long double ldexp(const long double x, const int y)				{ return ::ldexpl(x,y); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for hypotenuse
inline float hypot(const float x, const float y)						{ return ::hypotf(x,y); }
inline double hypot(const double x, const double y)						{ return ::hypot(x,y); }
inline long double hypot(const long double x, const long double y)		{ return ::hypotl(x,y); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for logarithm naturalis
inline float log(const float x)											{ return ::logf(x); }
inline double log(const double x)										{ return ::log(x); }
inline long double log(const long double x)								{ return ::logl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for logarithm base 10
inline float log10(const float x)										{ return ::log10f(x); }
inline double log10(const double x)										{ return ::log10(x); }
inline long double log10(const long double x)							{ return ::log10l(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for modf. note that integer part is given as reference, not as pointer
inline float modf(const float x, float& y)								{ return ::modff(x,&y); }
inline double modf(const double x, double& y)							{ return ::modf(x,&y); }
inline long double modf(const long double x, long double& y)			{ return ::modfl(x,&y); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for modf. returning a pair
template<typename T> inline pair<T,T> modf(const T& x)					{ return pair<T,T>(x,T()); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for pow
inline float pow(const float x, const float y)							{ return ::powf(x,y); }
inline double pow(const double x, const double y)						{ return ::pow(x,y); }
inline long double pow(const long double x, const long double y)		{ return ::powl(x,y); }
template<typename T>
T pow(T x, unsigned int n)
{
	T y = (n%2)?x:1;
	while ( n>>=1 )
	{
		x = x * x;
		if( n%2 )
			y = y * x;
	}
	return y;
}

///////////////////////////////////////////////////////////////////////////////
/// overloads for sine
inline float sin(const float x)											{ return ::sinf(x); }
inline double sin(const double x)										{ return ::sin(x); }
inline long double sin(const long double x)								{ return ::sinl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for hyperbolic sine
inline float sinh(const float x)										{ return ::sinhf(x); }
inline double sinh(const double x)										{ return ::sinh(x); }
inline long double sinh(const long double x)							{ return ::sinhl(x); }

///////////////////////////////////////////////////////////////////////////////
/// template for sqare.
template<typename T>
inline T sqr(const T x)													{ return x*x; }

///////////////////////////////////////////////////////////////////////////////
/// overloads for sqare root.
inline float sqrt(const float x)										{ return ::sqrtf(x); }
inline double sqrt(const double x)										{ return ::sqrt(x); }
inline long double sqrt(const long double x)							{ return ::sqrtl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for tangent
inline float tan(const float x)											{ return ::tanf(x); }
inline double tan(const double x)										{ return ::tan(x); }
inline long double tan(const long double x)								{ return ::tanl(x); }

///////////////////////////////////////////////////////////////////////////////
/// overloads for hyperbolic tangent
inline float tanh(const float x)										{ return ::tanhf(x); }
inline double tanh(const double x)										{ return ::tanh(x); }
inline long double tanh(const long double x)							{ return ::tanhl(x); }



///////////////////////////////////////////////////////////////////////////////
// templates for not-a-number checks
//##TODO: add portability layer from altex version of caldon
template<typename T> inline bool is_nan(const T& x)						{ return false; }
template<> inline bool is_nan<double>(const double& x)					{ return 0!=isnan(x); }

///////////////////////////////////////////////////////////////////////////////
// templates for finite checks
//##TODO: add portability layer from altex version of caldon
template<typename T> inline bool is_finite(const T& x)					{ return true; }
template<> inline bool is_finite<double>(const double& x)				{ return 0!=isfinite(x); }


/////////////////////////////
namespace detail {
/////////////////////////////

template<typename T>
bool equivalent(const T& a, const T& b, const T& EPSILON, const bool_true&)
{
	return (a < b + EPSILON) && (a + EPSILON > b);
}
template<typename T>
bool equivalent(const T& a, const T& b, const T&, const bool_false&)
{
	return a == b;
}

/////////////////////////////
}// end namespace detail
/////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// equivalence.
/// determine if a and b are within +/- epsilon range
template<typename T>
inline bool equivalent(const T& a, const T& b, const T& e=REAL_EPSILON)
{
	typedef typename is_rational<T>::Type ratial;
	return detail::equivalent(a, b, e, ratial());
}

///////////////////////////////////////////////////////////////////////////////
/// Greatest Common Divider of two variables.
/// usable types have to implement equal, lessthan, math::abs and modulo operation
template<typename T>
T gcd(T a, T b)
{
	const T zero = T();
	T r;
	if( a==zero || b==zero )
		return T(1);
	
	a=math::abs(a);
	b=math::abs(b);
	if(a<b)
	{	// swap
		r = a;
		a = b;
		b = r;
	}
	do
	{	// gcd
		r = a % b;
		a = b;
		b = r;
	}
	while( !( r==zero ) );
	return a;
}

///////////////////////////////////////////////////////////////////////////////
/// Least Common Multiple (SCM).
/// SCM of P(x) and Q(x) is defined by the relation
/// P(x)*Q(x) = GCD(P,Q)*SCM(P,Q).
template<typename T>
T scm(T a, T b)
{
	return a*b/gcd(a,b);
}


///////////////////////////////////////
}// end namespace math
///////////////////////////////////////


NAMESPACE_END(basics)


#endif//__BASEMATH_H__
