#ifndef __BASESWAP_H__
#define __BASESWAP_H__

#include "basebooltype.h"

NAMESPACE_BEGIN(basics)
///////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#if !defined (__ICL) && ( (defined (_MSC_VER) && (_MSC_VER <= 1200)) || defined(__GNUC__) && ((__GNUC__ < 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ < 4))) )
// VC++ 6.0 and before has problems with function member pointers as template parameters
// "some" gcc cannot handle the used SFINAE expression (assumingly below 3.4)
///////////////////////////////////////////////////////////////////////////////

template< typename T >
struct has_swap
{
	enum{ Result = false };
	typedef bool_false Type;
};

///////////////////////////////////////////////////////////////////////////////
#else // other compilers
// still need to test
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////
namespace detail {
/////////////////////////////

// check for a template member "void T::swap(T&);"
template < typename T, void (T::*)(T&) > struct has_swap_struct {};

template < typename T > char* has_swap_helper(...);
template < typename T > char  has_swap_helper(has_swap_struct<T, &T::swap>*);

/////////////////////////////
}// end namespace detail
/////////////////////////////

template< typename T >
struct has_swap
{
	enum{ Result = sizeof(detail::has_swap_helper<T>(0)) == sizeof(char) };
	typedef typename bool2type<Result>::Type Type;
};	

///////////////////////////////////////////////////////////////////////////////
#endif // other compilers
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
namespace detail {
///////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void swap_helper(T& a, T& b, const bool_true&) //swap implemented
{
	a.swap(b);
}
template <typename T>
inline void swap_helper(T& a, T& b, const bool_false&) //swap not implemented
{
	T tmp = a;
	a = b;
	b = tmp;
}

///////////////////////////////////////////////////////////////////////////////
}// end namespace detail
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// swap function.
/// decides if using the 3-copy-swap or a buildin-swap
template <typename T>
inline void swap(T& a, T& b)
{
	typedef typename has_swap<T>::Type hasswap;
	detail::swap_helper(a, b, hasswap());
}



///////////////////////////////////////////////////////////////////////////////
/// min/max templates.
/// The Windoze headers define macros called max/min which conflict with
/// the templates std::max and std::min. So, to avoid conflicts, 
/// MS removed the std::max/min rather than fixing the problem!
/// From Visual Studio .NET (SV7, compiler version 13.00) the STL templates 
/// have been added correctly. This fix switches off the macros and reinstates
/// the STL templates for earlier versions (SV6).
/// Note that this could break MFC applications that rely on the macros
/// (try it and see).
/// For MFC compatibility, only undef min and max in non-MFC programs - 
/// some bits of MFC use macro min/max in headers. For VC7 both the macros 
/// and template functions exist so there is no real need for the undefs
/// but do it anyway for consistency. So, if using VC6 and MFC then template
/// functions will not exist

#ifdef min // windef has macros for that, kill'em
#undef min
#endif
///////////////////////////////////////////////////////////////////////////////
/// min.
template <typename T>
inline const T& min(const T& i1, const T& i2)
{
	if(i1 < i2) return i1; else return i2;
}
#ifdef max // windef has macros for that, kill'em
#undef max
#endif
///////////////////////////////////////////////////////////////////////////////
/// max.
template <typename T>
inline const T& max(const T& i1, const T& i2)	
{
	if(i1 < i2) return (T&)i2; else return (T&)i1;
}

///////////////////////////////////////////////////////////////////////////////
/// minmax.
/// sets output according to min and max
template <typename T>
inline void minmax(const T &i1, const T &i2, T &minval, T &maxval)
{
	if(i1<i2)
		minval=i1, maxval=i2;
	else
		minval=i2, maxval=i1;
}



///////////////////////////////////////////////////////////////////////////////
/// min and max with comparator typename.
template <typename T, typename Compare>
inline const T& min(const T& a, const T& b, Compare cmp)
{
	return cmp(a,b) ? a : b;
}

template <typename T, typename Compare>
inline const T& max(const T& a, const T& b, Compare cmp)
{
	return cmp(a,b) ? b : a;
}




///////////////////////////////////////
NAMESPACE_END(basics)


#endif//__BASESWAP_H__
