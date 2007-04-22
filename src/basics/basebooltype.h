#ifndef __BASEBOOLTYPE_H__
#define __BASEBOOLTYPE_H__


NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////////////////////////
// boolean type and boolean operations

///////////////////////////////////////////////////////////////////////////////
/// boolean true struct.
struct bool_true
{	
	enum _dummy { Result = true };
	/// right hand cast. allow implicit type2value conversion
	operator bool() const	{ return Result; }
};
///////////////////////////////////////////////////////////////////////////////
/// boolean false struct.
struct bool_false
{	
	enum _dummy { Result = false };
	/// right hand cast. allow implicit type2value conversion
	operator bool() const	{ return Result; }
};


///////////////////////////////////////////////////////////////////////////////
/// bool to type conversion.
/// true is set as default, have explicit instanciations for each value
/// (having both would actually not be necessary)
template <bool is> struct bool2type
{
	typedef bool_true Type;
};

template<>
struct bool2type<true>
{
	typedef bool_true Type;
};

template<>
struct bool2type<false>
{
	typedef bool_false Type;
};


///////////////////////////////////////////////////////////////////////////////
/// type to bool conversion.
/// true is set as default, have explicit instanciations for each value
/// (having both would actually not be necessary)
template <typename T>
struct type2bool
{
	enum _dummy { Result = true };
};

template<>
struct type2bool<bool_true>
{
	enum _dummy { Result = true };
};

template<>
struct type2bool<bool_false>
{
	enum _dummy { Result = false };
};


///////////////////////////////////////////////////////////////////////////////
/// Negation.
template <typename T>
struct logic_not
{
	typedef bool_false Type;
};

template<>
struct logic_not<bool_false>
{
	typedef bool_true Type;
};

///////////////////////////////////////////////////////////////////////////////
/// logical AND.
/// result is only true when all types are true
/// allow up to 5 parameters, 
template <typename P1, typename P2, typename P3=bool_true, typename P4=bool_true, typename P5=bool_true>
struct logic_and
{
	typedef bool_false Type;
};

template<>
struct logic_and<bool_true, bool_true, bool_true, bool_true, bool_true>
{
	typedef bool_true Type;
};

///////////////////////////////////////////////////////////////////////////////
/// logical OR.
/// result is only false when all types are false
/// allow up to 5 parameters, 
template <typename P1, typename P2, typename P3=bool_false, typename P4=bool_false, typename P5=bool_false>
struct logic_or
{
	typedef bool_true Type;
};

template<>
struct logic_or<bool_false, bool_false, bool_false, bool_false, bool_false>
{
	typedef bool_false Type;
};


NAMESPACE_END(basics)

#endif//__BASEBOOLTYPE_H__

