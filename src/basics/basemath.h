#ifndef __BASEMATH_H__
#define __BASEMATH_H__

#include "basetypes.h"
#include "basebits.h"
#include "basepair.h"
#include "basemathtypes.h"

///////////////////////////////////////////////////////////////////////////////
// irregular part and missing functions
#if defined(_MSC_VER) || defined(__BORLANDC__)
///////////////////////////////////////////////////////////////////////////////
inline int fpclassify(double x)							{ return _fpclass(x); }
inline int isfinite(double x)							{ return _finite(x); }
inline int isnan(double x)								{ return _isnan(x); }

inline float hypotf(float x, float y)					{ return (float)_hypot((double)x,(double)y); }
inline double hypot(double x, double y)					{ return _hypot(x,y); }
inline long double hypotl(long double x, long double y)	{ return (long double)_hypot((double)x,(double)y); }

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
#else// others
///////////////////////////////////////////////////////////////////////////////

// When isfinite is not available, try to use one of the alternatives, or bail out.
#if !defined(isfinite) || defined(__CYGWIN__)
#undef isfinite
#if defined(fpclassify)
#define isfinite(x) (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#else
#define isfinite(x) ((x) - (x) == 0)
#endif// !defined(fpclassify)
#endif// !defined(isfinite)

#if !defined(isnan) || defined(__CYGWIN__)
#undef isnan
#if !defined(fpclassify)
#define isnan(x) ((x) != (x))
#else
#define isnan(x) (fpclassify(x) == FP_NAN)
#endif// !defined(fpclassify)
#endif// !defined(isnan)

///////////////////////////////////////////////////////////////////////////////
// assume C99 compliant environment when using without configure 
// placing the configure option in the "other" case does not disturb 
// the basics code generations as this is not used for the usual exports [marc]
#if defined(HAS_CONFIG)
// fall back to C89 double versions when no native extension available
// configure will enable decision between four cases,
// 1. declaration and library function exists (HAVE_DECL_ and HAVE_XXX)
// -> do nothing
// 2. no declaration but library function exists (!HAVE_DECL_ and HAVE_XXX)
// -> add a declaration
// 3. declaration but no library function exists (HAVE_DECL_ and !HAVE_XXX)
// -> do nothing, might be inlined or macroed, should not happen actually
// 4. neither declaration nor library function exists (!HAVE_DECL_ and !HAVE_XXX)
// -> add a replacement
extern "C" {

#if defined(HAVE_DECL_ACOSF) && !HAVE_DECL_ACOSF
#if !defined(HAVE_ACOSF)
inline float acosf(float x)								{ return (float)acos((double)x); }
#else
float acosf(float x);
#endif//!defined(HAVE_ACOSF)
#endif//defined(HAVE_DECL_ACOSF) && !HAVE_DECL_ACOSF

#if defined(HAVE_DECL_ACOSL) && !HAVE_DECL_ACOSL
#if !defined(HAVE_ACOSL)
inline long double acosl(long double x)					{ return (long double)acos((double)x); }
#else
long double acosl(long double x);
#endif//!defined(HAVE_ACOSL)
#endif//defined(HAVE_DECL_ACOSL) && !HAVE_DECL_ACOSL

#if defined(HAVE_DECL_ASINF) && !HAVE_DECL_ASINF
#if !defined(HAVE_ASINF)
inline float asinf(float x)								{ return (float)asin((double)x); }
#else
float asinf(float x);
#endif//!defined(HAVE_ASINF)
#endif//defined(HAVE_DECL_ASINF) && !HAVE_DECL_ASINF

#if defined(HAVE_DECL_ASINL) && !HAVE_DECL_ASINL
#if !defined(HAVE_ASINL)
inline long double asinl(long double x)					{ return (long double)asin((double)x); }
#else
long double asinl(long double x);
#endif//!defined(HAVE_ASINL)
#endif//defined(HAVE_DECL_ASINL) && !HAVE_DECL_ASINL

#if defined(HAVE_DECL_ATAN2F) && !HAVE_DECL_ATAN2F
#if !defined(HAVE_ATAN2F)
inline float atan2f(float x, float y)					{ return (float)atan2((double)x,(double)y); }
#else
float atan2f(float x, float y);
#endif//!defined(HAVE_ATAN2F)
#endif//defined(HAVE_DECL_ATAN2F) && !HAVE_DECL_ATAN2F

#if defined(HAVE_DECL_ATAN2L) && !HAVE_DECL_ATAN2L
#if !defined(HAVE_ATAN2L)
inline long double atan2l(long double x, long double y)	{ return (long double)atan2((double)x,(double)y); }
#else
long double atan2l(long double x, long double y);
#endif//!defined(HAVE_ATAN2L)
#endif//defined(HAVE_DECL_ATAN2L) && !HAVE_DECL_ATAN2L

#if defined(HAVE_DECL_ATANF) && !HAVE_DECL_ATANF
#if !defined(HAVE_ATANF)
inline float atanf(float x)								{ return (float)atan((double)x); }
#else
float atanf(float x);
#endif//!defined(HAVE_ATANF)
#endif//defined(HAVE_DECL_ATANF) && !HAVE_DECL_ATANF

#if defined(HAVE_DECL_ATANL) && !HAVE_DECL_ATANL
#if !defined(HAVE_ATANL)
inline long double atanl(long double x)					{ return (long double)atan((double)x); }
#else
long double atanl(long double x);
#endif//!defined(HAVE_ATANL)
#endif//defined(HAVE_DECL_ATANL) && !HAVE_DECL_ATANL

#if defined(HAVE_DECL_CEILF) && !HAVE_DECL_CEILF
#if !defined(HAVE_CEILF)
inline float ceilf(float x)								{ return (float)ceil((double)x); }
#else
float ceilf(float x);
#endif//!defined(HAVE_CEILF)
#endif//defined(HAVE_DECL_CEILF) && !HAVE_DECL_CEILF

#if defined(HAVE_DECL_CEILL) && !HAVE_DECL_CEILL
#if !defined(HAVE_CEILL)
inline long double ceill(long double x)					{ return (long double)ceil((double)x); }
#else
long double ceill(long double x);
#endif//!defined(HAVE_CEILL)
#endif//defined(HAVE_DECL_CEILL) && !HAVE_DECL_CEILL

#if defined(HAVE_DECL_COSF) && !HAVE_DECL_COSF
#if !defined(HAVE_COSF)
inline float cosf(float x)								{ return (float)cos((double)x); }
#else
float cosf(float x);
#endif//!defined(HAVE_COSF)
#endif//defined(HAVE_DECL_COSF) && !HAVE_DECL_COSF

#if defined(HAVE_DECL_COSHF) && !HAVE_DECL_COSHF
#if !defined(HAVE_COSHF)
inline float coshf(float x)								{ return (float)cosh((double)x); }
#else
float coshf(float x);
#endif//!defined(HAVE_COSHF)
#endif//defined(HAVE_DECL_COSHF) && !HAVE_DECL_COSHF

#if defined(HAVE_DECL_COSHL) && !HAVE_DECL_COSHL
#if !defined(HAVE_COSHL)
inline long double coshl(long double x)					{ return (long double)cosh((double)x); }
#else
long double coshl(long double x);
#endif//!defined(HAVE_COSHL)
#endif//defined(HAVE_DECL_COSHL) && !HAVE_DECL_COSHL

#if defined(HAVE_DECL_COSL) && !HAVE_DECL_COSL
#if !defined(HAVE_COSL)
inline long double cosl(long double x)					{ return (long double)cos((double)x); }
#else
long double cosl(long double x);
#endif//!defined(HAVE_COSL)
#endif//defined(HAVE_DECL_COSL) && !HAVE_DECL_COSL

#if defined(HAVE_DECL_EXPF) && !HAVE_DECL_EXPF
#if !defined(HAVE_EXPF)
inline float expf(float x)								{ return (float)exp((double)x); }
#else
float expf(float x);
#endif//!defined(HAVE_EXPF)
#endif//defined(HAVE_DECL_EXPF) && !HAVE_DECL_EXPF

#if defined(HAVE_DECL_EXPL) && !HAVE_DECL_EXPL
#if !defined(HAVE_EXPL)
inline long double expl(long double x)					{ return (long double)exp((double)x); }
#else
long double expl(long double x);
#endif//!defined(HAVE_EXPL)
#endif//defined(HAVE_DECL_EXPL) && !HAVE_DECL_EXPL

#if defined(HAVE_DECL_FABSF) && !HAVE_DECL_FABSF
#if !defined(HAVE_FABSF)
inline float fabsf(float x)								{ return (float)fabs((double)x); }
#else
float fabsf(float x);
#endif//!defined(HAVE_FABSF)
#endif//defined(HAVE_DECL_FABSF) && !HAVE_DECL_FABSF

#if defined(HAVE_DECL_FABSL) && !HAVE_DECL_FABSL
#if !defined(HAVE_FABSL)
inline long double fabsl(long double x)					{ return (long double)fabs((double)x); }
#else
long double fabsl(long double x);
#endif//!defined(HAVE_FABSL)
#endif//defined(HAVE_DECL_FABSL) && !HAVE_DECL_FABSL

#if defined(HAVE_DECL_FLOORF) && !HAVE_DECL_FLOORF
#if !defined(HAVE_FLOORF)
inline float floorf(float x)							{ return (float)floor((double)x); }
#else
float floorf(float x);
#endif//!defined(HAVE_FLOORF)
#endif//defined(HAVE_DECL_FLOORF) && !HAVE_DECL_FLOORF

#if defined(HAVE_DECL_FLOORL) && !HAVE_DECL_FLOORL
#if !defined(HAVE_FLOORL)
inline long double floorl(long double x)				{ return (long double)floor((double)x); }
#else
long double floorl(long double x);
#endif//!defined(HAVE_FLOORL)
#endif//defined(HAVE_DECL_FLOORL) && !HAVE_DECL_FLOORL

#if defined(HAVE_DECL_FMODF) && !HAVE_DECL_FMODF
#if !defined(HAVE_FMODF)
inline float fmodf(float x, float y)					{ return (float)fmod((double)x,(double)y); }
#else
float fmodf(float x, float y);
#endif//!defined(HAVE_FMODF)
#endif//defined(HAVE_DECL_FMODF) && !HAVE_DECL_FMODF

#if defined(HAVE_DECL_FMODL) && !HAVE_DECL_FMODL
#if !defined(HAVE_FMODL)
inline long double fmodl(long double x, long double y)	{ return (long double)fmod((double)x,(double)y); }
#else
long double fmodl(long double x, long double y);
#endif//!defined(HAVE_FMODL)
#endif//defined(HAVE_DECL_FMODL) && !HAVE_DECL_FMODL

#if defined(HAVE_DECL_FREXPF) && !HAVE_DECL_FREXPF
#if !defined(HAVE_FREXPF)
inline float frexpf(float x, int *e)					{ return (float)frexp((double)x,e); }
#else
float frexpf(float x, int *e);
#endif//!defined(HAVE_FREXPF)
#endif//defined(HAVE_DECL_FREXPF) && !HAVE_DECL_FREXPF

#if defined(HAVE_DECL_FREXPL) && !HAVE_DECL_FREXPL
#if !defined(HAVE_FREXPL)
inline long double frexpl(long double x, int *e)		{ return (long double)frexp((double)x,e); }
#else
long double frexpl(long double x, int *e);
#endif//!defined(HAVE_FREXPL)
#endif//defined(HAVE_DECL_FREXPL) && !HAVE_DECL_FREXPL

#if defined(HAVE_DECL_HYPOTF) && !HAVE_DECL_HYPOTF
#if !defined(HAVE_HYPOTF)
inline float hypotf(float x, float y)					{ return (float)hypot((double)x, (double)y); }
#else
float hypotf(float x, float y);
#endif//!defined(HAVE_HYPOTF)
#endif//defined(HAVE_DECL_HYPOTF) && !HAVE_DECL_HYPOTF

#if defined(HAVE_DECL_HYPOTL) && !HAVE_DECL_HYPOTL
#if !defined(HAVE_HYPOTL)
inline long double hypotl(long double x, long double y)	{ return (long double)hypot((double)x, (double)y); }
#else
long double hypotl(long double x, long double y);
#endif//!defined(HAVE_HYPOTL)
#endif//defined(HAVE_DECL_HYPOTL) && !HAVE_DECL_HYPOTL

#if defined(HAVE_DECL_LDEXPF) && !HAVE_DECL_LDEXPF
#if !defined(HAVE_LDEXPF)
inline float ldexpf(float v, int e)						{ return (float)ldexp((double)v,(double)e); }
#else
float ldexpf(float v, int e);
#endif//!defined(HAVE_LDEXPF)
#endif//defined(HAVE_DECL_LDEXPF) && !HAVE_DECL_LDEXPF

#if defined(HAVE_DECL_LDEXPL) && !HAVE_DECL_LDEXPL
#if !defined(HAVE_LDEXPL)
inline long double ldexpl(long double v, int e)			{ return (long double)ldexp((double)v,(double)e); }
#else
long double ldexpl(long double v, int e);
#endif//!defined(HAVE_LDEXPL)
#endif//defined(HAVE_DECL_LDEXPL) && !HAVE_DECL_LDEXPL

#if defined(HAVE_DECL_LOG10F) && !HAVE_DECL_LOG10F
#if !defined(HAVE_LOG10F)
inline float log10f(float x)							{ return (float)log10((double)x); }
#else
float log10f(float x);
#endif//!defined(HAVE_LOG10F)
#endif//defined(HAVE_DECL_LOG10F) && !HAVE_DECL_LOG10F

#if defined(HAVE_DECL_LOG10L) && !HAVE_DECL_LOG10L
#if !defined(HAVE_LOG10L)
inline long double log10l(long double x)				{ return (long double)log10((double)x); }
#else
long double log10l(long double x);
#endif//!defined(HAVE_LOG10L)
#endif//defined(HAVE_DECL_LOG10L) && !HAVE_DECL_LOG10L

#if defined(HAVE_DECL_LOGF) && !HAVE_DECL_LOGF
#if !defined(HAVE_LOGF)
inline float logf(float x)								{ return (float)log((double)x); }
#else
float logf(float x);
#endif//!defined(HAVE_LOGF)
#endif//defined(HAVE_DECL_LOGF) && !HAVE_DECL_LOGF

#if defined(HAVE_DECL_LOGL) && !HAVE_DECL_LOGL
#if !defined(HAVE_LOGL)
inline long double logl(long double x)					{ return (long double)log((double)x); }
#else
long double logl(long double x);
#endif//!defined(HAVE_LOGL)
#endif//defined(HAVE_DECL_LOGL) && !HAVE_DECL_LOGL

#if defined(HAVE_DECL_MODFF) && !HAVE_DECL_MODFF
#if !defined(HAVE_MODFF)
inline float modff(float x, float *i)					{ double tmp; double ret= modf((double)x,&tmp); *i=(float)tmp; return (float)ret; }
#else
float modff(float x, float *i);
#endif//!defined(HAVE_MODFF)
#endif//defined(HAVE_DECL_MODFF) && !HAVE_DECL_MODFF

#if defined(HAVE_DECL_MODFL) && !HAVE_DECL_MODFL
#if !defined(HAVE_MODFL)
inline long double modfl(long double x, long double *i)	{ double tmp; double ret= modf((double)x,&tmp); *i=(long double)tmp; return (long double)ret; }
#else
long double modfl(long double x, long double *i);
#endif//!defined(HAVE_MODFL)
#endif//defined(HAVE_DECL_MODFL) && !HAVE_DECL_MODFL

#if defined(HAVE_DECL_POWF) && !HAVE_DECL_POWF
#if !defined(HAVE_POWF)
inline float powf(float x, float y)						{ return (float)pow((double)x,(double)y); }
#else
float powf(float x, float y);
#endif//!defined(HAVE_POWF)
#endif//defined(HAVE_DECL_POWF) && !HAVE_DECL_POWF

#if defined(HAVE_DECL_POWL) && !HAVE_DECL_POWL
#if !defined(HAVE_POWL)
inline long double powl(long double x, long double y)	{ return (long double)pow((double)x,(double)y); }
#else
long double powl(long double x, long double y);
#endif//!defined(HAVE_POWL)
#endif//defined(HAVE_DECL_POWL) && !HAVE_DECL_POWL

#if defined(HAVE_DECL_SINF) && !HAVE_DECL_SINF
#if !defined(HAVE_SINF)
inline float sinf(float x)								{ return (float)sin((double)x); }
#else
float sinf(float x);
#endif//!defined(HAVE_SINF)
#endif//defined(HAVE_DECL_SINF) && !HAVE_DECL_SINF

#if defined(HAVE_DECL_SINHF) && !HAVE_DECL_SINHF
#if !defined(HAVE_SINHF)
inline float sinhf(float x)								{ return (float)sinh((double)x); }
#else
float sinhf(float x);
#endif//!defined(HAVE_SINHF)
#endif//defined(HAVE_DECL_SINHF) && !HAVE_DECL_SINHF

#if defined(HAVE_DECL_SINHL) && !HAVE_DECL_SINHL
#if !defined(HAVE_SINHL)
inline long double sinhl(long double x)					{ return (long double)sinh((double)x); }
#else
long double sinhl(long double x);
#endif//!defined(HAVE_SINHL)
#endif//defined(HAVE_DECL_SINHL) && !HAVE_DECL_SINHL

#if defined(HAVE_DECL_SINL) && !HAVE_DECL_SINL
#if !defined(HAVE_SINL)
inline long double sinl(long double x)					{ return (long double)sin((double)x); }
#else
long double sinl(long double x);
#endif//!defined(HAVE_SINL)
#endif//defined(HAVE_DECL_SINL) && !HAVE_DECL_SINL

#if defined(HAVE_DECL_SQRTF) && !HAVE_DECL_SQRTF
#if !defined(HAVE_SQRTF)
inline float sqrtf(float x)								{ return (float)sqrt((double)x); }
#else
float sqrtf(float x);
#endif//!defined(HAVE_SQRTF)
#endif//defined(HAVE_DECL_SQRTF) && !HAVE_DECL_SQRTF

#if defined(HAVE_DECL_SQRTL) && !HAVE_DECL_SQRTL
#if !defined(HAVE_SQRTL)
inline long double sqrtl(long double x)					{ return (long double)sqrt((double)x); }
#else
long double sqrtl(long double x);
#endif//!defined(HAVE_SQRTL)
#endif//defined(HAVE_DECL_SQRTL) && !HAVE_DECL_SQRTL

#if defined(HAVE_DECL_TANF) && !HAVE_DECL_TANF
#if !defined(HAVE_TANF)
inline float tanf(float x)								{ return (float)tan((double)x); }
#else
float tanf(float x);
#endif//!defined(HAVE_TANF)
#endif//defined(HAVE_DECL_TANF) && !HAVE_DECL_TANF

#if defined(HAVE_DECL_TANHF) && !HAVE_DECL_TANHF
#if !defined(HAVE_TANHF)
inline float tanhf(float x)								{ return (float)tanh((double)x); }
#else
float tanhf(float x);
#endif//!defined(HAVE_TANHF)
#endif//defined(HAVE_DECL_TANHF) && !HAVE_DECL_TANHF

#if defined(HAVE_DECL_TANHL) && !HAVE_DECL_TANHL
#if !defined(HAVE_TANHL)
inline long double tanhl(long double x)					{ return (long double)tanh((double)x); }
#else
long double tanhl(long double x);
#endif//!defined(HAVE_TANHL)
#endif//defined(HAVE_DECL_TANHL) && !HAVE_DECL_TANHL

#if defined(HAVE_DECL_TANL) && !HAVE_DECL_TANL
#if !defined(HAVE_TANL)
inline long double tanl(long double x)					{ return (long double)tan((double)x); }
#else
long double tanl(long double x);
#endif//!defined(HAVE_TANL)
#endif//defined(HAVE_DECL_TANL) && !HAVE_DECL_TANL

}// extern "C"
#endif//defined(HAS_CONFIG)
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#endif//
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
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
inline float exp10(const float x)										{ return ::expf((float)(x*LN10)); }
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
/// overloads for frexp. returning a pair
template<typename T> inline pair<T,int> frexp(const T& x)				{ int itmp; pair<T,int> ptmp( frexp<T>(x,&itmp),0); ptmp.second=itmp; return ptmp; }

///////////////////////////////////////////////////////////////////////////////
/// overloads for frexp. note that exponent is given as reference, not as pointer
inline float frexp(const float x, int& y)								{ return ::frexpf(x,&y); }
inline double frexp(const double x, int& y)								{ return ::frexp(x,&y); }
inline long double frexp(const long double x, int& y)					{ return ::frexpl(x,&y); }

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

///////////////////////////////////////////////////////////////////////////////
/// log base 2, the obvious way.
/// look at basebits for faster code
template <typename T>
inline T lg(T sz)
{
	T k;
	for (k = 0; sz != 1; sz >>= 1)
		++k;
	return k;
}

///////////////////////////////////////
}// end namespace math
///////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////


#endif//__BASEMATH_H__
