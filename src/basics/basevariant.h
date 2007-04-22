#ifndef __BASEVARIANT_H__
#define __BASEVARIANT_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basearray.h"
#include "basesafeptr.h"
#include "basestring.h"
#include "basemath.h"

//#include "basenew.h"
#include <new>	//##TODO: prepare a workaround when conflicts on windows
				// remember that global namespace is poisoned from here

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// test function
void test_variant();


///////////////////////////////////////////////////////////////////////////////
// predeclartions
struct value_empty;
struct value_invalid;
struct value_integer;
struct value_float;
struct value_string;
struct value_conststring;
struct value_array;
struct value_extern;
template<typename T>
struct value_unnamed;
struct value_named;

struct value_union;
struct variant;


///////////////////////////////////////////////////////////////////////////////
// variant types
enum var_t
{
	VAR_NONE = 0,
	VAR_AUTO,
	VAR_DEFAULT = VAR_AUTO,
	VAR_INTEGER,
	VAR_STRING,
	VAR_FLOAT,
	VAR_ARRAY
};


///////////////////////////////////////////////////////////////////////////
/// string type detector
/// false is set as default, have explicit instanciation for string type
template<typename T>
struct is_string_type
{
	enum _dummy { Result = false };
	typedef bool_false Type;
};
template<>
struct is_string_type< string<> >
{
	enum _dummy { Result = true };
	typedef bool_true Type;
};	


///////////////////////////////////////////////////////////////////////////////
/// interface to a variant host class.
/// a variant host can contain several variables
/// which are selected by a name inside the variant host
class variant_host
{
public:
	typedef TObjPtrCount<variant_host*> reference;
private:
	reference baseptr;
public:
	variant_host() : baseptr(this)
	{}
	virtual ~variant_host()
	{
		if( this->baseptr.exists() )
			*this->baseptr = NULL;
	}
	const reference& access() const
	{
		return baseptr;
	}
	virtual bool get_member(const string<>& name, value_empty& target)=0;
	virtual bool set_member(const string<>& name, const value_empty& target)=0;
};

///////////////////////////////////////////////////////////////////////////////
/// selection of default variant_hosts.
/// when asking for a global variant name, 
/// the variant_defaults<bool>::get_variable function is called.
/// user definition can be done either by a boolean specialisation
/// of the get_variable function or by deriving and instanciating
/// a user defined structure. 
/// only one concurrent user defined structure is supported.
template <typename T>
struct variant_defaults
{
private:
	static variant_defaults* global_variant_defaults;
protected:
	friend struct value_empty;
	variant_defaults()
	{
		if(variant_defaults::global_variant_defaults==NULL)
			variant_defaults::global_variant_defaults = this;
	}
public:
	virtual ~variant_defaults()
	{
		if( this==variant_defaults::global_variant_defaults )
			variant_defaults::global_variant_defaults=NULL;
	}

	variant_host::reference _get_variable(const string<>& name) const
	{
		if( variant_defaults::global_variant_defaults )
			return variant_defaults::global_variant_defaults->get_variable(name);
		else
			return this->get_variable(name);
	}

	///////////////////////////////////////////////////////////////////////////
	/// name to global parent.
	virtual variant_host::reference get_variable(const string<>& name) const;
};

///////////////////////////////////////////////////////////////////////////////
/// name to global parent.
/// default implementation, can be overwriten with boolean specialisation
template<typename T>
variant_host::reference variant_defaults<T>::get_variable(const string<>& name) const
{	// return an empty reference
	return variant_host::reference();
}



///////////////////////////////////////////////////////////////////////////////
/// value_empty type.
/// marks the "empty type", 
/// also defines the basic interface for the derived types
struct value_empty
{
	friend struct value_named;
	friend struct value_union;
protected:
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_empty()
	{}
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	template<typename T>
	explicit value_empty(const T& v)					{ this->assign(v); }
	explicit value_empty(const value_empty& v)			{ this->assign(v); }
public:
	virtual ~value_empty()
	{
		//printf("destroy %p value_empty\n", this);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	template<typename T>
	const value_empty& operator=(const T& v)			{ this->assign(v); return *this; }
	const value_empty& operator=(const value_empty& v)	{ this->assign(v); return *this; }

	///////////////////////////////////////////////////////////////////////////
	//
	virtual inline void assign(const value_empty &v);
	virtual inline void assign(const int v)					{ this->assign((sint64)v); }
	virtual inline void assign(const uint v)				{ this->assign((sint64)v); }
	virtual inline void assign(const long v)				{ this->assign((sint64)v); }
	virtual inline void assign(const ulong v)				{ this->assign((sint64)v); }
	virtual inline void assign(const sint64 v);
	virtual inline void assign(const uint64 v)				{ this->assign((sint64)v); }
	virtual inline void assign(const double v);
	virtual inline void assign(const char* v);
	virtual inline void assign(const string<>& v);
	virtual inline void assign(const vector<variant>& v);
	virtual inline void assign(const variant_host& v);
	virtual inline void assign(const variant_host::reference& v);
	virtual inline void assign(const variant &v);
	
	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{
		convertee.~value_empty();
		return *new (&convertee) value_empty;
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_empty();
	}
	void destruct()
	{
		this->~value_empty();
	}
	static const value_empty& convert(value_empty& convertee)
	{
		convertee.~value_empty();
		return *new (&convertee) value_empty();
	}
	static const value_empty& construct(void* p)
	{
		return *new (p) value_empty();
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual inline void clear()				{}
	virtual inline void invalidate();
	virtual inline void empty();

	///////////////////////////////////////////////////////////////////////////
	virtual inline bool is_valid() const		{ return true; }
	virtual inline bool is_empty() const		{ return true; }
	virtual inline bool is_int() const			{ return false; }
	virtual inline bool is_float() const		{ return false; }
	virtual inline bool is_number() const		{ return false; }
	virtual inline bool is_string() const		{ return false; }	
	virtual inline bool is_array() const		{ return false; }
	virtual inline bool is_extern() const		{ return false; }
	virtual inline size_t size() const			{ return 1; }
	virtual inline var_t type() const			{ return VAR_NONE; }
	///////////////////////////////////////////////////////////////////////////
	virtual inline variant_host::reference get_parent() const
	{
		return variant_host::reference(NULL);
	}
	virtual inline bool access_member(const string<>& name)
	{
		variant_defaults<bool> dv;
		variant_host::reference parent = dv._get_variable(name);
		if( parent.exists() && *parent )
		{
			this->assign(**parent);
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	virtual inline bool resize_array(size_t cnt)	{ return false; }
	virtual inline bool append_array(size_t cnt)	{ return false; }
	///////////////////////////////////////////////////////////////////////////
	// Access to array elements
	virtual inline const variant* operator[](size_t inx) const	{ return NULL; }
	virtual inline       variant* operator[](size_t inx)		{ return NULL; }

	///////////////////////////////////////////////////////////////////////////
	// local conversions
	virtual void convert2string();
	virtual void convert2float();
	virtual void convert2int();
	virtual void convert2number();
	virtual void numeric_cast(const variant& v);

	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const		{ return false; }
	virtual int64 get_int() const		{ return 0; }
	virtual double get_float() const	{ return 0.0; }
	virtual string<> get_string() const	{ return string<>(); }
	virtual string<> get_arraystring() const { return this->get_string(); }
	virtual const char* get_cstring() const	{ return ""; }

	operator bool() const				{ return this->get_bool(); }
	operator int() const				{ return (int)this->get_int(); }
	operator int64() const				{ return this->get_int(); }
	operator double() const				{ return this->get_float(); }
	operator string<>() const			{ return this->get_string(); }


protected:
	///////////////////////////////////////////////////////////////////////////
	// internal type depending value selectors
	template<typename T>
	void get_string(T& v, const bool_true&) const
	{
		v = this->get_string();
	}
	template<typename T>
	void get_string(T& v, const bool_false&) const
	{	// type T is not recognized
		//##TODO: check if some conversion could be offered
		struct unrecognized_type
		{} x = v;
	}
	template<typename T>
	void get_float(T& v, const bool_true&) const
	{
		v = this->get_float();
	}
	template<typename T>
	void get_float(T& v, const bool_false&) const
	{
		typedef typename is_string_type<T>::Type isstring;
		get_string(v, isstring());
	}	
	template<typename T>
	void get_integer(T& v, const bool_true&) const
	{
		v = this->get_int();
	}	
	template<typename T>
	void get_integer(T& v, const bool_false&) const
	{
		typedef typename is_rational<T>::Type isfloat;
		get_float(v, isfloat());
	}	
public:
	template<typename T>
	void get_value(T& v) const
	{
		typedef typename is_integral<T>::Type isint;
		this->get_integer(v, isint());
	}
	template<typename T>
	void set_value(const T& v)
	{
		this->assign(v);
	}

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	virtual inline void negate();
	virtual inline void invert();
	virtual inline void lognot();

	///////////////////////////////////////////////////////////////////////////
	// prefix increment operator. (no postfix)
	virtual const value_empty& operator++();
	virtual const value_empty& operator--();

	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator_assign(const variant& v);
	virtual const value_empty& operator+=(const variant& v);
	virtual const value_empty& operator-=(const variant& v);
	virtual const value_empty& operator*=(const variant& v);
	virtual const value_empty& operator/=(const variant& v);
	virtual const value_empty& operator%=(const variant& v);

	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	virtual const value_empty& operator &=(const variant& v);
	virtual const value_empty& operator |=(const variant& v);
	virtual const value_empty& operator ^=(const variant& v);
	virtual const value_empty& operator>>=(const variant& v);
	virtual const value_empty& operator<<=(const variant& v);
};




///////////////////////////////////////////////////////////////////////////////
/// value_invalid type.
/// implements the invalid result of ie. division by zero
struct value_invalid : public value_empty
{
protected:
	///////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_invalid()
	{}
public:
	virtual ~value_invalid()
	{
		//printf("destroy %p value_invalid\n", this);
	}
	///////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{
		convertee.~value_empty();
		return *new (&convertee) value_invalid();
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_invalid();
	}
	static const value_invalid& convert(value_empty& convertee)
	{
		convertee.~value_empty();
		return *new (&convertee) value_invalid();
	}
	static const value_invalid& construct(void* p)
	{
		return *new (p) value_invalid();
	}

	///////////////////////////////////////////////////////////////////////
	//
	virtual void clear()				{ value_empty::convert(*this); }
	virtual void invalidate()			{}
	///////////////////////////////////////////////////////////////////////
	virtual bool is_valid() const		{ return false; }
	virtual bool is_empty() const		{ return true; }
	virtual var_t type() const			{ return VAR_NONE; }
	///////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const		{ return 0; }
	virtual int64 get_int() const		{ return 0; }
	virtual double get_float() const	{ return 0; }
	virtual string<> get_string() const	{ return ""; }

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	virtual void negate()	{ }
	virtual void invert()	{ }
	virtual void lognot()	{ }

	///////////////////////////////////////////////////////////////////////////
	// prefix increment operator. (no postfix)
	virtual const value_empty& operator++()	{ return *this; }
	virtual const value_empty& operator--()	{ return *this; }

	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator+=(const variant& v)		{ return *this; }
	virtual const value_empty& operator-=(const variant& v)		{ return *this; }
	virtual const value_empty& operator*=(const variant& v)		{ return *this; }
	virtual const value_empty& operator/=(const variant& v)		{ return *this; }
	virtual const value_empty& operator%=(const variant& v)		{ return *this; }
	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	virtual const value_empty& operator &=(const variant& v)	{ return *this; }
	virtual const value_empty& operator |=(const variant& v)	{ return *this; }
	virtual const value_empty& operator ^=(const variant& v)	{ return *this; }
	virtual const value_empty& operator>>=(const variant& v)	{ return *this; }
	virtual const value_empty& operator<<=(const variant& v)	{ return *this; }
};


///////////////////////////////////////////////////////////////////////////////
/// value_integer type.
/// implements the "integer" type
struct value_integer : public value_empty
{
	friend struct variant;
protected:
	sint64 value;
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_integer() : value(0)
	{}
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_integer(const int v) : value(v)		{}
	explicit value_integer(const uint v) : value(v)		{}
	explicit value_integer(const sint64 v) : value(v)	{}
	explicit value_integer(const uint64 v) : value(v)	{}
public:
	virtual ~value_integer()
	{
		//printf("destroy %p value_integer\n", this);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	const value_integer& operator=(const int v)			{ this->assign(v); return *this; }
	const value_integer& operator=(const uint v)		{ this->assign(v); return *this; }
	const value_integer& operator=(const sint64 v)		{ this->assign(v); return *this; }
	const value_integer& operator=(const uint64 v)		{ this->assign(v); return *this; }
	///////////////////////////////////////////////////////////////////////////
	//
	using value_empty::assign;
	virtual void assign(const int v)					{ this->value = v; }
	virtual void assign(const uint v)					{ this->value = v; }
	virtual void assign(const sint64 v)					{ this->value = v; }
	virtual void assign(const uint64 v)					{ this->value = v; }
	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{
		convertee.~value_empty();
		return *new (&convertee) value_integer(this->value);
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_integer(this->value);
	}
	static const value_integer& convert(value_empty& convertee, int64 v)
	{
		convertee.~value_empty();
		return *new (&convertee) value_integer(v);
	}
	static const value_integer& construct(void* p, int64 v)
	{
		return *new (p) value_integer(v);
	}

	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear()				{ value=0; }
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_empty() const		{ return false; }
	virtual bool is_int() const			{ return true; }
	virtual bool is_number() const		{ return true; }
	virtual var_t type() const			{ return VAR_INTEGER; }
	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const		{ return this->value!=0; }
	virtual int64 get_int() const		{ return this->value; }
	virtual double get_float() const	{ return (double)this->value; }
	virtual string<> get_string() const	{ return string<>((int)this->value); }

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	virtual void negate()	{ this->value = -this->value; }
	virtual void invert()	{ this->value = ~this->value; }
	virtual void lognot()	{ this->value = (this->value=!0); }

	///////////////////////////////////////////////////////////////////////////
	// prefix increment operator. (no postfix)
	virtual const value_empty& operator++()	{ ++this->value; return *this; }
	virtual const value_empty& operator--()	{ --this->value; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator+=(const variant& v);
	virtual const value_empty& operator-=(const variant& v);
	virtual const value_empty& operator*=(const variant& v);
	virtual const value_empty& operator/=(const variant& v);
	virtual const value_empty& operator%=(const variant& v);
	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	virtual const value_empty& operator &=(const variant& v);
	virtual const value_empty& operator |=(const variant& v);
	virtual const value_empty& operator ^=(const variant& v);
	virtual const value_empty& operator>>=(const variant& v);
	virtual const value_empty& operator<<=(const variant& v);
};

///////////////////////////////////////////////////////////////////////////////
/// value_float type.
/// implements the "floating point" type
struct value_float : public value_empty
{
	friend struct variant;
protected:
	double value;
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_float() : value(0)
	{}
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_float(const double v) : value(v)		{}
public:
	virtual ~value_float()
	{
		//printf("destroy %p value_float\n", this);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	const value_float&   operator=(const double v)		{ this->assign(v); return *this; }
	///////////////////////////////////////////////////////////////////////////
	//
	using value_empty::assign;
	virtual void assign(const double v)
	{
		this->value = v;
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{
		convertee.~value_empty();
		return *new (&convertee) value_float(this->value);
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_float(this->value);
	}
	static const value_float& convert(value_empty& convertee, double v)
	{
		convertee.~value_empty();
		return *new (&convertee) value_float(v);
	}
	static const value_float& construct(void* p, double v)
	{
		return *new (p) value_float(v);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear()				{ value=0; }
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_empty() const		{ return false; }
	virtual bool is_float() const		{ return true; }
	virtual bool is_number() const		{ return true; }
	virtual var_t type() const			{ return VAR_FLOAT; }
	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const		{ return this->value != 0.0 /*&& !nan(value)*/; }
	virtual int64 get_int() const		{ return (int64)floor( this->value+((this->value>=0)?+0.5:-0.5)); }
	virtual double get_float() const	{ return this->value; }
	virtual string<> get_string() const	{ return string<>(this->value); }

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	virtual void negate()	{ this->value = -this->value; }
	virtual void invert()	{ this->assign(~this->get_int()); }
	virtual void lognot()	{ this->assign((int64)(this->value=!0)); }

	///////////////////////////////////////////////////////////////////////////
	// prefix increment operator. (no postfix)
	virtual const value_empty& operator++()	{ ++this->value; return *this; }
	virtual const value_empty& operator--()	{ --this->value; return *this; }

	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator+=(const variant& v);
	virtual const value_empty& operator-=(const variant& v);
	virtual const value_empty& operator*=(const variant& v);
	virtual const value_empty& operator/=(const variant& v);
	virtual const value_empty& operator%=(const variant& v);
	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	virtual const value_empty& operator &=(const variant& v);
	virtual const value_empty& operator |=(const variant& v);
	virtual const value_empty& operator ^=(const variant& v);
	virtual const value_empty& operator>>=(const variant& v);
	virtual const value_empty& operator<<=(const variant& v);
};

///////////////////////////////////////////////////////////////////////////////
/// value_string type.
/// implements the "string" type
struct value_string : public value_empty
{
	friend struct variant;
protected:
	string<> value;
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_string()
	{}
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_string(const char* v) : value(v)		{}
	explicit value_string(const string<>& v) : value(v)	{}
public:
	virtual ~value_string()
	{
		//printf("destroy %p value_string\n", this);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	const value_string&  operator=(const char* v)		{ this->assign(v); return *this; }
	const value_string&  operator=(const string<>& v)	{ this->assign(v); return *this; }
	///////////////////////////////////////////////////////////////////////////
	//
	using value_empty::assign;
	virtual void assign(const char* v)
	{
		this->value = v;
	}
	virtual void assign(const string<>& v)
	{
		this->value = v;
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{
		convertee.~value_empty();
		return *new (&convertee) value_string(this->value);
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_string(this->value);
	}
	static const value_string& convert(value_empty& convertee, const char* val)
	{
		convertee.~value_empty();
		return *new (&convertee) value_string(val);
	}
	static const value_string& convert(value_empty& convertee, const string<>& val)
	{
		convertee.~value_empty();
		return *new (&convertee) value_string(val);
	}
	static const value_string& construct(void* p, const string<>& val)
	{
		return *new (p) value_string(val);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear()				{ value.clear(); }
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_empty() const		{ return false; }
	virtual bool is_string() const		{ return true; }
	virtual size_t size() const			{ return 1; }
	virtual var_t type() const			{ return VAR_STRING; }
	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const		{ return value.size()>0; }
	virtual int64 get_int() const		{ const double d=stringtod(this->value.c_str()); return (int64)floor(d+((d>=0)?+0.5:-0.5)); }
	virtual double get_float() const	{ return stringtod(this->value.c_str()); }
	virtual string<> get_string() const	{ return this->value; }
	virtual const char* get_cstring() const	{ return this->value.c_str(); }


	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator+=(const variant& v);
	virtual const value_empty& operator&=(const variant& v);
};

///////////////////////////////////////////////////////////////////////////////
/// value_conststring type.
/// implements a constant string type, that is using an external storage,
/// nonwritable, converts to string on write access
struct value_conststring : public value_empty
{
	friend struct variant;
protected:
	const char* value;
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_conststring() : value(NULL)
	{}
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_conststring(const char* v) : value(v)	{}
public:
	virtual ~value_conststring()
	{
		//printf("destroy %p value_conststring\n", this);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	const value_conststring&  operator=(const char* v)		{ this->assign(v); return *this; }
	///////////////////////////////////////////////////////////////////////////
	//
	using value_empty::assign;
	virtual void assign(const char* v)
	{
		this->value = v;
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{
		convertee.~value_empty();
		return *new (&convertee) value_conststring(this->value);
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_conststring(this->value);
	}
	static const value_conststring& convert(value_empty& convertee, const char* val)
	{
		convertee.~value_empty();
		return *new (&convertee) value_conststring(val);
	}
	static const value_conststring& construct(void* p, const char* val)
	{
		return *new (p) value_conststring(val);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear()				{ value = ""; }
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_empty() const		{ return false; }
	virtual bool is_string() const		{ return true; }
	virtual size_t size() const			{ return 1; }
	virtual var_t type() const			{ return VAR_STRING; }
	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const		{ return this->value && *this->value; }
	virtual int64 get_int() const		{ const double d=stringtod(this->value); return (int64)floor(d+((d>=0)?+0.5:-0.5)); }
	virtual double get_float() const	{ return stringtod(this->value); }
	virtual string<> get_string() const	{ return value; }
	virtual const char* get_cstring() const	{ return value; }


	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator+=(const variant& v)
	{
		value_string::convert(*this, this->value);
		return this->operator+=(v);
	}
	virtual const value_empty& operator&=(const variant& v)
	{
		value_string::convert(*this, this->value);
		return this->operator&=(v);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// value_array type.
/// contains an array of variants
struct value_array : public value_empty
{
	friend struct variant;
	friend int compare(const variant& va, const variant& vb);
protected:
	vector<variant> value;
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_array();
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_array(const vector<variant>& v);
public:
	virtual ~value_array();
	///////////////////////////////////////////////////////////////////////////
	//
	const value_empty& operator=(const variant& v)	{ this->assign(v); return *this; }
	const value_empty& operator=(const vector<variant>& v);

	///////////////////////////////////////////////////////////////////////////
	//
	using value_empty::assign;
	virtual void assign(const vector<variant>& v);
	virtual void assign(const variant &v);
	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const;
	virtual const value_empty& duplicate(void* p) const;
	static const value_array& convert(value_empty& convertee, const size_t sz, const variant& elem);
	static const value_array& convert(value_empty& convertee, const vector<variant>& v);
	static const value_array& construct(void* p, const vector<variant>& v);

	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear();
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_empty() const		{ return false; }
	virtual bool is_array() const		{ return true; }
	virtual size_t size() const;
	virtual var_t type() const			{ return VAR_ARRAY; }
	///////////////////////////////////////////////////////////////////////////
	//
	virtual bool resize_array(size_t cnt);
	virtual bool append_array(size_t cnt);
	///////////////////////////////////////////////////////////////////////////
	// Access to array elements
	virtual const variant* operator[](size_t inx) const;
	virtual       variant* operator[](size_t inx);

	///////////////////////////////////////////////////////////////////////////
	// local conversion 
	virtual void convert2string();
	virtual void convert2float();
	virtual void convert2int();
	virtual void convert2number();
	virtual void numeric_cast(const variant& v);
	
	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const		{ return this->size()>0; }
	virtual int64 get_int() const;
	virtual double get_float() const;
	virtual string<> get_string() const;
	virtual string<> get_arraystring() const;

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	virtual void negate();
	virtual void invert();
	virtual void lognot();

	///////////////////////////////////////////////////////////////////////////
	// prefix increment operator. (no postfix)
	virtual const value_empty& operator++();
	virtual const value_empty& operator--();

	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator_assign(const variant& v);
	virtual const value_empty& operator+=(const variant& v);
	virtual const value_empty& operator-=(const variant& v);
	virtual const value_empty& operator*=(const variant& v);
	virtual const value_empty& operator/=(const variant& v);
	virtual const value_empty& operator%=(const variant& v);

	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	virtual const value_empty& operator &=(const variant& v);
	virtual const value_empty& operator |=(const variant& v);
	virtual const value_empty& operator ^=(const variant& v);
	virtual const value_empty& operator>>=(const variant& v);
	virtual const value_empty& operator<<=(const variant& v);

	void operate(const variant& v, const variant& (variant::*func)(const variant&));
};





///////////////////////////////////////////////////////////////////////////////
/// value_extern type.
/// implements the connection to a variant_host object via a weak pointer,
/// so when the parent is destroyed, any instance pointing to it is
/// automatically invalidated.
/// the connection to the parent is switchable: 
///  - assigning a value parent creates a valid external variant
///    which has no type
///  - with the "access_member" function the parent object is called
///    to assign a pointer to the actual variable element inside the parent.
///    This assignment also changes the type of the value_extern
struct value_extern : public value_empty
{
protected:
	friend struct value_empty;
	variant_host::reference parent_ref;
public:
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_extern()
	{}
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_extern(const variant_host& parent)
		: parent_ref(parent.access())
	{}
	explicit value_extern(const variant_host::reference& parent)
		: parent_ref(parent)
	{}
public:
	virtual ~value_extern()
	{
		//printf("destroy %p value_extern\n", this);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	using value_empty::assign;
	virtual void assign(const variant_host& v)
	{
		this->parent_ref = v.access();
	}
	virtual void assign(const variant_host::reference& v)
	{
		this->parent_ref = v;
	}

	///////////////////////////////////////////////////////////////////////////
	virtual variant_host::reference get_parent() const
	{
		return this->parent_ref;
	}
	virtual bool access_member(const string<>& name)
	{
		return ((this->parent_ref.exists() && *this->parent_ref && (*this->parent_ref)->get_member(name, *this)) ||
				(this->value_empty::access_member(name)));
	}

	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{	
		convertee.~value_empty();
		return *new (&convertee) value_extern(this->parent_ref);
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_extern(this->parent_ref);
	}
	static const value_extern& convert(value_empty& convertee, const variant_host& parent)
	{
		convertee.~value_empty();
		return *new (&convertee) value_extern(parent);
	}
	static const value_extern& convert(value_empty& convertee, const variant_host::reference& parent)
	{
		convertee.~value_empty();
		return *new (&convertee) value_extern(parent);
	}
	static const value_extern& construct(void* p, const variant_host& parent)
	{
		return *new (p) value_extern(parent);
	}
	static const value_extern& construct(void* p, const variant_host::reference& parent)
	{
		return *new (p) value_extern(parent);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear()				{ value_empty::convert(*this); }
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_valid() const		{ return (this->parent_ref.exists() && *this->parent_ref); }
	virtual bool is_extern() const		{ return true; }
};

///////////////////////////////////////////////////////////////////////////////
/// value_unnamed type.
/// an unnamed value is an external value 
/// with a direct pointer to the actual storage element, 
/// only pod's and string allowed
///
template<typename T>
struct value_unnamed : public value_extern
{
protected:
	friend struct value_empty;
	T *value;
public:
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_unnamed() : value(NULL)
	{}
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_unnamed(const variant_host& parent, const T *v)
		: value_extern(parent.access()), value(const_cast<T*>(v))
	{}
	explicit value_unnamed(const variant_host::reference& parent, const T *v)
		: value_extern(parent), value(const_cast<T*>(v))
	{}
public:
	virtual ~value_unnamed()
	{
		//printf("destroy %p value_unnamed\n", this);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	using value_extern::assign;
	virtual void assign(const value_empty &v);
	virtual void assign(const int v)		{ this->assign((sint64)v); }
	virtual void assign(const uint v)		{ this->assign((sint64)v); }
	virtual void assign(const sint64 v);
	virtual void assign(const uint64 v)		{ this->assign((sint64)v); }
	virtual void assign(const double v);
	virtual void assign(const char* v);
	virtual void assign(const string<>& v);
	virtual void assign(const vector<variant>& v);
	virtual void assign(const variant_host& v)
	{
		this->value_extern::assign(v);
		this->value = NULL;
	}
	virtual void assign(const variant_host::reference& v)
	{
		this->value_extern::assign(v);
		this->value = NULL;
	}

	///////////////////////////////////////////////////////////////////////////
	virtual bool access_member(const string<>& name)
	{
		return this->value_empty::access_member(name);
	}

	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const
	{	// only duplicate the content
		if(this->parent_ref.exists() && *this->parent_ref && this->value)
			convertee = *value;
		else
			convertee.clear();
		return convertee;
	}
	virtual const value_empty& duplicate(void* p) const
	{
		return *new (p) value_unnamed(this->parent_ref, this->value);
	}
	template<typename X>
	static const value_unnamed<T>& convert(value_empty& convertee, const variant_host& parent, X *v)
	{
		convertee.~value_empty();
		return *new (&convertee) value_unnamed<X>(parent, v);
	}
	template<typename X>
	static const value_unnamed<X>& convert(value_extern& convertee, X *v)
	{
		variant_host::reference parent(convertee.parent_ref);
		convertee.~value_extern();
		return *new (&convertee) value_unnamed<X>(parent, v);
	}
	template<typename X>
	static const value_unnamed<X>& construct(void* p, const variant_host& parent, const X *v)
	{
		return *new (p) value_unnamed<X>(parent, v);
	}
	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear()				{ value_empty::convert(*this); }
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_valid() const		{ return (this->parent_ref.exists() && *this->parent_ref); }
	virtual bool is_empty() const		{ return false; }
	virtual bool is_int() const			{ return (this->parent_ref.exists() && *this->parent_ref && value) && is_integral<T>::Result; }
	virtual bool is_float() const		{ return (this->parent_ref.exists() && *this->parent_ref && value) && is_rational<T>::Result; }
	virtual bool is_number() const		{ return (this->parent_ref.exists() && *this->parent_ref && value) && (is_integral<T>::Result || is_rational<T>::Result); }
	virtual bool is_string() const		{ return (this->parent_ref.exists() && *this->parent_ref && value) && is_string_type<T>::Result; }
	virtual bool is_extern() const		{ return (this->parent_ref.exists() && *this->parent_ref); }
	virtual var_t type() const
	{
		if(this->parent_ref.exists() && *this->parent_ref && value)
		{
			if( is_rational<T>::Result ) return VAR_FLOAT;
			else if( is_integral<T>::Result ) return VAR_INTEGER;
			else if( is_string_type<T>::Result ) return VAR_STRING;
		}
		return VAR_NONE;
	}

	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const;
	virtual int64 get_int() const;
	virtual double get_float() const;
	virtual string<> get_string() const;

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	virtual void negate();
	virtual void invert();
	virtual void lognot();

	///////////////////////////////////////////////////////////////////////////
	// prefix increment operator. (no postfix)
	virtual const value_empty& operator++();
	virtual const value_empty& operator--();
	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator+=(const variant& v);
	virtual const value_empty& operator-=(const variant& v);
	virtual const value_empty& operator*=(const variant& v);
	virtual const value_empty& operator/=(const variant& v);
	virtual const value_empty& operator%=(const variant& v);

	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	virtual const value_empty& operator &=(const variant& v);
	virtual const value_empty& operator |=(const variant& v);
	virtual const value_empty& operator ^=(const variant& v);
	virtual const value_empty& operator>>=(const variant& v);
	virtual const value_empty& operator<<=(const variant& v);
};


///////////////////////////////////////////////////////////////////////////////
/// value_named type.
/// implements the connection to a variant_host object via a weak pointer,
/// so when the parent is destroyed, any instance pointing to it is
/// automatically invalidated.
///
struct value_named : public value_extern
{
protected:
	friend struct value_empty;
	string<> name;
	variant* value;

	bool get_value();
	bool set_value();
public:
	///////////////////////////////////////////////////////////////////////////
	// default constructor/destructor
	value_named();
	///////////////////////////////////////////////////////////////////////////
	// type constructors
	explicit value_named(const variant_host& parent, const string<>& n);
	explicit value_named(const variant_host& parent, const string<>& n, const variant& v);
	explicit value_named(const variant_host::reference& parent, const string<>& n);
	explicit value_named(const variant_host::reference& parent, const string<>& n, const variant& v);
public:
	virtual ~value_named();
	///////////////////////////////////////////////////////////////////////////
	//
	using value_extern::assign;
	virtual void assign(const value_empty &v);
	virtual void assign(const int v)		{ this->assign((sint64)v); }
	virtual void assign(const uint v)		{ this->assign((sint64)v); }
	virtual void assign(const sint64 v);
	virtual void assign(const uint64 v)		{ this->assign((sint64)v); }
	virtual void assign(const double v);
	virtual void assign(const char* v);
	virtual void assign(const string<>& v);
	virtual void assign(const vector<variant>& v);
	virtual void assign(const variant_host& v);

	///////////////////////////////////////////////////////////////////////////
	virtual bool access_member(const string<>& n)
	{
		return this->value_empty::access_member(name);
	}

	///////////////////////////////////////////////////////////////////////////
	//
	virtual const value_empty& duplicate(value_empty& convertee) const;
	virtual const value_empty& duplicate(void* p) const;
	static const value_named& convert(value_empty& convertee, const variant_host& parent, const string<>& n, const variant& v);
	static const value_named& convert(value_named& convertee, const string<>& n, const variant& v);
	static const value_empty& construct(void* p, const variant_host& parent, const string<>& n);

	///////////////////////////////////////////////////////////////////////////
	//
	virtual void clear();
	///////////////////////////////////////////////////////////////////////////
	virtual bool is_valid() const;
	virtual bool is_empty() const;
	virtual bool is_int() const;
	virtual bool is_float() const;
	virtual bool is_number() const;
	virtual bool is_string() const;
	virtual bool is_extern() const		{ return (this->parent_ref.exists() && *this->parent_ref); }
	virtual var_t type() const;

	///////////////////////////////////////////////////////////////////////////
	// access conversion 
	virtual bool get_bool() const;
	virtual int64 get_int() const;
	virtual double get_float() const;
	virtual string<> get_string() const;

	///////////////////////////////////////////////////////////////////////////
	// unary operations
	virtual void negate();
	virtual void invert();
	virtual void lognot();

	///////////////////////////////////////////////////////////////////////////
	// prefix increment operator. (no postfix)
	virtual const value_empty& operator++();
	virtual const value_empty& operator--();
	///////////////////////////////////////////////////////////////////////////
	// arithmetic operations
	virtual const value_empty& operator+=(const variant& v);
	virtual const value_empty& operator-=(const variant& v);
	virtual const value_empty& operator*=(const variant& v);
	virtual const value_empty& operator/=(const variant& v);
	virtual const value_empty& operator%=(const variant& v);

	///////////////////////////////////////////////////////////////////////////
	// binary/logic operations
	virtual const value_empty& operator &=(const variant& v);
	virtual const value_empty& operator |=(const variant& v);
	virtual const value_empty& operator ^=(const variant& v);
	virtual const value_empty& operator>>=(const variant& v);
	virtual const value_empty& operator<<=(const variant& v);
};


///////////////////////////////////////////////////////////////////////////////
/// value_union. 
/// this is not a type but implementing the storage for all implemented types 
/// by providing the necessary memory for their instanciation and 
/// the access interface. all access has to be done via the virtual function table
struct value_union
{
private:
	enum _dummy
	{
		SizeA = sizeof(value_empty),
		SizeB = (SizeA > sizeof(value_integer))    ? SizeA : sizeof(value_integer),
		SizeC = (SizeB > sizeof(value_float))  ? SizeB : sizeof(value_float),
		SizeD = (SizeC > sizeof(value_string)) ? SizeC : sizeof(value_string),
		SizeE = (SizeD > sizeof(value_array)) ? SizeD : sizeof(value_array),
		SizeF = (SizeE > sizeof(value_unnamed<int>)) ? SizeE : sizeof(value_unnamed<int>),
		Size  = (SizeF > sizeof(value_named)) ? SizeF : sizeof(value_named)
	};
	char dummy[Size];
public:
	///////////////////////////////////////////////////////////////////////////
	/// default constructor/destructor
	value_union()
	{
		value_empty::construct(dummy);
	}
	~value_union()
	{
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		//printf("destroy %p value_union\n", dummy);
		this->access().~value_empty();
		//printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	}
// don't use a template here
// because then construction is done by first creating a value_empty object
// which is calling the value_empty::assign that is bound to type T, which is
// destructing the just created value_empty and creating the type associated to T.
// putting all explicit type creation constructors here is a little more effort
// but allows direct creation of the necessary object [Shinomori]
//	template<typename T>
//	explicit value_union(const T& v)
//	{
//		new (dummy) value_empty(v);
//	}
	///////////////////////////////////////////////////////////////////////////
	/// copy constructor
	value_union(const value_union& v)
	{
		v.access().duplicate(dummy);
	}
	///////////////////////////////////////////////////////////////////////////
	/// copy assignment
	const value_union& operator=(const value_union& v)
	{
		v.access().duplicate(this->access());
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// type creation constructors.
	/// calling the constructor of the associated class
	explicit value_union(const value_empty &v)		{ v.duplicate(dummy); }
	explicit value_union(const int v)				{ value_integer::construct(dummy, v); }
	explicit value_union(const uint v)				{ value_integer::construct(dummy, v); }
	explicit value_union(const long v)				{ value_integer::construct(dummy, v); }
	explicit value_union(const ulong v)				{ value_integer::construct(dummy, v); }
	explicit value_union(const sint64 v)			{ value_integer::construct(dummy, v); }
	explicit value_union(const uint64 v)			{ value_integer::construct(dummy, v); }
	explicit value_union(const float v)				{ value_float::construct(dummy, v); }
	explicit value_union(const double v)			{ value_float::construct(dummy, v); }
	explicit value_union(const long double v)		{ value_float::construct(dummy, v); }
	explicit value_union(const char* v)				{ value_conststring::construct(dummy, v); }
	explicit value_union(const string<>& v)			{ value_string::construct(dummy, v); }
	explicit value_union(const vector<variant>& v)	{ value_array::construct(dummy, v); }
	explicit value_union(const variant_host& v)		{ value_extern::construct(dummy, v); }
	explicit value_union(const variant_host::reference& v){ value_extern::construct(dummy, v); }
	template <typename P, typename T>
	explicit value_union(const P& v, const T P::*t)
	{
		value_unnamed<T>::construct(dummy, v, &(v.*t));
	}
	explicit value_union(const variant_host& v, const string<>& n)
	{
		value_named::construct(dummy, v, n);
	}


	///////////////////////////////////////////////////////////////////////////
	/// access to stored type.
	value_empty& access()				{ return *((value_empty*)dummy); }
	const value_empty& access() const	{ return *((value_empty*)dummy); }
	operator value_empty&()				{ return this->access(); }
	operator const value_empty&() const	{ return this->access(); }
};


///////////////////////////////////////////////////////////////////////////////
/// variant.
/// implements a smart pointer to a value_union object 
/// which provides both call-by-value and call-by-reference schemes
struct variant
{
private:
	friend struct value_empty;
	friend struct value_array;
	friend struct value_named;
	TObjPtrCommon<value_union> value;
	// only use this to cast to the basetype 
	// to force calls from the virtual function table
	value_empty& access()				{ return value->access(); }
	const value_empty& access() const	{ return value->access(); }
public:
	///////////////////////////////////////////////////////////////////////////
	/// default constructor. creates an empty element
	variant()  {}
	///////////////////////////////////////////////////////////////////////////
	/// destructor
	~variant() {}

	///////////////////////////////////////////////////////////////////////////
	/// type create construction template.
	/// gives the type creation down to the pointer type
	template<typename T>
	variant(const T& v)							{ this->value.create(v); }
	///////////////////////////////////////////////////////////////////////////
	/// copy constructor. 
	/// set the access behaviour to copy-on-write
	variant(const variant& v) : value(v.value)	{ this->make_value(); }

	///////////////////////////////////////////////////////////////////////////
	/// assignment template.
	template<typename T>
	const variant& operator=(const T& v)		{ this->assign(v); return *this; }
	///////////////////////////////////////////////////////////////////////////
	/// copy assignment.
	/// set the access behaviour to copy-on-write
	const variant& operator=(const variant& v)	{ this->assign(v); return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// constructor for direct access to externals
	template <typename P, typename T>
	explicit variant(const P& v, const T P::*t)	{ this->value.create(v, t); }
	explicit variant(const variant_host& v, const string<>& n) { this->value.create(v, n); }
	explicit variant(const variant_host& v, const char* n) { this->value.create(v, string<>(n)); }

	///////////////////////////////////////////////////////////////////////////
	/// constructor with specifying access behaviour
	explicit variant(const variant &v, bool ref)
	{
		this->assign(v, ref);
	}

	///////////////////////////////////////////////////////////////////////////
	/// query the type of variant
	bool is_reference() const	{ return this->value.isAutoCreate(); }
	///////////////////////////////////////////////////////////////////////////
	/// enable copy-on-write
	void make_value() const		{ this->value.setCopyonWrite(); }
	///////////////////////////////////////////////////////////////////////////
	/// disable copy-on-write
	void make_reference() const	{ this->value.setAutoCreate(); }

	///////////////////////////////////////////////////////////////////////////
	//
	template<typename T>
	void assign(const T &v)
	{
		if( this->value.exists() )
			this->access().assign(v);
		else
			this->value.create(v);
	}
	void assign(const variant &v)
	{
		if( this->is_reference() )
		{	// copy the content
			if( this->value.pointer() != v.value.pointer() )
				this->access() = v.access();
		}
		else
		{	// share the pointers
			this->value = v.value;
			this->make_value();
		}
	}
	void assign(const variant &v, bool set_reference)
	{	// just share the pointer
		this->value = v.value;
		if(set_reference)
			this->make_reference();
		else
			this->make_value();
	}
	///////////////////////////////////////////////////////////////////////////
	/// clear the variant.
	void clear()				{ this->value.clear(); this->make_value(); }
	void empty()				{ if(this->value.exists()) this->access().empty(); }
	void invalidate()			{ if(this->value.exists()) this->access().invalidate(); }
	///////////////////////////////////////////////////////////////////////////
	bool is_valid() const		{ return !this->value.exists() || this->access().is_valid(); }
	bool is_empty() const		{ return !this->value.exists() || this->access().is_empty(); }
	bool is_int() const			{ return this->value.exists() && this->access().is_int(); }
	bool is_float() const		{ return this->value.exists() && this->access().is_float(); }
	bool is_number() const		{ return this->value.exists() && this->access().is_number(); }
	bool is_string() const		{ return this->value.exists() && this->access().is_string(); }
	bool is_array() const		{ return this->value.exists() && this->access().is_array(); }
	bool is_extern() const		{ return this->value.exists() && this->access().is_extern(); }
	size_t size() const			{ return this->value.exists() ? this->access().size() : 0; }
	///////////////////////////////////////////////////////////////////////////
	variant_host::reference get_parent() const	{ return this->value.exists() ? this->access().get_parent() : variant_host::reference(); }
	bool access_member(const string<>& name)	{ return this->access().access_member(name); }
	///////////////////////////////////////////////////////////////////////////
	/// add a dimension at the front. expand the content to all array elements
	void create_array(size_t cnt)
	{	// add a dimension at the front
		variant tmp(*this);			// make a copy of the current element
		this->value.clear();		// clear the current pointer
		value_array::convert(this->access(), cnt, tmp);	// create a new pointer and build the array there
	}
	///////////////////////////////////////////////////////////////////////////
	/// change size of an array or create an array if not yet exists
	void resize_array(size_t cnt)
	{	// create an array if resize does not exist
		if( !this->access().resize_array(cnt) )
			this->create_array(cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// add a dimension at the end. expand the content to all array elements,
	/// create an array if append does not exist
	void append_array(size_t cnt)
	{	
		if( !this->access().append_array(cnt) )
			this->create_array(cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// access to array elements
	const variant& operator[](size_t inx) const	
	{
		const variant*ptr = this->access()[inx];
		return ptr?*ptr:*this;
	}
	variant& operator[](size_t inx)
	{
		variant*ptr = this->access()[inx];
		return ptr?*ptr:*this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// access conversion 
	bool get_bool() const		{ return this->value.exists() ? this->access().get_bool() : false; }
	int64 get_int() const		{ return this->value.exists() ? this->access().get_int() : 0; }
	double get_float() const	{ return this->value.exists() ? this->access().get_float() : 0.0; }
	string<> get_string() const	{ return this->value.exists() ? this->access().get_string() : string<>(); }
	string<> get_arraystring() const	{ return this->value.exists() ? this->access().get_arraystring() : string<>(); }
	const char* get_cstring() const	{ return this->value.exists() ? this->access().get_cstring() : ""; }

private:
	struct variant_conversion
	{
		const variant& parent;
		variant_conversion(const variant& p) : parent(p)
		{}
		operator bool() const		{ return this->parent.get_bool(); }
		operator int() const		{ return (int)this->parent.get_int(); }
		operator int64() const		{ return this->parent.get_int(); }
		operator double() const		{ return this->parent.get_float(); }
		operator string<>() const	{ return this->parent.get_string(); }
	};
public:
	variant_conversion operator()()	{ return variant_conversion(*this); }

	template<typename T>
	void get_value(T& v) const
	{
		if( this->value.exists() )
			this->access().get_value(v);
		else
			v = T();
	}
	template<typename T>
	void set_value(const T& v)
	{
		this->access().set_value(v);
	}

	///////////////////////////////////////////////////////////////////////////
	/// local conversions
	void convert2string()	{ this->access().convert2string(); }
	void convert2float()	{ this->access().convert2float(); }
	void convert2int()		{ this->access().convert2int(); }
	void convert2number()	{ this->access().convert2number(); }
	void cast(var_t type);
	void numeric_cast(const variant& v)	{ if(v.is_valid()) this->access().numeric_cast(v); else this->access().convert2number(); }
	var_t type() const		{ return (this->is_valid())?this->access().type():VAR_NONE; }
	static var_t name2type(const string<>& name)
	{
		return  (name=="double")?VAR_FLOAT:
				(name=="int")?VAR_INTEGER:
				(name=="string")?VAR_STRING:VAR_AUTO;
	}
	static const char* type2name(var_t type);
	///////////////////////////////////////////////////////////////////////////
	/// unary operations
	void negate()	{ if( this->value.exists() ) this->access().negate(); }
	void invert()	{ if( this->value.exists() ) this->access().invert(); }
	void lognot()	{ if( this->value.exists() ) this->access().lognot(); }

	const variant operator-() const;
	const variant operator~() const;
	const variant operator!() const;

	///////////////////////////////////////////////////////////////////////////
	/// postfix increment operator.
	variant operator++(int);
	variant operator--(int);
	///////////////////////////////////////////////////////////////////////////
	/// prefix increment operator.
	const variant& operator++();
	const variant& operator--();

	///////////////////////////////////////////////////////////////////////////
	/// arithmetic operations
	const variant& operator_assign(const variant& v);
	const variant& operator+=(const variant& v);
	const variant& operator-=(const variant& v);
	const variant& operator*=(const variant& v);
	const variant& operator/=(const variant& v);
	const variant& operator%=(const variant& v);

	///////////////////////////////////////////////////////////////////////////
	/// binary/logic operations
	const variant& operator&=(const variant& v);
	const variant& operator|=(const variant& v);
	const variant& operator^=(const variant& v);
	const variant& operator>>=(const variant& v);
	const variant& operator<<=(const variant& v);

	///////////////////////////////////////////////////////////////////////////
	/// compare operations
	friend int compare(const variant& va, const variant& vb);
};


inline variant operator+(const variant& va, const variant& vb)
{
	variant temp(va);
	temp += vb;
	return temp;
}
inline variant operator-(const variant& va, const variant& vb)
{
	variant temp(va);
	temp -= vb;
	return temp;

}
inline variant operator*(const variant& va, const variant& vb)
{
	variant temp(va);
	temp *= vb;
	return temp;

}
inline variant operator/(const variant& va, const variant& vb)
{
	variant temp(va);
	temp /= vb;
	return temp;
}
inline variant operator%(const variant& va, const variant& vb)
{
	variant temp(va);
	temp %= vb;
	return temp;
}
inline variant operator&(const variant& va, const variant& vb)
{
	variant temp(va);
	temp &= vb;
	return temp;
}
inline variant operator| (const variant& va, const variant& vb)
{
	variant temp(va);
	temp |= vb;
	return temp;
}
inline variant operator^ (const variant& va, const variant& vb)
{
	variant temp(va);
	temp ^= vb;
	return temp;
}
inline variant operator>>(const variant& va, const variant& vb)
{
	variant temp(va);
	temp >>= vb;
	return temp;
}
inline variant operator<<(const variant& va, const variant& vb)
{
	variant temp(va);
	temp <<= vb;
	return temp;
}

inline bool operator&&(const variant& va, const variant& vb)
{
	return va.get_bool() && vb.get_bool();
}

inline bool operator||(const variant& va, const variant& vb)
{
	return va.get_bool() || vb.get_bool();
}

///////////////////////////////////////////////////////////////////////////
// boolean compare
inline bool operator==(const variant& a, const variant& b)	{ return 0==compare(a,b); }
inline bool operator!=(const variant& a, const variant& b)	{ return 0!=compare(a,b); }
inline bool operator< (const variant& a, const variant& b)	{ return 0> compare(a,b); }
inline bool operator<=(const variant& a, const variant& b)	{ return 0>=compare(a,b); }
inline bool operator> (const variant& a, const variant& b)	{ return 0< compare(a,b); }
inline bool operator>=(const variant& a, const variant& b)	{ return 0<=compare(a,b); }











///////////////////////////////////////////////////////////////////////////////
// inline implementations
inline void value_empty::assign(const value_empty &v)
{
	v.duplicate(*this);
}
inline void value_empty::assign(const sint64 v)
{
	value_integer::convert(*this, v);
}
inline void value_empty::assign(const double v)
{
	value_float::convert(*this, v);
}
inline void value_empty::assign(const char* v)
{
	value_conststring::convert(*this, v);
}
inline void value_empty::assign(const string<>& v)
{
	value_string::convert(*this, v);
}
inline void value_empty::assign(const vector<variant>& v)
{
	value_array::convert(*this, v);
}
inline void value_empty::assign(const variant_host& v)
{
	value_extern::convert(*this, v);
}
inline void value_empty::assign(const variant_host::reference& v)
{
	value_extern::convert(*this, v);
}
inline void value_empty::assign(const variant &v)
{
	this->assign(v.access());
}

///////////////////////////////////////////////////////////////////////////////
// 
inline void value_empty::invalidate()
{
	value_invalid::convert(*this);
}
inline void value_empty::empty()
{
	value_empty::convert(*this);
}

///////////////////////////////////////////////////////////////////////////////
// unary operations
inline void value_empty::negate()
{
	value_integer::convert(*this, 0);
}
inline void value_empty::invert()
{
	value_integer::convert(*this, ~((sint64)0));
}
inline void value_empty::lognot()
{
	value_integer::convert(*this, 1);
}

///////////////////////////////////////////////////////////////////////////////
// prefix increment operator. (no postfix)
inline const value_empty& value_empty::operator++()
{
	return value_integer::convert(*this, 1);
}
inline const value_empty& value_empty::operator--()
{
	return value_integer::convert(*this, -1);
}

///////////////////////////////////////////////////////////////////////////////
// arithmetic operations
inline const value_empty& value_empty::operator_assign(const variant& v)
{
	this->assign(v.access());
	return *this;
}
inline const value_empty& value_empty::operator+=(const variant& v)
{
	this->assign(v.access());
	return *this;
}
inline const value_empty& value_empty::operator-=(const variant& v)
{
	this->assign(v.access());
	this->negate();
	return *this;
}
inline const value_empty& value_empty::operator*=(const variant& v)
{
	return value_integer::convert(*this, 0);
}
inline const value_empty& value_empty::operator/=(const variant& v)
{
	return value_integer::convert(*this, 0);
}
inline const value_empty& value_empty::operator%=(const variant& v)
{
	return value_integer::convert(*this, 0);
}

///////////////////////////////////////////////////////////////////////////////
// binary/logic operations
inline const value_empty& value_empty::operator &=(const variant& v)
{
	return value_integer::convert(*this, 0);
}
inline const value_empty& value_empty::operator |=(const variant& v)
{
	this->assign(v.access());
	return *this;
}
inline const value_empty& value_empty::operator ^=(const variant& v)
{
	this->assign(v.access());
	return *this;
}
inline const value_empty& value_empty::operator>>=(const variant& v)
{
	return value_integer::convert(*this, 0);
}
inline const value_empty& value_empty::operator<<=(const variant& v)
{
	return value_integer::convert(*this, 0);
}

///////////////////////////////////////////////////////////////////////////////
// not implemented as member template
// because member templates with function definition outside the struct body
// cause problems on at least MSVC, and we need it externally here
template<typename P, typename T>
bool set_externmember(value_empty& target, const P& v, const T P::*t)
{
	variant_host::reference parent = v.access();
	target.~value_empty();
	new (&target) value_unnamed<T>(parent, &(v.*t));
	return true;
}
template<typename P, typename T>
bool set_namedmember(value_empty& target, const P& v, const T P::*t, const string<>& name)
{
	variant_host::reference parent = v.access();
	target.~value_empty();
	new (&target) value_named(parent, name, variant(v.*t));
	return true;
}


///////////////////////////////////////////////////////////////////////////////


inline value_array::value_array()
{}
inline value_array::~value_array()
{
	//printf("destroy %p value_array\n", this);
}
inline const value_empty& value_array::operator=(const vector<variant>& v)
{
	this->assign(v);
	return *this;
}
inline void value_array::assign(const variant &v)
{
	this->operate(v, &variant::operator=);
}
inline void value_array::assign(const vector<variant>& v)
{
	this->value = v;
}
inline const value_empty& value_array::duplicate(value_empty& convertee) const
{
	convertee.~value_empty();
	return *new (&convertee) value_array(this->value);
}
inline const value_empty& value_array::duplicate(void* p) const
{
	return *new (p) value_array(this->value);
}
inline void value_array::clear()
{
	value.clear();
}
inline size_t value_array::size() const
{
	return this->value.size();
}
inline const variant* value_array::operator[](size_t inx) const
{
	return (inx<this->value.size()) ? &this->value[inx] : NULL;
}
inline variant* value_array::operator[](size_t inx)
{
	return (inx<this->value.size()) ? &this->value[inx] : NULL;
}
inline int64 value_array::get_int() const
{
	return this->value.size()?this->value[0].get_int():0;
}
inline double value_array::get_float() const
{
	return this->value.size()?this->value[0].get_float():0.0;
}
inline string<> value_array::get_string() const
{
	return this->value.size()?this->value[0].get_string():"";
}
inline const value_empty& value_array::operator_assign(const variant& v)
{
	this->operate(v, &variant::operator_assign);
	return *this;
}
inline const value_empty& value_array::operator+=(const variant& v)
{
	this->operate(v, &variant::operator+=);
	return *this;
}
inline const value_empty& value_array::operator-=(const variant& v)
{
	this->operate(v, &variant::operator-=);
	return *this;
}
inline const value_empty& value_array::operator*=(const variant& v)
{
	this->operate(v, &variant::operator*=);
	return *this;
}
inline const value_empty& value_array::operator/=(const variant& v)
{
	this->operate(v, &variant::operator/=);
	return *this;
}
inline const value_empty& value_array::operator%=(const variant& v)
{
	this->operate(v, &variant::operator%=);
	return *this;
}
inline const value_empty& value_array::operator &=(const variant& v)
{
	this->operate(v, &variant::operator &=);
	return *this;
}
inline const value_empty& value_array::operator |=(const variant& v)
{
	this->operate(v, &variant::operator |=);
	return *this;
}
inline const value_empty& value_array::operator ^=(const variant& v)
{
	this->operate(v, &variant::operator ^=);
	return *this;
}
inline const value_empty& value_array::operator>>=(const variant& v)
{
	this->operate(v, &variant::operator>>=);
	return *this;
}
inline const value_empty& value_array::operator<<=(const variant& v)
{
	this->operate(v, &variant::operator<<=);
	return *this;
}



template<typename T> 
void value_unnamed<T>::assign(const value_empty &v)
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		v.get_value(*value);
	}
}
template<typename T> 
void value_unnamed<T>::assign(const sint64 v)
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(v);
		tmp.access().get_value(*value);
	}
}
template<typename T> 
void value_unnamed<T>::assign(const double v)
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(v);
		tmp.access().get_value(*value);
	}
}
template<typename T> 
void value_unnamed<T>::assign(const char* v)
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(v);
		tmp.access().get_value(*value);
	}
}
template<typename T> 
void value_unnamed<T>::assign(const string<>& v)
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(v);
		tmp.access().get_value(*value);
	}
}
template<typename T> 
void value_unnamed<T>::assign(const vector<variant>& v)
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		v[0].get_value(*value);
	}
}
template<typename T> 
bool value_unnamed<T>::get_bool() const
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(*this->value);
		return tmp.access().get_bool();
	}
	return false;
}
template<typename T> 
int64 value_unnamed<T>::get_int() const
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(*this->value);
		int64 ret = tmp.access().get_int();
		return ret;
	}
	return 0;
}
template<typename T>
double value_unnamed<T>::get_float() const
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(*this->value);
		double ret = tmp.access().get_float();
		return ret;
	}
	return 0;
}
template<typename T>
string<> value_unnamed<T>::get_string() const
{
	if(this->parent_ref.exists() && *this->parent_ref && value)
	{
		value_union tmp(*this->value);
		string<> ret = tmp.access().get_string();
		return ret;
	}
	return string<>();
}

template<typename T>
void value_unnamed<T>::negate()
{
	*this->value = (T)(-to_signed(*this->value));
}
template<>
inline void value_unnamed< string<> >::negate()
{}

template<typename T>
void value_unnamed<T>::invert()
{
	*this->value = (T)(~((int64)(*this->value)));
}
template<>
inline void value_unnamed< string<> >::invert()
{}

template<typename T>
void value_unnamed<T>::lognot()
{
	*this->value = (T)(*this->value!=0);
}
template<>
inline void value_unnamed< string<> >::lognot()
{}

template<typename T>
const value_empty& value_unnamed<T>::operator++()
{
	++(*this->value);
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator++()
{
	*this->value = string<>(this->get_float()+1);
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator--()
{
	--(*this->value);
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator--()
{
	*this->value = string<>(this->get_float()-1);
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator+=(const variant& v)
{
	*this->value += this->is_float()?(T)v.get_float():(T)v.get_int();
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator+=(const variant& v)
{
	*this->value += v.get_string();
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator-=(const variant& v)
{
	*this->value -= this->is_float()?(T)v.get_float():(T)v.get_int();
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator-=(const variant& v)
{
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator*=(const variant& v)
{
	*this->value *= this->is_float()?(T)v.get_float():(T)v.get_int();
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator*=(const variant& v)
{
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator/=(const variant& v)
{
	if( 0.0!=v.get_float() )
	{
		double x = ((double)*this->value) / v.get_float();
		if(this->is_int()) x += (x>0)?+0.5:-0.5; 
		*this->value = (T)(x);
	}
	else if( this->is_int() && 0!=v.get_int() )
	{
		*this->value /= v.get_int();
	}
	else
	{
		value_empty::convert(*this);
	}
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator/=(const variant& v)
{
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator%=(const variant& v)
{
	if( this->is_float() && 0.0!=v.get_float() )
	{
		*this->value = (T)0;
	}
	else if( this->is_int() && 0!=v.get_int() )
	{
		*this->value %= v.get_int();
	}
	else
	{
		value_empty::convert(*this);
	}
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator%=(const variant& v)
{
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// binary/logic operations
template<typename T>
const value_empty& value_unnamed<T>::operator &=(const variant& v)
{
	if( this->is_float() )
	{
		this->assign( ((int64)*this->value) & v.get_int() );
	}
	else if( this->is_int() )
	{
		*this->value &= v.get_int();
	}
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator &=(const variant& v)
{
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator |=(const variant& v)
{
	if( this->is_float() )
	{
		this->assign( ((int64)*this->value) | v.get_int() );
	}
	else if( this->is_int() )
	{
		*this->value |= v.get_int();
	}
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator |=(const variant& v)
{
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator ^=(const variant& v)
{
	if( this->is_float() )
	{
		this->assign( ((int64)*this->value) ^ v.get_int() );
	}
	else if( this->is_int() )
	{
		*this->value ^= v.get_int();
	}
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator ^=(const variant& v)
{
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator>>=(const variant& v)
{
	if( this->is_float() )
	{
		this->assign( ((int64)*this->value) >> v.get_int() );
	}
	else if( this->is_int() )
	{
		*this->value >>= v.get_int();
	}
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator>>=(const variant& v)
{
	return *this;
}

template<typename T>
const value_empty& value_unnamed<T>::operator<<=(const variant& v)
{
	if( this->is_float() )
	{
		this->assign( ((int64)*this->value) << v.get_int() );
	}
	else if( this->is_int() )
	{
		*this->value <<= v.get_int();
	}
	return *this;
}
template<>
inline const value_empty& value_unnamed< string<> >::operator<<=(const variant& v)
{
	return *this;
}






///////////////////////////////////////////////////////////////////////////////
/// string to variant conversion.
/// combines stringtoib/stringtoio/stringtoix/stringtoi/stringtod
template <typename T>
variant stringtov(T const*str, T const** run=NULL);
template <typename T>
variant tovariant(const string<T>& str)
{
	return stringtov<T>(str.c_str(), NULL);
}
string<> tostring(const variant& v);


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASEVARIANT_H__
