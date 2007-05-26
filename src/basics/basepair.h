#ifndef __BASE_PAIR_H__
#define __BASE_PAIR_H__

#include "basetypes.h"


NAMESPACE_BEGIN(basics)



///////////////////////////////////////////////////////////////////////////////
/// pair.
template<typename T1, typename T2=T1>
struct pair
{
	typedef pair type;
    typedef T1 first_type;
    typedef T2 second_type;

	T1 first;
	T2 second;

	pair()
		: first(T1()), second(T2())
	{}
	pair(const first_type& t1, const second_type& t2)
		: first(t1), second(t2)
	{}
	template <typename U1, typename U2>
	pair(const pair<U1, U2>& p)
		: first(p.first), second(p.second)
	{}
	pair(const pair& p)
		: first(p.first), second(p.second)
	{}
	~pair()
	{}
};



////////////////////////////////////////////////////////////////////////////////
// creation
template <typename T1, typename T2>
inline pair<T1, T2> make_pair(T1 a, T2 b)
{
	return pair<T1, T2>(a, b);
}
/*
template <typename T1, typename T2, int SZ>
inline pair<T1, T2 const*> make_pair(T1 const& a, T2 const (&b)[SZ])
{
	return pair<T1, T2 const*>(a, static_cast<T2 const*>(b));
}

template <typename T1, typename T2, int SZ>
inline pair<T1 const*, T2> make_pair(T1 const (&a)[SZ], T2 const& b)
{
	return pair<T1 const*, T2>(static_cast<T1 const*>(a), b);
}

template <typename T1, typename T2, int SZ1, int SZ2>
inline pair<T1 const*, T2 const*> make_pair(T1 const (&a)[SZ1], T2 const (&b)[SZ2])
{
	return pair<T1 const*, T2 const*>(static_cast<T1 const*>(a), static_cast<T2 const*>(b));
}
*/

////////////////////////////////////////////////////////////////////////////////
// comparison
template <typename T1, typename T2>
inline bool operator==(const pair<T1, T2>& a, const pair<T1, T2>& b)
{
	return a.first == b.first && a.second == b.second;
}

template <typename T1, typename T2>
inline bool operator<(const pair<T1, T2>& a, const pair<T1, T2>& b)
{
	return a.first < b.first || ( a.first == b.first && a.second < b.second);
}









///////////////////////////////////////////////////////////////////////////////
/// triple.
template<typename T1, typename T2=T1, typename T3=T2>
struct triple
{
	typedef triple type;
    typedef T1 first_type;
    typedef T2 second_type;
	typedef T3 third_type;

	T1 first;
	T2 second;
	T3 third;

	triple()
		: first(T1()), second(T2()), third(T3())
	{}
	triple(const first_type& t1, const second_type& t2, const third_type& t3)
		: first(t1), second(t2), third(t3)
	{}
	template <typename U1, typename U2, typename U3>
	triple(const triple<U1, U2, U3>& p)
		: first(p.first), second(p.second), third(p.third)
	{}
	triple(const triple& p)
		: first(p.first), second(p.second), third(p.third)
	{}
	~triple()
	{}
};

////////////////////////////////////////////////////////////////////////////////
// creation
template <typename T1, typename T2, typename T3>
inline triple<T1, T2, T3> make_triple(T1 a, T2 b, T3 c)
{
	return triple<T1, T2, T3>(a, b, c);
}

////////////////////////////////////////////////////////////////////////////////
// comparison
template<typename T1, typename T2, typename T3>
inline bool operator == (const triple<T1,T2,T3>& a, const triple<T1,T2,T3>& b)
{	
	return a.first == b.first && a.second == b.second && a.third == b.third;
}

template<typename T1, typename T2, typename T3>
inline bool operator < (const triple<T1,T2,T3>& a, const triple<T1,T2,T3>& b)
{
	if( a.first < b.first ) return true;
	if( b.first < a.first ) return false;
	if( a.second < b.second ) return true;
	if( b.second < a.second ) return false;
	return ( a.third < b.third );
}



NAMESPACE_END(basics)

#endif//__BASE_PAIR_H__
