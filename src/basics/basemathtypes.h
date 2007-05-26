#ifndef __BASEMATHTYPES_H__
#define __BASEMATHTYPES_H__

#include "basetypes.h"
#include "basebooltype.h"

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////


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
