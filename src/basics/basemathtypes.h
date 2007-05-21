#ifndef __BASEMATHTYPES_H__
#define __BASEMATHTYPES_H__

#include "basetypes.h"

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// functional conversion overloads.
/// to change signed types to the appropriate unsigned
template <typename T>
inline const T& to_unsigned(const T& t)
{
	return t;
}
inline unsigned char to_unsigned(char t)
{
	return (unsigned char)(t);
}
inline unsigned short to_unsigned(short t)
{
	return (unsigned short)(t);
}
inline unsigned int to_unsigned(int t)
{
	return (unsigned int)(t);
}
inline unsigned long to_unsigned(long t)
{
	return (unsigned long)(t);
}
inline uint64 to_unsigned(int64 t)
{
	return (uint64)(t);
}

///////////////////////////////////////////////////////////////////////////////
/// functional conversion overloads.
/// to change unsigned types to the appropriate signed
template <typename T>
inline const T& to_signed(const T& t)
{
	return t;
}
inline signed char to_signed(unsigned char t)
{
	return (signed char)(t);
}
inline signed short to_signed(unsigned short t)
{
	return (signed short)(t);
}
inline signed int to_signed(unsigned int t)
{
	return (signed int)(t);
}
inline signed long to_signed(unsigned long t)
{
	return (signed long)(t);
}
inline sint64 to_signed(uint64 t)
{
	return (sint64)(t);
}

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////


#endif//__BASEMATHTYPES_H__
