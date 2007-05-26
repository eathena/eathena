
#include "basevariant.h"
#include "basestring.h"

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
/// global variant_defaults anchor. 
/// stores the variant_defaults active handler.
template<>
variant_defaults<bool>* variant_defaults<bool>::global_variant_defaults = NULL;



///////////////////////////////////////////////////////////////////////////////
// supported literal formats:
// DecLiteral       = [0123456789]{digit}*
// BinLiteral       = 0b{Bin Digit}+
// OctLiteral       = 0o{Oct Digit}+
// HexLiteral       = 0x{Hex Digit}+
// FloatLiteral     = ({Digit}* '.' {Digit}+ (([eE][+-]?{Digit}+)|[TGMKkmunpfa])?)
//                  | ({Digit}+ '.' {Digit}* (([eE][+-]?{Digit}+)|[TGMKkmunpfa])?)
//                  | ({Digit}+              (([eE][+-]?{Digit}+)|[TGMKkmunpfa]) )
// StringLiteral   = '"' ({stringchar} | ('\' {char}))* '"'
// CharLiteral     = '\'' ({charchar} | ('\' {char})) '\''
// Array           = '{' [ <literal> { ',' <literal>} ] '}'
template <typename T>
static void stringtov_internal(variant& v, T const*& str)
{
	if( str )
	{
		// skip leading whitespace
		while( stringcheck::isspace(*str) ) ++str;

		v.clear();

		// array
		if( str[0]=='{' )
		{
			++str;
			v.resize_array(1);
			for(;;)
			{
				stringtov_internal(v[v.size()-1], str);
				while( stringcheck::isspace(*str) ) ++str;
				if( str[0]==',' )
				{	// next element
					v.resize_array(v.size()+1);
					++str;
				}
				else
				{
					if( str[0]=='}' )
					{	// array end
						++str;
					}
					// error otherwise,
					// just close the array here
					break;
				}
			}
		}
		else if( str[0]=='"' )
		{	// string literal
			string<> tmp;
			
			for(++str; *str&&*str!='"';++str)
			{	// unescape the string
				const char chr = unescape(str);
				if(chr)
					tmp << chr;
				else
					break;
			}
			if(*str=='"')
				++str;
			v = tmp;
		}
		else if( str[0]=='\'' )
		{	// char literal
			v = (int)str[1];
			str += 2+str[2]=='\'';
		}
		// normal numbers
		else if( str[0]=='0' && basics::locase(str[1]) == 'b' )
		{	// binary
			v= stringtoib(str+2,&str);
		}
		else if( str[0]=='0' && basics::locase(str[1]) == 'o' )
		{	// octal
			v= stringtoio(str+2,&str);
		}
		else if( str[0]=='0' && basics::locase(str[1]) == 'x' )
		{	// hex
			v= stringtoix(str+2,&str);
		}
		else if( stringcheck::isdigit(str[0]) )
		{	// decimal or float
			double f = stringtod(str,&str);
			// test if fits into a int64
			const int64 i = (int64)f;
			if( ((double)i)==f && ((f<0 && f>INT64_MIN) || (f>0 && f<INT64_MAX)) )
				v = i;
			else
				v = f;
		}
		// not recognized otherwise
		// just return empty
	}
}

///////////////////////////////////////////////////////////////////////////////
/// string to variant conversion.
/// combines stringtoib/stringtoio/stringtoix/stringtoi/stringtod
template <typename T>
basics::variant stringtov(T const*str, T const** run)
{	
	basics::variant ret;
	T const*s=str;
	stringtov_internal(ret, s);
	if(run) *run = s;
	return ret;
}
template variant stringtov<char>(char const* p, char const** run);
template variant stringtov<wchar_t>(wchar_t const* p, wchar_t const** run);

///////////////////////////////////////////////////////////////////////////////
/// variant to string conversion.
/// as reverse operation to stringtov, converts the complete variant,
/// adds quotes, escapes and array syntax
string<> tostring(const variant& v)
{
	string<> ret;
	if( v.is_array() )
	{
		ret << '{';
		int i;
		for(i=0; i<(int)v.size()-1; ++i)
			ret << tostring(v[i]) << ',';
		ret << tostring(v[i]);
		ret << '}';
	}
	else if( v.is_string() )
	{
		ret << '"';
		string<> str = v.get_string();
		const char* p = str.c_str();
		for(; *p; ++p)
		{
			const char x=escape(*p);
			if(x)
				ret << '\\' << x;
			else
				ret << *p;
		}
		ret << '"';
	}
	else if( v.is_valid() && !v.is_empty() )
	{
		string<> str = v.get_string();
		ret << str.c_str();
	}
	return ret;
}






///////////////////////////////////////////////////////////////////////////
// local conversions
void value_empty::convert2string()
{
	if( !this->is_string() )
		this->assign( this->get_string() );
}
void value_empty::convert2float()
{
	if( !this->is_float() )
		this->assign( this->get_float() );
}
void value_empty::convert2int()
{
	if( !this->is_int() )
	{
		this->assign( this->get_int() );
	}
}
void value_empty::convert2number()
{
	if( !this->is_int() )
	{	// only convert non-int's
		const double f = this->get_float();
		const int64 i = (int64)((f<0)?f-0.5:f+0.5);
		if( 1e-24>fabs(f-(double)i) && ((f<0 && f>INT64_MIN) || (f>0 && f<INT64_MAX)) )
		{
			this->assign( i );
		}
		else
		{
			if( !this->is_float() )
				this->assign( f );			
		}
	}
}
void value_empty::numeric_cast(const variant& v)
{	
	if( !this->is_int() )
	{
		const double f = this->get_float();
		const int64 i = (int64)((f<0)?f-0.5:f+0.5);
		if( 1e-24<fabs(f-(double)i) || ((f>=0 || f<=INT64_MIN) && (f<=0 || f>=INT64_MAX)) )
		{
			this->convert2float();
			return;
		}
	}
	if( !v.is_int() )
	{
		const double f = this->get_float();
		const int64 i = (int64)((f<0)?f-0.5:f+0.5);
		if( 1e-24<fabs(f-(double)i) || ((f>=0 || f<=INT64_MIN) && (f<=0 || f>=INT64_MAX)) )
		{
			this->convert2float();
			return;
		}
	}
	this->convert2int();
}



const value_empty& value_integer::operator+=(const variant& v)
{
	this->value += v.get_int();
	return *this;
}
const value_empty& value_integer::operator-=(const variant& v)
{
	this->value -= v.get_int();
	return *this;
}
const value_empty& value_integer::operator*=(const variant& v)
{
	this->value *= v.get_int();
	return *this;
}
const value_empty& value_integer::operator/=(const variant& v)
{
	const double d = v.get_float();
	if( 0!=d )
	{
		const double f = ((double)this->value) / d;
		const int64 i = (int64)f;
		if( f==(double)i && ((f<0 && f>INT64_MIN) || (f>0 && f<INT64_MAX)) )
			this->value = i;
		else
			this->value_empty::assign(f);
	}
	else
		value_invalid::convert(*this);
	return *this;
}
const value_empty& value_integer::operator%=(const variant& v)
{
	if( v.is_int() )
	{
		const int64 d = v.get_int();
		if( 0!=d )
			this->value %= d;
		else
			value_invalid::convert(*this);
	}
	else
	{
		const double d = v.get_float();
		if( 0!=d )
		{
			const double f = fmod(this->value, d);
			const int64 i = (int64)f;
			if( f==(double)i && ((f<0 && f>INT64_MIN) || (f>0 && f<INT64_MAX)) )
				this->value = i;
			else
				this->value_empty::assign( f );
		}
		else
			value_invalid::convert(*this);
	}
	return *this;
}
const value_empty& value_integer::operator &=(const variant& v)
{
	this->value &= v.get_int();
	return *this;
}
const value_empty& value_integer::operator |=(const variant& v)
{
	this->value |= v.get_int();
	return *this; }
const value_empty& value_integer::operator ^=(const variant& v)
{
	this->value ^= v.get_int();
	return *this; }
const value_empty& value_integer::operator>>=(const variant& v)
{
	this->value >>= v.get_int();
	return *this; }
const value_empty& value_integer::operator<<=(const variant& v)
{
	this->value <<= v.get_int();
	return *this;
}


const value_empty& value_float::operator+=(const variant& v)
{
	this->value += v.get_float();
	this->convert2number();
	return *this;
}
const value_empty& value_float::operator-=(const variant& v)
{
	this->value -= v.get_float();
	this->convert2number();
	return *this;
}
const value_empty& value_float::operator*=(const variant& v)
{
	const double f = this->value * v.get_float();
	const int64 i = (int64)f;
	if( ((double)i)==f && ((f<0 && f>INT64_MIN) || (f>0 && f<INT64_MAX)) )
		this->value_empty::assign( i );
	else
		this->value = f;
	return *this;
}
const value_empty& value_float::operator/=(const variant& v)
{
	const double d = v.get_float();
	if( 0!=d )
	{
		const double f = this->value / d;
		const int64 i = (int64)f;
		if( ((double)i)==f && ((f<0 && f>INT64_MIN) || (f>0 && f<INT64_MAX)) )
			this->value_empty::assign( i );
		else
			this->value = f;
	}
	else
		value_invalid::convert(*this);
	return *this;
}
const value_empty& value_float::operator%=(const variant& v)
{
	const double d = v.get_float();
	if( 0!=d )
	{
		const double f = fmod(this->value, d);
		const int64 i = (int64)f;
		if( ((double)i)==f && ((f<0 && f>INT64_MIN) || (f>0 && f<INT64_MAX)) )
			this->value_empty::assign( i );
		else
			this->value = f;
	}
	else
		value_invalid::convert(*this);
	return *this;
}
const value_empty& value_float::operator &=(const variant& v)
{
	this->assign( ((int64)this->value) & v.get_int());
	return *this;
}
const value_empty& value_float::operator |=(const variant& v)
{
	this->assign( ((int64)this->value) | v.get_int());
	return *this;
}
const value_empty& value_float::operator ^=(const variant& v)
{
	this->assign( ((int64)this->value) ^ v.get_int());
	return *this;
}
const value_empty& value_float::operator>>=(const variant& v)
{
	this->assign( ((int64)this->value) >> v.get_int());
	return *this;
}
const value_empty& value_float::operator<<=(const variant& v)
{
	this->assign( ((int64)this->value) << v.get_int());
	return *this;
}


const value_empty& value_string::operator+=(const variant& v)
{
	this->value += v.get_string();
	return *this;
}
const value_empty& value_string::operator&=(const variant& v)
{
	this->value += v.get_string();
	return *this;
}




value_array::value_array(const vector<variant>& v)
	: value_empty(), value(v)
{}
const value_array& value_array::convert(value_empty& convertee, const size_t sz, const variant& elem)
{
	convertee.~value_empty();
	value_array& arr = *new (&convertee) value_array();
	arr.value.resize(sz);
	if(sz)
	{	// have the old content at element 0 only 
		// arr.value[0] = elem;

		// expand the skalar to a vector
		vector<variant>::iterator iter(arr.value);
		for(; iter; ++iter)
		{
			*iter = elem;
		}
	}
	return arr;
}
const value_array& value_array::convert(value_empty& convertee, const vector<variant>& v)
{
	convertee.~value_empty();
	return *new (&convertee) value_array(v);
}
const value_array& value_array::construct(void* p, const vector<variant>& v)
{
	return *new (p) value_array(v);
}

bool value_array::resize_array(size_t cnt)
{
	if(cnt)
		this->value.resize(cnt);
	else
		value_empty::convert(*this);
	return true;
}
bool value_array::append_array(size_t cnt)
{	// add a dimension at the end
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
		iter->append_array(cnt);
	return true;
}



///////////////////////////////////////////////////////////////////////////
// local conversions
void value_array::convert2string()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		iter->convert2string();
	}
}
void value_array::convert2float()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		iter->convert2float();
	}
}
void value_array::convert2int()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		iter->convert2int();
	}
}
void value_array::convert2number()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		iter->convert2number();
	}
}
void value_array::numeric_cast(const variant& v)
{	
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		iter->numeric_cast(v);
	}
}

string<> value_array::get_arraystring() const
{
	string<> ret;
	ret << '{';
	vector<variant>::iterator iter(this->value);
	if(iter)
	{
		ret << iter->get_arraystring();
		for(++iter; iter; ++iter)
			ret << ',' << iter->get_arraystring();
	}
	ret << '}';
	return (ret);
}

void value_array::negate()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		if( iter->is_number() )
			iter->negate();
	}
}
void value_array::invert()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		if( iter->is_number() )
			iter->invert();
	}
}
void value_array::lognot()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		if( iter->is_number() )
			iter->lognot();
	}
}
const value_empty& value_array::operator++()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		++(*iter);
	}
	return *this;
}
const value_empty& value_array::operator--()
{
	vector<variant>::iterator iter(this->value);
	for(; iter; ++iter)
	{
		--(*iter);
	}
	return *this;
}


void value_array::operate(const variant& v, const variant& (variant::*func)(const variant&))
{
	if( v.is_array() )
	{	// two arrays
		value_array& a2 = (value_array&)v.access();
		vector<variant>::iterator iterV(a2.value);

		if( this->value.size() < a2.value.size() )
		{	
			this->value.resize(a2.value.size());
		}

		vector<variant>::iterator iterA(this->value);
		for(; iterA && iterV; ++iterA, ++iterV)
			((*iterA).*func)(*iterV);
		for(; iterA ; ++iterA)
			((*iterA).*func)( variant() );
	}
	else
	{
		vector<variant>::iterator iterA(this->value);
		for(; iterA; ++iterA)
		{
			if( iterA->is_valid() )
				((*iterA).*func)(v);
		}
	}
}



///////////////////////////////////////////////////////////////////////////
// default constructor
value_named::value_named()
	: value_extern(), value(new variant)
{}
///////////////////////////////////////////////////////////////////////////
// type constructors
value_named::value_named(const variant_host& parent, const string<>& n, const variant& v)
	: value_extern(parent.access()), name(n), value(new variant(v))
{}
value_named::value_named(const variant_host::reference& parent, const string<>& n, const variant& v)
	: value_extern(parent), name(n), value(new variant(v))
{}

value_named::~value_named()
{
	//printf("destroy %p value_named\n", static_cast<void*>(this));
	if(value) delete value;
}

///////////////////////////////////////////////////////////////////////////////
bool value_named::set_value()
{
	return this->parent_ref.exists() && *this->parent_ref && this->name.size() && (*this->parent_ref)->set_member(this->name, this->value->access());
}

///////////////////////////////////////////////////////////////////////////
//
void value_named::assign(const value_empty &v)		{ this->value->assign(v); this->set_value(); }
void value_named::assign(const sint64 v)			{ this->value->assign(v); this->set_value(); }
void value_named::assign(const double v)			{ this->value->assign(v); this->set_value(); }
void value_named::assign(const char* v)				{ this->value->assign(v); this->set_value(); }
void value_named::assign(const string<>& v)			{ this->value->assign(v); this->set_value(); }
void value_named::assign(const vector<variant>& v)	{ this->value->assign(v); this->set_value(); }
void value_named::assign(const variant_host& v)		{ this->value->assign(v); this->set_value(); }



///////////////////////////////////////////////////////////////////////////
//
const value_empty& value_named::duplicate(value_empty& convertee) const
{	// only duplicate the content
	if(this->parent_ref.exists() && *this->parent_ref)
		convertee = *value;
	else
		convertee.clear();
	return convertee;
}
const value_empty& value_named::duplicate(void* p) const
{
	return *new (p) value_named(this->parent_ref, this->value->access(), *this->value);
}
const value_named& value_named::convert(value_empty& convertee, const variant_host& parent, const string<>& n, const variant& v)
{
	convertee.~value_empty();
	return *new (&convertee) value_named(parent, n, v);
}
const value_named& value_named::convert(value_named& convertee, const string<>& n, const variant& v)
{
	variant_host::reference parent(convertee.parent_ref);
	convertee.~value_named();
	return *new (&convertee) value_named(parent, n, v);
}
const value_empty& value_named::construct(void* p, const variant_host& parent, const string<>& n)
{
	value_empty * tmp = new (p) value_empty();
	const_cast<variant_host&>(parent).get_member(n, *tmp);
	return *tmp;
}
///////////////////////////////////////////////////////////////////////////
//
void value_named::clear()				{ this->value->clear(); }
///////////////////////////////////////////////////////////////////////////
bool value_named::is_valid() const		{ return this->value->is_valid(); }
bool value_named::is_empty() const		{ return this->value->is_empty(); }
bool value_named::is_int() const		{ return this->value->is_int(); }
bool value_named::is_float() const		{ return this->value->is_float(); }
bool value_named::is_number() const		{ return this->value->is_number(); }
bool value_named::is_string() const		{ return this->value->is_string(); }
var_t value_named::type() const			{ return this->value->type(); }

///////////////////////////////////////////////////////////////////////////
// access conversion 
bool value_named::get_bool() const		{ return this->value->get_bool(); }
int64 value_named::get_int() const		{ return this->value->get_int(); }
double value_named::get_float() const	{ return this->value->get_float(); }
string<> value_named::get_string() const{ return this->value->get_string(); }

///////////////////////////////////////////////////////////////////////////
// unary operations
void value_named::negate()					{ this->value->negate(); this->set_value(); }
void value_named::invert()					{ this->value->invert(); this->set_value(); }
void value_named::lognot()					{ this->value->lognot(); this->set_value(); }

///////////////////////////////////////////////////////////////////////////
// prefix increment operator. (no postfix)
const value_empty& value_named::operator++()	{ this->value->operator++(); this->set_value(); return *this; }
const value_empty& value_named::operator--()	{ this->value->operator--(); this->set_value(); return *this; }
///////////////////////////////////////////////////////////////////////////
// arithmetic operations
const value_empty& value_named::operator+=(const variant& v)	{ this->value->operator+=(v); this->set_value(); return *this; }
const value_empty& value_named::operator-=(const variant& v)	{ this->value->operator-=(v); this->set_value(); return *this; }
const value_empty& value_named::operator*=(const variant& v)	{ this->value->operator*=(v); this->set_value(); return *this; }
const value_empty& value_named::operator/=(const variant& v)	{ this->value->operator/=(v); this->set_value(); return *this; }
const value_empty& value_named::operator%=(const variant& v)	{ this->value->operator%=(v); this->set_value(); return *this; }

///////////////////////////////////////////////////////////////////////////
// binary/logic operations
const value_empty& value_named::operator &=(const variant& v)	{ this->value->operator &=(v); this->set_value(); return *this; }
const value_empty& value_named::operator |=(const variant& v)	{ this->value->operator |=(v); this->set_value(); return *this; }
const value_empty& value_named::operator ^=(const variant& v)	{ this->value->operator ^=(v); this->set_value(); return *this; }
const value_empty& value_named::operator>>=(const variant& v)	{ this->value->operator>>=(v); this->set_value(); return *this; }
const value_empty& value_named::operator<<=(const variant& v)	{ this->value->operator<<=(v); this->set_value(); return *this; }







///////////////////////////////////////////////////////////////////////////////

void variant::cast(var_t type)
{	
	switch(type)
	{
	case VAR_NONE:
		if( this->is_valid() ) value_empty::convert(this->access());
		break;
	case VAR_INTEGER:
		this->access().convert2int();
		break;
	case VAR_FLOAT:
		this->access().convert2float();
		break;
	case VAR_STRING:
		this->access().convert2string();
		break;
	case VAR_ARRAY:
		if( !this->is_array() ) this->create_array(1);
		break;
	// nothing for VAR_AUTO/VAR_DEFAULT, they keep the type
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
// unary operations
const variant variant::operator-() const
{
	if( this->is_valid() )
	{
		variant temp(*this);
		temp.negate();
		return temp;
	}
	return *this;
}
const variant variant::operator~() const
{
	if( this->is_valid() )
	{
		variant temp(*this);
		temp.invert();
		return temp;
	}
	return *this;
}
const variant variant::operator!() const
{
	if( this->is_valid() )
	{
		variant temp(*this);
		temp.lognot();
		return temp;
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// Define postfix increment operator.
variant variant::operator++(int)
{
	variant temp(*this);
	if( this->is_valid() )
	{
		this->access().convert2number();
		++this->access();
	}
	return temp;
}
variant variant::operator--(int)
{
	variant temp(*this);
	if( this->is_valid() )
	{
		this->access().convert2number();
		--this->access();
	}
	return temp;
}
///////////////////////////////////////////////////////////////////////////
// Define prefix increment operator.
const variant& variant::operator++() 
{
	if( this->is_valid() )
	{
		this->access().convert2number();
		++this->access();
	}
	return *this;
}
const variant& variant::operator--() 
{
	if( this->is_valid() )
	{
		this->access().convert2number();
		--this->access();
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// arithmetic operations
const variant& variant::operator_assign(const variant& v)
{
	if( this->is_array() )
	{	// when array do hierarchical assignment
		this->access().operator_assign(v);
	}
	else
	{	// otherwise fall back to normal assignment
		this->assign(v);
	}
	return *this;
}

const variant& variant::operator+=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at least one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else if( this->is_string() || v.is_string() )
			this->convert2string();
		else if( this->is_float() || v.is_float() )
			this->convert2float();
		else
			this->convert2int();

		this->access() += v;
	}
	return *this;
}

const variant& variant::operator-=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->numeric_cast(v);

		this->access() -= v;
	}
	return *this;
}

const variant& variant::operator*=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->numeric_cast(v);

		this->access() *= v;
	}

	return *this;
}

const variant& variant::operator/=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->numeric_cast(v);

		this->access() /= v;
	}
	return *this;
}
const variant& variant::operator%=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->numeric_cast(v);

		this->access() %= v;
	}
	return *this;
}



///////////////////////////////////////////////////////////////////////////
// binary/logic operations
const variant& variant::operator&=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else if( this->is_string() || v.is_string() )
		{
			this->convert2string();
		}
		else
		{
			this->convert2int();
		}
		this->access() &= v;
	}
	return *this;
}

const variant& variant::operator|=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->convert2int();
		this->access() |= v;
	}
	return *this;
}

const variant& variant::operator^=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->convert2int();
		this->access() ^= v;
	}
	return *this;
}

const variant& variant::operator>>=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->convert2int();
		this->access() >>= v;
	}
	return *this;
}

const variant& variant::operator<<=(const variant& v)
{
	if( !this->is_valid() || !v.is_valid() )
	{	// invalid inputs, invalid output
		this->invalidate();
	}
	else 
	{
		if( this->is_array() || v.is_array() )
		{	// at leat one array
			if( !this->is_array() )
			{	// local skalar with given array
				this->create_array(v.size());
			}
		}
		else
			this->convert2int();
		this->access() <<= v;
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////
// compare operations
int compare(const variant& va, const variant& vb)
{
	if( va.value.pointer() == vb.value.pointer() )
	{	// identical pointers
		return 0;
	}
	else if( !va.is_valid() && !vb.is_valid() )
	{	// both invalid
		return 0;
	}
	else if( !va.is_valid() || !vb.is_valid() )
	{	// only one invalud
		return (!va.is_valid()) ? 1 : -1;
	}
	////////////////////////////
	// simple types at the end
	else if( va.is_array() && vb.is_array() )
	{	// two arrays
		int val = va.size() - vb.size();
		if(!val)
		{
			value_array& a1 = (value_array&)va.access();
			value_array& a2 = (value_array&)vb.access();
			vector<variant>::iterator iterA(a1.value);
			vector<variant>::iterator iterV(a2.value);
			int cmp=0;
			for(; !cmp && iterA && iterV; ++iterA, ++iterV)
				cmp = compare(*iterA, *iterV);
			return cmp;
		}
		return val;
	}
	else if( va.is_array() && !vb.is_array() )
	{	
		return 1;
	}
	else if( !va.is_array() && vb.is_array() )
	{	
		return -1;
	}
	else if( va.is_string() || vb.is_string() )
	{	
		return va.get_string().compareTo( vb.get_string() );
	}
	else if( va.is_float() || vb.is_float() )
	{
		const double v = va.get_float() - vb.get_float();
		return (v<0) ? -1 : (v>0) ? 1 : 0;
	}
	else if( va.is_int() || vb.is_int() )
	{
		return va.get_int() - vb.get_int();
	}
	return 0;
}




const char* variant::type2name(var_t type)
{
	static const char* names[] =
	{
		"none",
		"auto",
		"integer",
		"string",
		"float",
		"array"
	};
	return names[type];
}











struct ttt1 : public variant_host
{
	ulong val;
	virtual bool get_member(const string<>& name, value_empty& target)
	{
		if( name=="val" )
			return set_externmember(target, *this, &ttt1::val);
		return false;
	}
	virtual bool set_member(const string<>& name, const value_empty& target)
	{
		if( name=="val" )
			val = target.get_int();
		else
			return false;
		return true;
	}
} ttt1_i;

struct ttt2 : public variant_host
{
	string<> val;

	ttt2() : val("")
	{}

	virtual bool get_member(const string<>& name, value_empty& target)
	{
		if( name=="val" )
			return set_namedmember(target, *this, &ttt2::val, name);
		else if( name=="other" )
		{
			target = ttt1_i;
			return true;
		}
		return false;
	}
	virtual bool set_member(const string<>& name, const value_empty& target)
	{
		if( name=="val" )
			val = target.get_string();
		else
			return false;
		return true;
	}
} ttt2_i;




/// example for user definition by deriving a handler
namespace {
struct my_variant_defaults : public variant_defaults<bool>
{
	virtual ~my_variant_defaults()	{}
	virtual variant_host::reference get_variable(const string<>& name) const
	{
		if( name=="tti" )
			return ttt2_i.access();
		return variant_host::reference();
	}
} my_variant_defaults_instance;
}// end namespace (anonymous)
/*
/// example for user definition by function specialisation
template<>
variant_host::reference variant_defaults<bool>::get_variable(const string<>& name) const
{
	if( name=="tti" )
		return ttt2_i.access();
	return variant_host::reference();
}
*/

void test_variant()
{
#if defined(DEBUG)
	ttt2_i.val="";
	int prev_count = global::getcount();
	{
		ttt2& x = ttt2_i;
		variant a;
		a /= 0;
		a += 1;

		a =variant(ttt2_i, &ttt2::val);

		a = 123;
		a += 123;

		a = "hallo";
		a += 123;

		x.val = "";

		variant bb(ttt2_i, "val");

		bb = 100;
		bb = 100;
		bb = 100;
		
	}

	{
		int i;
		
		printf(tostring(stringtov<char>("\"a\\na\\x64\\123\"",NULL)));
		printf("\n");

		i = sizeof(value_empty);
		i = sizeof(value_integer);
		i = sizeof(value_float);
		i = sizeof(value_string);
		i = sizeof(value_array);
		i = sizeof(value_extern);
		i = sizeof(value_unnamed<int>);
		i = sizeof(value_named);
		i = sizeof(value_union);
		i = sizeof(variant);
		i = sizeof(string<>);
		i = sizeof(stringoperator<>);
		i = sizeof(stringinterface<>);
		i = sizeof(allocator<>);
		i = sizeof(vectorinterface<char>);
		i = sizeof(allocator_w_dy<char>);
		i = sizeof(vectorbase<char,allocator_w_dy<char> >);
		
		i = sizeof(TObjPtrCommon<char>);
		i = sizeof(TObjPtrCount<char>);
		i = sizeof(TObjPtr<char>);
		i = sizeof(variant);
		printf("%i\n", i);

	}

	{
		value_union aa("hallo");
		value_union bb(aa);

		printf(aa.access().get_string());printf("\n");
		printf(bb.access().get_string());printf("\n");
	}
	{


		variant tmp;
		tmp = "hallo";

		tmp.append_array(5);
		tmp[3] = 1;
		
	}

	{
		variant a,b,c;

		a = 1.8;
		b = "12hallo";
		c = 2.2;

		var_t vt = c.type();
		printf("%i\n",vt);
		c += a;
		vt = c.type();
		double xy = c();
		printf("%f\n",xy);

		a=10, b=3;

		xy = (a/b)();
		xy = (a%b)();
		a=10.6, b=2.4;
		xy = (a/b)();
		xy = (a%b)();

		a.cast(VAR_INTEGER);

		ttt2 xx;
		c = xx;
		c.access_member("val");
		c = 2.2;

		b.cast(VAR_INTEGER);
		variant d = (a+b)*c;
		double xd = d.get_float();

		variant tmp("hallo");

		xd=5;
		tmp.append_array((int)xd);
		tmp[3] = 1;
	}

	{
		int i; 
		double d;
		string<> s;
		const char* b;

		variant ext;

		ext = 1.2;


		ttt1& xxxx = ttt1_i;
		xxxx.val=0;

		{
			ttt2 xx;

			ext = xx;
			ext.access_member("val");
			ext = 1234;

		}
		ext = 0000;


		ext.clear();

		ext.access_member("tti");
//		ext.access_member("val");

		ext.access_member("other");
		ext.access_member("val");

		i = ext();
		ext = i;
		ext = 1;
		i = ext();
		ext = 2.5;
		i = ext();
		ext = "123hallo";
		i = ext();




		
		

		variant  a;

		a = 1;
		a = 4;



		a = 1;
		i = a();
		d = a();
		s = a.get_string();
		b = s;

		a = 1.3;
		i = a();
		d = a();
		s = a.get_string();
		b = s;

		a = "hallo";
		i = a();
		d = a();
		s = a.get_string();
		b = s;

		a.append_array(5);
		i = a();
		d = a();
		s = a.get_string();
		b = s;

		i = a[0]();
		d = a[0]();
		s = a.get_string();
		b = s;
		printf("%s\n",b);



		a[3] = "no3";
		a[2] = 1;

		a.append_array(3);
		i = a.size();
		i = a[3].size();
		i = a[3][1].size();

		a[3].get_value(i);
		a[3].get_value(d);
		a[3].get_value(s);
		b = s;
	}
	{
		/////////////////////////
		variant a,b,c;
	
		a = 8.6;

		a == b;
		a != b;
		a <  b;
		a <= b;
		a >  b;
		a >= b;

		a == 0;
		a != 0;
		a <  0;
		a <= 0;
		a >  0;
		a >= 0;

		0 == b;
		0 != b;
		0 <  b;
		0 <= b;
		0 >  b;
		0 >= b;



		variant ref(a,true);
		ref += 2;


		printf("%s (10.6)\n", (const char*)a.get_string());
		printf("%s (10.6)\n", (const char*)ref.get_string());

		b = "hallo";

		ref = a+b;

		printf("%s (10.6hallo)\n", (const char*)a.get_string());
		printf("%s (10.6hallo)\n", (const char*)ref.get_string());

		ref++;
		printf("%s (11.6)\n", (const char*)a.get_string());
		printf("%s (11.6)\n", (const char*)ref.get_string());

		b.resize_array(3);
		b[1] = 1;
		b[2] = 2.2;

		c = a;
		c = b;
		printf("%s (hallo)\n", (const char*)c[0].get_string());
		printf("%s (1)\n", (const char*)c[1].get_string());
		printf("%s (2.2)\n\n", (const char*)c[2].get_string());
		c = a+b;
		printf("%s (11.6hallo)\n", (const char*)c[0].get_string());
		printf("%s (12.6)\n", (const char*)c[1].get_string());
		printf("%s (13.8)\n\n", (const char*)c[2].get_string());
		b = c++;
		printf("%s (12.6)\n", (const char*)c[0].get_string());
		printf("%s (13.6)\n", (const char*)c[1].get_string());
		printf("%s (14.8)\n\n", (const char*)c[2].get_string());

		printf("%s (11.6hallo)\n", (const char*)b[0].get_string());
		printf("%s (12.6)\n", (const char*)b[1].get_string());
		printf("%s (13.8)\n\n", (const char*)b[2].get_string());

		c[1] += c[0];
		c[2] =  4;

		printf("%s (12.6)\n", (const char*)c[0].get_string());
		printf("%s (26.2)\n", (const char*)c[1].get_string());
		printf("%s (4)\n\n", (const char*)c[2].get_string());
		c += a;
		printf("%s (24.2)\n", (const char*)c[0].get_string());
		printf("%s (37.8)\n", (const char*)c[1].get_string());
		printf("%s (15.6)\n\n", (const char*)c[2].get_string());
		
		c[2] /= a;			printf("%s (1.344)\n", (const char*)c[2].get_string());
		c[2] &= a;			printf("%s (0)\n", (const char*)c[2].get_string());
		c[2] |= a;			printf("%s (12)\n", (const char*)c[2].get_string());
		c[2] ^= a;			printf("%s (0)\n", (const char*)c[2].get_string());
		c[2] = a && c[2];	printf("%s (0)\n", (const char*)c[2].get_string());
		c[2] = a || c[2];	printf("%s (1)\n\n", (const char*)c[2].get_string());

		printf("%s (24.2)\n", (const char*)c[0].get_string());
		printf("%s (37.8)\n", (const char*)c[1].get_string());
		printf("%s (1)\n\n", (const char*)c[2].get_string());
	}

	if( global::getcount() != prev_count )
		printf("missing destruction (%i %i)\n", (int)global::getcount(), (int)prev_count);
#endif//DEBUG
}



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

int test_variant()
{
	basics::test_variant();
	return 0;
}
