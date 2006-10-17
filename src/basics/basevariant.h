#ifndef __BASEVARIANT_H__
#define __BASEVARIANT_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basearray.h"
#include "basestring.h"



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// test function
void test_variant();




///////////////////////////////////////////////////////////////////////////////
/// variant class.
/// can be of integer, floating point, string, array or reference type 
/// conversions are done internally on appropriate access
class Variant : public global
{
	typedef enum _value_type
	{
		VAR_NONE = 0,
		VAR_INTEGER,
		VAR_STRING,
		VAR_FLOAT,
		VAR_ARRAY,
	} var_t;

	class _value : public noncopyable
	{
		friend class Variant;
		var_t					cType;
		union
		{	// integer
			int64				cInteger;
			// double
			double				cFloat;
			// string (actually a pointer to a string<> object since a class cannot be a union member)
			string<>*			cString;
			// array
			vector<Variant>*	cArray;
		};
	public:
		///////////////////////////////////////////////////////////////////////
		// default constructor/destructor
		_value() :cType(VAR_NONE), cInteger(0)
		{}
		~_value()
		{
			clear();
		}
		///////////////////////////////////////////////////////////////////////
		// type constructors
		_value(const _value &v) :cType(VAR_NONE), cInteger(0)
		{
			assign(v);
		}
		_value(const int v) :cType(VAR_INTEGER), cInteger(v)
		{}
		_value(const double v) :cType(VAR_FLOAT), cFloat(v)
		{}
		_value(const char* v) :cType(VAR_NONE), cInteger(0)
		{
			assign(v);
		}
		_value(const string<>& v) :cType(VAR_NONE), cInteger(0)
		{
			assign(v);
		}

		///////////////////////////////////////////////////////////////////////
		//
		const _value& operator=(const _value &v)
		{
			assign(v);
			return *this; 
		}
		const _value& operator=(const int val)
		{
			assign(val);
			return *this; 
		}
		const _value& operator=(const double val)
		{
			assign(val);
			return *this; 
		}
		const _value& operator=(const char* val)
		{
			assign(val);
			return *this; 
		}
		const _value& operator=(const string<>& val)
		{
			assign(val);
			return *this; 
		}
		///////////////////////////////////////////////////////////////////////
		//
		void assign(const _value &v);
		void assign(const int val);
		void assign(const double val);
		void assign(const char* val);
		void assign(const string<>& val);

		///////////////////////////////////////////////////////////////////////
		//
		void clear();
		///////////////////////////////////////////////////////////////////////
		bool isValid() const	{ return cType != VAR_NONE; }	
		bool isInt() const		{ return cType == VAR_INTEGER; }	
		bool isFloat() const	{ return cType == VAR_FLOAT; }	
		bool isString() const	{ return cType == VAR_STRING; }	
		bool isArray() const	{ return cType == VAR_ARRAY; }
		size_t size() const		{ return (cType== VAR_ARRAY) ? (cArray?cArray->size():0): 1; }
		///////////////////////////////////////////////////////////////////////
		void setarray(size_t cnt);
		///////////////////////////////////////////////////////////////////////
		void addarray(size_t cnt);

		friend int compare(const Variant& va, const Variant& vb);
	};

	friend class _value;

	TObjPtrCommon< _value > cValue;

public:
	///////////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	Variant()	{}
	~Variant()	{}
	///////////////////////////////////////////////////////////////////////////
	// Copy/Assignment
	Variant(const Variant &v, bool ref=false)	{ assign(v, ref); }
	Variant(int val)							{ assign(val); }
	Variant(double val)							{ assign(val); }
	Variant(const char* val)					{ assign(val); }
	const Variant& operator=(const Variant &v)	{ assign(v);   return *this; }
	const Variant& operator=(const int val)		{ assign(val); return *this; }
	const Variant& operator=(const double val)	{ assign(val); return *this; }
	const Variant& operator=(const char* val)	{ assign(val); return *this; }

	///////////////////////////////////////////////////////////////////////
	// type of the variant
	bool isReference() const	{ return cValue.isAutoCreate(); }
	void makeValue() const		{ cValue.setCopyonWrite(); }		// enable copy-on-write
	void makeVariable() const	{ cValue.setAutoCreate(); }			// disable copy-on-write

	///////////////////////////////////////////////////////////////////////////
	// Type Initialize Variant (aka copy)
	void assign(const Variant& v, bool ref=false);
	///////////////////////////////////////////////////////////////////////////
	// Type Initialize Integer
	void assign(int val);
	///////////////////////////////////////////////////////////////////////////
	// Type Initialize double
	void assign(double val);
	///////////////////////////////////////////////////////////////////////////
	// Type Initialize String
	void assign(const char* val);
	///////////////////////////////////////////////////////////////////////////
	// Create Array from actual element
	void setarray(size_t cnt);
	///////////////////////////////////////////////////////////////////////////
	// Create Array by vectorizing deepest elements
	void addarray(size_t cnt);

	///////////////////////////////////////////////////////////////////////////
	// clear this
	void clear()
	{
		cValue->clear();
	}


	///////////////////////////////////////////////////////////////////////////
	// Access to elements
	bool isValid() const	{ return cValue->isValid(); }
	bool isInt() const		{ return cValue->isInt(); }
	bool isFloat() const	{ return cValue->isFloat(); }
	bool isString() const	{ return cValue->isString(); }
	bool isArray() const	{ return cValue->isArray(); }
	size_t size() const		{ return cValue->size(); }

	///////////////////////////////////////////////////////////////////////////
	// Access to array elements
	const Variant& operator[](size_t inx) const;
	Variant& operator[](size_t inx);

	///////////////////////////////////////////////////////////////////////////
	// local conversions
	void convert2string();
	void convert2float();
	void convert2int();
	void convert2number();
	void cast(int type);

	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	string<> getString() const;
	double getFloat() const;
	int getInt() const;

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	const Variant operator-() const;
	const Variant operator~() const;
	const Variant operator!() const;


	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	const Variant& operator+=(const Variant& v);
	const Variant& operator-=(const Variant& v);
	const Variant& operator*=(const Variant& v);
	const Variant& operator/=(const Variant& v);

	///////////////////////////////////////////////////////////////////////////
	// Define postfix increment operator.
	Variant operator++(int);
	Variant operator--(int);
	///////////////////////////////////////////////////////////////////////////
	// Define prefix increment operator.
	Variant& operator++();
	Variant& operator--();

	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	const Variant& operator&=(const Variant& v);
	const Variant& operator|=(const Variant& v);
	const Variant& operator^=(const Variant& v);
	const Variant& operator>>=(const Variant& v);
	const Variant& operator<<=(const Variant& v);

	///////////////////////////////////////////////////////////////////////////
	/// compare operations
	friend int compare(const Variant& va, const Variant& vb);

};


inline Variant operator+(const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp += vb;
	return temp;
}
inline Variant operator-(const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp -= vb;
	return temp;

}
inline Variant operator*(const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp *= vb;
	return temp;

}
inline Variant operator/(const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp /= vb;
	return temp;
}
inline Variant operator&(const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp &= vb;
	return temp;
}
inline Variant operator| (const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp |= vb;
	return temp;
}
inline Variant operator^ (const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp ^= vb;
	return temp;
}
inline Variant operator>>(const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp >>= vb;
	return temp;
}
inline Variant operator<<(const Variant& va, const Variant& vb)
{
	Variant temp(va);
	temp <<= vb;
	return temp;
}
inline bool operator&&(const Variant& va, const Variant& vb)	{ return va.getInt() && vb.getInt(); }
inline bool operator||(const Variant& va, const Variant& vb)	{ return va.getInt() || vb.getInt(); }

///////////////////////////////////////////////////////////////////////////
// boolean compare
inline bool operator==(const Variant& a, const Variant& b)	{ return 0==compare(a,b); }
inline bool operator!=(const Variant& a, const Variant& b)	{ return 0!=compare(a,b); }
inline bool operator< (const Variant& a, const Variant& b)	{ return 0< compare(a,b); }
inline bool operator<=(const Variant& a, const Variant& b)	{ return 0<=compare(a,b); }
inline bool operator> (const Variant& a, const Variant& b)	{ return 0> compare(a,b); }
inline bool operator>=(const Variant& a, const Variant& b)	{ return 0>=compare(a,b); }



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASEVARIANT_H__
