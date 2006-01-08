#include "basestring.h"
#include "basearray.h"
#include "baseministring.h"



/*
// supported string types
template class string<char>;
template class string<wchar_t>;
*/



///////////////////////////////////////////////////////////////////////////////
// string constant table
// until I find a way to embedd cahr/wchar string constant definitions 
// directly into the templates
///////////////////////////////////////////////////////////////////////////////
const char* StringConstant(size_t i, char dummy)
{
	static char* table[] =
	{
		"",
		"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
		"-9223372036854775808",
	};
	if( i<sizeof(table)/sizeof(table[0]) )
		return table[i];
	return table[0];
}
const wchar_t* StringConstant(size_t i, wchar_t dummy)
{
	static wchar_t* table[] =
	{
		L"",
		L"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
		L"-9223372036854775808"
	};
	if( i<sizeof(table)/sizeof(table[0]) )
		return table[i];
	return table[0];
}


///////////////////////////////////////////////////////////////////////////////
// some standard string functions
///////////////////////////////////////////////////////////////////////////////
template<class T> const T* toupper(T* str)
{
	if(str)
	{
		T *src=str;
		while(*src)
			*src++ = upcase(*src);
	}
	return str;
}
// explicit instanciation
template const char* toupper<char>(char* str);
template const wchar_t* toupper<wchar_t>(wchar_t* str);

///////////////////////////////////////////////////////////////////////////////
template<class T> const T* tolower(T* str)
{
	if(str)
	{
		T *src=str;
		while(*src)
			*src++ = locase(*src);
	}
	return str;
}
// explicit instanciation
template const char* tolower<char>(char* str);
template const wchar_t* tolower<wchar_t>(wchar_t* str);

///////////////////////////////////////////////////////////////////////////////
template<class T> const T* ltrim(T* str)
{
	if(str)
	{
		T *src=str, *tar=str, *mk=NULL;
		while(*src && _isspace(*src) )
			src++;
		while(*src)
		{
			mk = ( _isspace(*src) )?mk?mk:tar:NULL;
			*tar++ = *src++;
		}
		if(mk) 
			*mk=0;
		else
			*tar=0;
	}
	return str;
}
// explicit instanciation
template const char* ltrim<char>(char* str);
template const wchar_t* ltrim<wchar_t>(wchar_t* str);

///////////////////////////////////////////////////////////////////////////////
template<class T> const T* rtrim(T* str)
{
	if(str)
	{
		T *src=str, *tar=str, *mk=NULL;
		while(*src && _isspace(*src) )
			src++;
		while(*src)
		{
			mk = ( _isspace(*src) )?mk?mk:tar:NULL;
			*tar++ = *src++;
		}
		if(mk) 
			*mk=0;
		else
			*tar=0;
	}
	return str;
}
// explicit instanciation
template const char* rtrim<char>(char* str);
template const wchar_t* rtrim<wchar_t>(wchar_t* str);

///////////////////////////////////////////////////////////////////////////////
template<class T> const T* trim(T* str)
{
	if(str)
	{
		T *src=str, *tar=str, *mk=NULL;
		while(*src && _isspace(*src) )
			src++;
		while(*src)
		{
			mk = ( _isspace(*src) )?mk?mk:tar:NULL;
			*tar++ = *src++;
		}
		if(mk) 
			*mk=0;
		else
			*tar=0;
	}
	return str;
}
// explicit instanciation
template const char* trim<char>(char* str);
template const wchar_t* trim<wchar_t>(wchar_t* str);

///////////////////////////////////////////////////////////////////////////////
template<class T> const T* itrim(T* str, bool removeall)
{
	if(str)
	{
		T *src=str, *tar=str, mk=0;
		while(*src && _isspace(*src) )
			src++;
		while(*src)
		{
			if( _isspace(*src) )
				mk=' ', src++;
			else
			{
				if( mk && !removeall )
					*tar++=mk, mk=0;
				*tar++ = *src++;
			}
		}
		*tar=0;
	}
	return str;
}
// explicit instanciation
template const char* itrim<char>(char* str, bool removeall);
template const wchar_t* itrim<wchar_t>(wchar_t* str, bool removeall);



///////////////////////////////////////////////////////////////////////////////
// basic function for number2string conversion
// supported bases are 2..64
///////////////////////////////////////////////////////////////////////////////
template <class T> const T* _itobase(sint64 value, T* buf, size_t base, size_t& len, bool _signed)
{
    // internal conversion routine: converts the value to a string 
    // at the end of the buffer and returns a pointer to the first
    // character. this is to get rid of copying the string to the 
    // beginning of the buffer, since finally the string is supposed 
    // to be copied to a dynamic string in itostring(). the buffer 
    // must be at least 65 bytes long.

    static const T* digits = StringConstant(1,(T)0);
//  []=      "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		

    const T* pdigits = (base>36) ? digits : digits+2;
    size_t i = 64;
    buf[i] = 0;
    bool neg = false;
    uint64 v = value;
    if (_signed && base==10 && value < 0)
    {
        v = -value;
        // since we can't handle the lowest signed 64-bit value, we just
        // return a built-in string.
        if(sint64(v) < 0)   // the LLONG_MIN negated results in the same value
        {
            len = 20;
            return StringConstant(2,(T)0);
//				"-9223372036854775808";
        }
        neg = true;
    }
    do
    {
        buf[--i] = pdigits[uint(v % base)];
        v /= base;
    } while (v > 0);

    if (neg)
        buf[--i] = '-';

    len = 64 - i;
    return buf+i;
}
// explicit instanciation
template const char*    _itobase(sint64 value, char*    buf, size_t base, size_t& len, bool _signed);
template const wchar_t* _itobase(sint64 value, wchar_t* buf, size_t base, size_t& len, bool _signed);

///////////////////////////////////////////////////////////////////////////////
// function for number2string conversion
// prepares buffers and proper padding
///////////////////////////////////////////////////////////////////////////////
template <class T> void _itobase2(string<T>& result, sint64 value, size_t base, bool _signed, size_t width=0, T padchar=0)
{
    if(base < 2 || base > 64)
    {
        clear(result);
        return;
    }
    T buf[128];   // the longest possible string is 64 elems+eos when base=2
    size_t reslen;
    const T* p = _itobase<T>(value, buf, base, reslen, _signed);
    if (width > reslen)
    {
        if (padchar == 0)
        {	// default pad char
            if (base == 10)
                padchar = ' ';
            else if (base > 36)
                padchar = '.';
            else
                padchar = '0';
        }
		bool neg = *p == '-';
		result.clear();
		width -= reslen;
		if( neg && padchar!=' ' )
		{	// no space padding, need to strip the sign
			result.append('-');		// add the sign in front
			p++;					// buffer now starts after the sign
			reslen--;				// and is one element shorter
		}
		result.append(padchar, width);	// padding
		result.append(p, reslen);	// buffer
    }
    else 
        result.assign(p, reslen);
}
// explicit instanciation
template void _itobase2<char   >(string<char   >& result, sint64 value, size_t base, bool _signed, size_t width, char    padchar);
template void _itobase2<wchar_t>(string<wchar_t>& result, sint64 value, size_t base, bool _signed, size_t width, wchar_t padchar);


///////////////////////////////////////////////////////////////////////////////
// string to unsigned number conversion
// variable base
///////////////////////////////////////////////////////////////////////////////
template <class T> uint64 stringtoue(const T* str, size_t base)
{
	uint64 result = 0;

    if( str != 0 && *str != 0 && base >= 2 && base <= 64)
	{
		uint64 t;
	    const T* p = str;
		do
		{
			int c = *p++;
			if (c >= 'a')
			{
				// for the numeration bases that use '.', '/', digits and
				// uppercase letters the letter case is insignificant.
				if (base <= 38)
					c -= 'a' - '9' - 1;
				else  // others use both upper and lower case letters
					c -= ('a' - 'Z' - 1) + ('A' - '9' - 1);
			}
			else if (c > 'Z')
				break;
			else if (c >= 'A')
				c -= 'A' - '9' - 1;
			else if (c > '9')
				break;

			c -= (base > 36) ? '.' : '0';
			if (c < 0 || (size_t)c >= base)
				break;

			t = result * base;
			if (t / base != result)
				return INT64_MAX;
			result = t;
			t = result + uint(c);
			if (t < result)
				return INT64_MAX;
			result = t;
		} while (*p != 0);
	}
    return result;
}
// explicit instanciation
template uint64 stringtoue<char>(const char*  str, size_t base);
template uint64 stringtoue<wchar_t>(const wchar_t*  str, size_t base);


///////////////////////////////////////////////////////////////////////////////
// string to signed number conversion
// base 10
///////////////////////////////////////////////////////////////////////////////
template <class T> sint64 stringtoie(const T* str)
{
    if(str==0 || *str==0)
        return 0;
    bool neg = *str == '-';
    uint64 result = stringtoue(str + int(neg), 10);
    if (result > (uint64(INT64_MAX) + uint(neg)))
        return (neg)?INT64_MIN:INT64_MAX;
    if (neg)
        return -sint64(result);
    else
        return  sint64(result);
}
// explicit instanciation
template sint64 stringtoie<char>(const char* str);
template sint64 stringtoie<wchar_t>(const wchar_t* str);


///////////////////////////////////////////////////////////////////////////////
// string to signed number conversion
// base 10
///////////////////////////////////////////////////////////////////////////////
template <class T> sint64 stringtoi(const T* p)
{
	sint64 r = 0;
	bool sign=false;
	if( p != 0 &&  *p != 0)
	{
		sint64 t;
		while( _isspace(*p) ) p++;
		sign = (*p=='-');
		if(sign || *p=='+') p++;
		// allow whitespace between sign and number
		while( _isspace(*p) ) p++;
		do
		{
			char c = *p++;
			if (c < '0' || c > '9')	// invalid character
				break;
			t = r * 10;
			if (t < r)	// overflow
				return (sign)?INT64_MIN:INT64_MAX;
			t += c - '0';
			if (t < r)	// overflow
				return (sign)?INT64_MIN:INT64_MAX;
			r = t;
		} while (*p != 0);
	}
    return (sign)?(-r):(r);
}
// explicit instanciation
template sint64 stringtoi<char>(const char* p);
template sint64 stringtoi<wchar_t>(const wchar_t* p);




///////////////////////////////////////////////////////////////////////////////
// number to string conversions
// using string default type as return value
///////////////////////////////////////////////////////////////////////////////
string<> itostring(sint64 value, size_t base, size_t width, char padchar)
{
    string<> result;
    _itobase2(result, value, base, true, width, padchar);
    return result;
}
string<> itostring(uint64 value, size_t base, size_t width, char padchar)
{
    string<> result;
    _itobase2(result, value, base, false, width, padchar);
    return result;
}
string<> itostring(int value, size_t base, size_t width, char padchar)
{
    string<> result;
    _itobase2(result, sint64(value), base, true, width, padchar);
    return result;
}
string<> itostring(uint value, size_t base, size_t width, char padchar)
{
    string<> result;
    _itobase2(result, uint64(value), base, false, width, padchar);
    return result;
}




///////////////////////////////////////////////////////////////////////////////
// string interface
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// internal StringConstant aquire
///////////////////////////////////////////////////////////////////////////
template <class T> inline const T* stringinterface<T>::getStringConstant(size_t i) const
{
	return StringConstant(i, (T)0 );
}
// explicit instanciation
template const char*    stringinterface<char   >::getStringConstant(size_t i) const;
template const wchar_t* stringinterface<wchar_t>::getStringConstant(size_t i) const;

///////////////////////////////////////////////////////////////////////////
// provide an exception interface without including it in the h file (circle reference)
///////////////////////////////////////////////////////////////////////////
template <class T> void stringinterface<T>::throw_bound() const
{	// just use a fixed ansi string here
	throw exception_bound("String out of bound");
}
// explicit instanciation
//template const void stringinterface<char   >::throw_bound() const;
//template const void stringinterface<wchar_t>::throw_bound() const;


///////////////////////////////////////////////////////////////////////////////
// Search function
///////////////////////////////////////////////////////////////////////////////
template <class T> bool stringinterface<T>::findnext(const stringinterface<T>& pattern, size_t &startpos, bool ignorecase) const
{	// modified boyer-moore search
	
	// this table size is bound to 8bit values
	// so just treat other values like single steps
	size_t	SkipTable[256]; 

	size_t i,k,sp, inx;
	size_t len = pattern.length();

	// initialisation
	for (i=0; i<256; i++)
	{	// skip len+1 string chars if search char was not found,
		// not exactly boyer-moore but fastens up the thing
		SkipTable[i] = len+1;
	}
	for (i=0; i<len; i++)
	{	// otherwise skip as only that many 
		// so the next char in the string fits with the one frome the pattern
		inx = to_unsigned( pattern[i] );
		if( inx < (sizeof(SkipTable)/sizeof(SkipTable[0])) )
		SkipTable[ inx ] = len-i;
	}

	// run first to last / case sensitive
	sp=i=startpos;
	k=0; 
	while( i<this->length() && k<len )
	{
		if( ignorecase ? 
			( locase((*this)[i]) != locase(pattern[k]) ) :
			( (*this)[i] != pattern[k] ) )
		{	// no match at that position, find the next starting pos
			size_t inx = to_unsigned( (*this)[sp+len] );
			sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
			i=sp;
			k=0;
		}
		else
		{	// check the next char
			i++;
			k++;
		}
	}
	if( k<len ) // not found
		return false;
	startpos = sp;
	return true;
}
// explicit instanciation
template bool stringinterface<char   >::findnext(const stringinterface<char   >& pattern, size_t &startpos, bool ignorecase) const;
template bool stringinterface<wchar_t>::findnext(const stringinterface<wchar_t>& pattern, size_t &startpos, bool ignorecase) const;

///////////////////////////////////////////////////////////////////////////////
// Search function
///////////////////////////////////////////////////////////////////////////////
template <class T> TArrayDST<size_t> stringinterface<T>::findall(const stringinterface<T>& pattern, bool ignorecase) const
{	// modified boyer-moore search

	// store results in this list
	TArrayDST<size_t> poslist;

	// this table size is bound to 8bit values
	// so just treat other values like single steps
	size_t	SkipTable[256]; 

	size_t i,k,sp, inx;
	size_t len = pattern.length();

	// initialisation
	for (i=0; i<256; i++)
	{	// skip len+1 string chars if search char was not found,
		// not exactly boyer-moore but fastens up the thing
		SkipTable[i] = len+1;
	}
	for (i=0; i<len; i++)
	{	// otherwise skip as only that many 
		// so the next char in the string fits with the one frome the pattern
		inx = to_unsigned( pattern[i] );
		if( inx < (sizeof(SkipTable)/sizeof(SkipTable[0])) )
		SkipTable[ inx ] = len-i;
	}

	// run first to last / case sensitive
	sp=i=0;
	k=0; 
	while( i<this->length() )
	{
		if( ignorecase ? 
			( locase((*this)[i]) != locase(pattern[k]) ) :
			( (*this)[i] != pattern[k] ) )
		{	// no match at that position, find the next starting pos
			size_t inx = to_unsigned( (*this)[sp+len] );
			sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
			i=sp;
			k=0;
		}
		else
		{	// check the next char
			i++;
			k++;
		}

		if(k>=len)
		{	// found
			poslist.push(sp);
			sp++;
		}
	}
	return poslist;
}
// explicit instanciation
template TArrayDST<size_t> stringinterface<char   >::findall(const stringinterface<char   >& pattern, bool ignorecase) const;
template TArrayDST<size_t> stringinterface<wchar_t>::findall(const stringinterface<wchar_t>& pattern, bool ignorecase) const;






///////////////////////////////////////////////////////////////////////////////
// patternstring constructor
///////////////////////////////////////////////////////////////////////////////
template<class T> patternstring<T>::patternstring(const string<T>& pattern) : string<T>(pattern)
{
	size_t len = pattern.length();
	size_t i;
	// initialisation
	for (i=0; i<256; i++)
	{	// skip len+1 string chars if search char was not found
		// not exactly boyer-moore but fastens up the thing
		SkipTable[i] = len+1;
	}
	for (i=0; i<len; i++)
	{	// otherwise skip as only that many 
		// so the next char in the string fits with the one frome the pattern
		size_t inx = to_unsigned( pattern[i] );
		if( inx < (sizeof(SkipTable)/sizeof(SkipTable[0])) )
			SkipTable[ inx  ] = len-i;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Search function
///////////////////////////////////////////////////////////////////////////////
template<class T> bool patternstring<T>::findnext(const stringinterface<T>& searchstring, size_t &startpos, bool ignorecase) const
{	// modified boyer-moore search
	
	size_t i,k,sp;
	size_t len = this->length();

	sp=i=startpos;
	k=0; 
	while( i<searchstring.length() && k<len )
	{
		if( ignorecase ? 
			( locase(searchstring[i]) != locase((*this)[k]) ) :
			( searchstring[i] != (*this)[k] ) )
		{	// no match at that position, find the next starting pos
			size_t inx = to_unsigned( searchstring[sp+len] );
			sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
			i=sp;
			k=0;
		}
		else
		{	// check the next char
			i++;
			k++;
		}
	}
	if( k<len ) // not found
		return false;
	startpos = sp;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Search function
///////////////////////////////////////////////////////////////////////////////
template<class T> TArrayDST<size_t> patternstring<T>::findall(const stringinterface<T>& searchstring, bool ignorecase) const
{	// modified boyer-moore search

	// store results in this list
	TArrayDST<size_t> poslist;

	size_t i,k,sp;
	size_t len = this->length();

	sp=i=0;
	k=0; 
	while( i<searchstring.length() )
	{
		if( ignorecase ? 
			( locase(searchstring[i]) != locase((*this)[k]) ) :
			( searchstring[i] != (*this)[k] ) )
		{	// no match at that position, find the next starting pos
			size_t inx = to_unsigned( searchstring[sp+len] );
			sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
			i=sp;
			k=0;
		}
		else
		{	// check the next char
			i++;
			k++;
		}

		if(k>=len)
		{	// found
			poslist.push(sp);
			sp++;
		}
	}
	return poslist;
}

///////////////////////////////////////////////////////////////////////////////
// explicit instanciation of the whole class
template class patternstring<char>;
template class patternstring<wchar_t>;
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// default objects
// using default type
///////////////////////////////////////////////////////////////////////////////
string<> nullstring("");


















///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// checking usage of operator[]
class yyytest
{
	char array[16];
public:
	const char* c_str() const	{ return array;}
	yyytest()	{}
};

class xxxtest : public yyytest
{
	
public:
	xxxtest()	{}
	~xxxtest()	{}


	operator const char*()				{ printf("string\n"); return this->c_str(); }


	      char& operator[](int i)		{ printf("array\n");return ((char*)this->c_str())[i]; }
	const char& operator[](int i) const	{ printf("const array\n");return this->c_str()[i]; }
};



///////////////////////////////////////////////////////////////////////////////

int stringbuffer_test()
{

	{
		dbstring<>::get("hallo");
		dbstring<>::get("1111");

		string<> a = "  xxx  ";

		string<> b = dbstring<>::get(a);

		string<> c = dbstring<>::get("hallo");


		a = "  hallo  ";
		b = a;

		a.trim();
		b.ltrim();
		b.rtrim();

		staticstring<> xx("hallo");
		xx[2]='b';
	
	}

	{
		char buffer[128];
		staticstring<> a(buffer, sizeof(buffer));
		basestring<> b;

		a << "hallo" << 1.23;
		string<> tt = "  12345  12345   ";
		tt.trim();
	}


	{
		string<> a;
		const char *c;
		uint b, i;
	
		for(i=0; i<66; i++)
		{
			a = itostring(1234567890, i);
			c = a;
			b = stringtoue(a.c_str(), i);
			printf("base 10 '%u' / base %2i '%s'\n", b, i, c);
		}
		printf("\n");
	}




	{
		basestring<> a("halloballo");
		basestring<> b(a);
		basestring<> c("11111");
	
		patternstring<> pat("ll");

		size_t pos=0;
		pat.findnext(a, pos);
		TArrayDST<size_t> list = pat.findall(a);

		char buffer[1024];

		TString< char, allocator_w_dy<char, elaborator_st<char> > > aa;
		TString< char, allocator_w_st<char, elaborator_st<char> > > bb(buffer,1024);

		aa = bb;


		a.append("xxxxx");
		a.insert(5, c, 2);
		a.replace(2, 2, '.', 6);

	}

	{
		string<> a="aa11bb";
		a(2,2) = a;

		a.assign( a(2,4) );
	}

	{



		xxxtest xxx;

		char c = xxx[3];
		if( xxx[5]!='t' )
			xxx[1] = c;

MiniString a; bool bb = true;

a << "hallo" << 1 << bb;


		basestring<> sa, sb, sc;
		globalstring<> ga, gb, gc;
		string<> da, db, dc;

	//	const char c = da[3];
size_t sz=0;
		da = "abcde";
		da.findnext("cd",sz);
		db = "test";
		dc = da;

		ga = da;

		ga += "1111";
		da += "2222";

		sa = ga;

		gb = sa;

	}







	size_t i, sz;
	char buffer1[1024];

	char buffer2[1024];
	staticstring<> sa(buffer2, sizeof(buffer2));
	basestring<> sb;
	MiniString a;




	sa = "hallo";
	sb = "hallo";
	sa << "test" << 19999;

	sb = sa + 111111;

	ulong tick = clock();
	for(i=0; i<100000; i++)
		sz = snprintf(buffer1, sizeof(buffer1), "hallo %i ballo %i no %lf", i, i*2/3+i, ((double)i)*1.3);
	printf("sprintf %i\n", clock()-tick);

	tick = clock();
	for(i=0; i<100000; i++)
		a = MiniString("hallo ") + (int)i + " ballo " + (int)(i*2/3+i) + " no " +(((double)i)*1.3);
	printf("Ministring %i\n", clock()-tick);

	tick = clock();
	for(i=0; i<100000; i++)
	{
		sa.clear();
		sa << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("staticstring %i\n", clock()-tick);
	
	tick = clock();
	for(i=0; i<100000; i++)
	{
		sb.clear();
		sb << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("basestring %i\n", clock()-tick);

	string<> ds;
	tick = clock();
	for(i=0; i<100000; i++)
	{
		ds.clear();
		ds << "a";
		ds << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("string %i\n", clock()-tick);

	return 0;
}

















///////////////////////////////////////////////////////////////////////////////
// string test functions
// just redefine the types from here
#define string string<>
#define substring substring<>



// checkRefCntVal: pass by value, where reference count gets incremented
// by one on entry and decremented by one on exit.
 
void checkRefCntVal(string s, size_t shouldBe, const char *prefix )
{
  if (s.getRefCount() != shouldBe)
    printf("%s refCnt = %d (should be %d)\n", 
           prefix, s.getRefCount(), shouldBe );
} // checkRefCntVal



// checkRefCnt: pass by reference.  The reference count in the string
// object is unchanged, since the object is passed by reference.

void checkRefCnt(string &s, size_t shouldBe, const char *prefix )
{
  if (s.getRefCount() != shouldBe)
    printf("%s refCnt = %d (should be %d)\n", 
           prefix, s.getRefCount(), shouldBe );
} // checkRefCnt



// test_constructors
void test_constructors()
{
  string a( "abcd" );
  string b( a );

  printf("Test string constructors\n");

  if (b.getRefCount() != 2)
    printf("1. Reference count is wrong: refCnt = %d (should be 2)\n",
           b.getRefCount() );

  string c = a;
  if (b.getRefCount() != 3)
    printf("2. Reference count is wrong: refCnt = %d (should be 3)\n",
           b.getRefCount() );

  if (a.getRefCount() != 3)
    printf("3. Reference count is wrong: refCnt = %d (should be 3)\n",
           b.getRefCount() );

  checkRefCntVal( a, 4, "4. ");

  if (a.getRefCount() != 3)
    printf("4. Reference count is wrong: refCnt = %d (should be 3)\n",
           b.getRefCount() );

  checkRefCnt( a, 3, "5. ");

  string d( 'd' );

  // test for construction with an empty string
  string e;
  string f( e );
  checkRefCnt( f, 2, "6. ");
  checkRefCntVal( f, 3, "7. ");

  // test of a pointer containing null
  const char *nullPtr = 0;
  string g( nullPtr );

  // test of a null address
  string h( (const char *)0 );

  string i( '\0' );
  if (i.length() != 0) {
    printf("8. Adding a null character should not increase length\n");
  }
} // test_constructors



// test_char_cast
void test_char_cast()
{
  printf("Test character cast\n");

  const char *testStr = "the quick brown fox";
  string a( testStr );
  const char *tmp = a;

  if (tmp == 0)
    printf("1. error: result of cast is null\n");
  else if (strcmp(testStr, tmp) != 0)
    printf("2. error: strings are not equal\n");

} // test_char_cast



// test_assign
//
// The privious version of the string class created a new copy when
// ever the index operator ([]) was used.  I was surprised to 
// discover that this was the case even with the index operator
// was on the right hand size.  This function includes a test
// which checks that copy on read does not take place in this
// version.
void test_assign()
{
  printf("Test assignment\n");

  const char *testStr = "my girl is the best";
  string a = "abcd";

  const char *tmp = a;

  if (strcmp(tmp, "abcd") != 0)
    printf("1. Assignment in declaration failed\n");

  const char *init_b = "this is not it";
  string b(init_b);
  string original_b;

  original_b = b;

  b = testStr;
  if (b.getRefCount() != 1)
    printf("2. reference count for b is wrong\n");

  tmp = b;
  if (strcmp(tmp, testStr) != 0)
    printf("3. string has incorrect contents\n");

  if (original_b.getRefCount() != 1)
     printf("4. reference count for original_b is wrong\n");

  if (original_b != init_b)
     printf("5. modification of b improperly changed original_b\n");

  string c( testStr );
  c = b;
  if (b.getRefCount() != 2)
    printf("6. reference count is wrong\n");

  const char *nullPtr = 0;
  string d;

  if (d != "") {
    printf("7. comparision to a null string failed\n");
  }

  d = nullPtr;
  if (d != "") {
    printf("8. assignment of a null C-string failed\n");
  }

  d = testStr;
  tmp = d;
  if (strcmp(tmp, testStr) != 0)
    printf("9. string has incorrect contents\n");

  string e = string( testStr );
  tmp = e;
  if (strcmp(tmp, testStr) != 0)
    printf("10. string has incorrect contents\n");

  if (e.getRefCount() != 1)
    printf("11. refCnt is wrong: refCnt = %d, should be 1\n",
           e.getRefCount() );

  const char *constCStr = "1234567890";
  const size_t len = sizeof(constCStr) / sizeof(char);
  string foo = constCStr;
  string bar = foo;
  if (foo.getRefCount() != 2) {
    printf("12. refcnt is wrong: refCnt = %d, should be 2\n",
	   foo.getRefCount() );
  }

  // This makes sure that the [] operator is implemented properly
  // and does not cause a "copy-on-write" on a read operation.
  bool contentOK = true;
  for (size_t i = 0; i < len; i++) {
    if (constCStr[i] != foo[i]) {
      contentOK = false;
      break;
    }
  }
  if (!contentOK) {
    printf("13: content is wrong\n");
  }
  // make sure refCnt is still OK
  if (bar.getRefCount() != 2) {
    printf("14. refcnt is wrong: refCnt = %d, should be 2\n",
	   bar.getRefCount() );
  }

  const char *testStr2 = "null is a lonely number";
  string r = testStr2;
  string r2 = r;
  r = '\0';
  if (r != "") {
    printf("15. assignment of null character did not result in empty str\n");
  }

  if (r2 != testStr2) {
    printf("16. null character assignment changed a shared string\n");
  }

  if (r2.getRefCount() != 1) {
    printf("17. reference count is wrong\n");
  }

  const char *testStr3 = "\"Writing tests is hard!\" said Barbie";
  string s = testStr3;
  string s2 = s;
  s = "";
  if (s != "") {
    printf("18. assignment of empty string did not result in empty str\n");
  }

  if (s2 != testStr3) {
    printf("19. empty string assignment changed a shared string\n");
  }

  if (s2.getRefCount() != 1) {
    printf("17. reference count is wrong\n");
  }

  // Test chained assignment
  const char *testStr4 = "working on the chain gang";
  string w, x, y;
  w = x = y = testStr4;

  if (w != testStr4 ||
      x != testStr4 ||
      y != testStr4) {
    printf("18. chained assignment failed\n");
  }

  if (y.getRefCount() != 3) {
    printf("19. reference count in chained assignment is wrong\n");
  }

  string z = "still working on the gang";
  w = x = y = z;
  if((w != x) ||
     (x != y) ||
     (y != z)) {
    printf("20. chained assignment failed\n");
  }

  if (y.getRefCount() != 4) {
    printf("21. reference count in chained assignment is wrong\n");
  }
} // test_assign



// test_plus_equal
void test_plus_equal()
{
  const char *firstHalf = "abcd";
  const char *secondHalf = " efgh";
  const char *concatStr = "abcd efgh";

  printf("Test += operator\n");

  string a( firstHalf );

  a += secondHalf;

  const char *tmp = a;

  if (strcmp(tmp, concatStr) != 0)
    printf("1. Strings did not match: str = %s (should be [%s]\n",
           tmp, concatStr );

  string b;

  b += firstHalf;
  tmp = b;
  if (strcmp(tmp, firstHalf) != 0)
    printf("2. Strings did not match: str = %s (should be [%s]\n",
           tmp, firstHalf );

  string d, c;

  c += d;
  if (c.getRefCount() != 1)
    printf("3. refCnt should (still) be 1\n");

  if (d != "" || c != "") {
    printf("4. Strings c and d should be the empty string, but are not\n");
  }

  c += secondHalf;
  tmp = c;
  if (strcmp(tmp, secondHalf) != 0)
    printf("5. Strings did not match: str = %s (should be [%s]\n",
           tmp, secondHalf );

  string e("1234");

  for (size_t i = 5; i < 10; i++) {
    e += (char)(i + (char)'0');
  }
  if (e != "123456789") {
    tmp = e;
    printf("6. Character concat failed: d = %s, should be 123456789\n",
           tmp );
  }

  const char *testStr1 = "metal jacket";
  string empty;
  string full(testStr1);

  empty += full;

  if (empty.getRefCount() != 1) {
    printf("7. empty.getRefCount() = %d, should be 1\n", empty.getRefCount() );
  }

  if (empty != testStr1) {
    printf("8. empty string += string failed\n");
  }

  const char *testStr2 = "foo";
  string empty2;
  const char *str = testStr2;

  empty2 += str;
  if (empty2.getRefCount() != 1) {
    printf("9. empty2.getRefCount() = %d, should be 1\n", empty2.getRefCount() );
  }

  if (empty2 != testStr2) {
    printf("10. empty string += C-string failed\n");
  }

  // test chained assignment
  const char *testStr3 = "twas brillig";
  string s1 = "twas ";
  string s2 = "brillig";
  string s3 = s1 += s2;

  if (s3 != s1 && s1 != testStr3) {
    printf("11. chained assignment with += failed\n");
  }

  if (s3.getRefCount() != 2 &&
      s1.getRefCount() != 2 &&
      s2.getRefCount() != 1) {
    printf("12. reference count for chained assignment with += failed\n");
  }
} // test_plus_equal




// test_plus
void test_plus()
{
  printf("Test + operator\n");

  const char *firstHalf = "abcd";
  const char *secondHalf = " efgh";
  const char *concatStr = "abcd efgh";


  //
  // Test string + string
  //
  string t1( firstHalf );
  string t2( secondHalf );
  string a = t1 + t2;
  if (a.getRefCount() != 1) {
    printf("1. refCnt is wrong: refCnt = %d, should be 1\n",
           a.getRefCount() );
  }

  if (strcmp((const char *)a, concatStr) != 0)
    printf("2. string contents are not correct: a = %s (should be [%s])\n",
           (const char *)a, concatStr );

  //
  // Test string + const char *
  //
  string b = t1 + secondHalf;

  if (b.getRefCount() != 1) {
    printf("3. refCnt is wrong: refCnt = %d, should be 1\n",
           b.getRefCount() );
  }

  const char *tmp = b;
  if (strcmp(tmp, concatStr) != 0)
    printf("4. string contents are not correct: b = %s (should be [%s])\n",
           tmp, concatStr );

  //
  // test the global string addition operator const char * + string
  //
  string c = firstHalf + t2;
  tmp = c;
  if (strcmp(tmp, concatStr) != 0)
    printf("5. string contents are not correct: c = %s (should be [%s])\n",
           tmp, concatStr );

  //
  // Make sure that the operands of the addition are not altered by
  // the addition
  //
  string first( firstHalf );
  string second( secondHalf );
  string d = first + second;
  tmp = first;
  if (strcmp(tmp, firstHalf) != 0)
    printf("6. first has been altered: first = %s (should be [%s])\n",
           tmp, firstHalf );

  tmp = second;
  if (strcmp(tmp, secondHalf) != 0)
    printf("7. second has been altered: second = %s (should be [%s])\n",
           tmp, secondHalf );

  tmp = d;
  if (strcmp(tmp, concatStr) != 0)
    printf("8. string contents are not correct: d = %s (should be [%s])\n",
           tmp, concatStr );

  //
  // Test character concatenation.  Here the character operands
  // are converted to string objects.
  //
  string e("12345");
  string f;
  string g;

  f = e + '6' + '7' + '8' + '9';
  g = 'a' + f;

  if (e != "12345")
    printf("9. string e changed\n");

  if (f != "123456789") {
     const char *tmp = f;
     printf("10. string f is %s, it should be \"123456789\"\n", tmp);
  }

  if (g != "a123456789")
    printf("11. g is incorrect\n");

  // Test addition with chained assignment and string operands
  string w( "foo" );
  string x( "bar" );
  string y;
  string z = y = w + x;

  if (y != "foobar") {
     const char *tmp = y;
     printf("12. string y is %s, it should be \"foobar\"\n", tmp );
  }

  if (z != "foobar") {
     const char *tmp = z;
     printf("13. string z is %s, it should be \"foobar\"\n", tmp );
  }

  // The reference counts for w and x should both be 1
  if (w.getRefCount() != 1) {
     printf("14. w.getRefCount() = %d, it should be 1\n", w.getRefCount() );
  }
  if (x.getRefCount() != 1) {
     printf("15. x.getRefCount() = %d, it should be 1\n", x.getRefCount() );
  }

  if (y.getRefCount() != 2) {
     printf("16. y.getRefCount() = %d, it should be 2\n", y.getRefCount() );
  }
  if (z.getRefCount() != 2) {
     printf("17. z.getRefCount() = %d, it should be 2\n", z.getRefCount() );
  }
} // test_plus




// test_relops
//
// Test relational operators

void test_relops()
{
  printf("Test relational operators\n");

  const char *less = "abcd";
  const char *greater = "wxyz";
  const char *equal = "abcd";
  string lessString( less );
  string greaterString( greater );
  string equalString( equal );  // note that equalString == lessString == less
  string same;

  same = less;

  //
  // ==
  //
  // string string
  //
  // Check the case where both strings contain no data
  // (and so are equal).
  string x, y;  // two empty strings
  if (x != y)
    printf("0. empty strings are not equal\n");

  if (x != "") {
    printf("0.5: string not equal to empty C string\n");
  }

  if (! (lessString == equalString))
    printf("1. string == string failed\n");

  // string const char *
  if (! (lessString == equal))
    printf("2. string == const char * failed\n");

  // const char *  string
  if (! (equal == lessString))
    printf("3. const char * == string failed\n");

  if (! (same == less))
    printf("string == string failed for string objs w/same shared data\n");

  //
  // !=
  //
  // string string
  if (! (lessString != greaterString))
    printf("4. string != string failed\n");

  // string const char *
  if (! (lessString != greater))
    printf("5. string != const char * failed\n");

  // const char *  string
  if (! (less != greaterString))
    printf("6. const char * != string failed\n");

  //
  // >=
  //
  // string string
  if (! (greaterString >= lessString))
    printf("7. string >= string failed for >\n");

  if (! (lessString >= equalString))
    printf("8. string >= string failed for ==\n");

  // string const char *
  if (! (greaterString >= less))
    printf("9. string >= const char * failed for >\n");

  if (! (lessString >= equal))
    printf("10. string >= const char * failed ==\n");

  // const char *  string
  if (! (greater >= lessString))
    printf("11. const char * >= string failed for >\n");

  if (! (equal >= lessString))
    printf("12. const char * >= string failed for ==\n");

  //
  // <=
  //
  // string string
  if (! (lessString <= greaterString))
    printf("13. string <= string failed for <\n");

  if (! (lessString <= equalString))
    printf("14. string <= string failed for ==\n");

  // string const char *
  if (! (lessString <= greater))
    printf("15. string <= const char * failed for <\n");

  if (! (lessString <= equal))
    printf("16. string <= const char * failed for ==\n");

  // const char *  string
  if (! (less <= greaterString))
    printf("17. const char * <= string failed for <\n");

  if (! (equal <= lessString))
    printf("18. const char * <= string failed for ==\n");

  //
  // >
  //
  // string string
  if (! (greaterString > lessString))
    printf("19. string > string failed\n");

  // string const char *
  if (! (greaterString > less))
    printf("20. string > const char * failed\n");

  // const char *  string
  if (! (greater > lessString))
    printf("21. const char * > string failed\n");

  //
  // <
  //
  // string string
  if (! (lessString < greaterString))
    printf("22. string < string failed\n");

  // string const char *
  if (! (lessString < greater))
    printf("23. string < const char * failed\n");

  // const char *  string
  if (! (less < greaterString))
    printf("24. const char * < string failed\n");

} // test_relops




// Array reference [] operator tests

// There are two versions: left hand size and right
// hand size array operators.  The right hand side
// version returns a value, the left hand size
// operator returns an address (to which a value
// can be assigned).

void test_arrayop()
{
  const char *jabbar     = "He took his vorpal sword in hand";
  const char *newJabbar1 = "He took her vorpal sword in hand";
  //     index 9   -----------------^
  const char *newJabbar2 = "He took her vorpal ruler in hand";
  //     index 19  ----------------------------^
  const char *newWord = "ruler";

  const size_t len = strlen( jabbar );
  string jabbarString( jabbar );
  string jabbarRefStr = jabbarString;


  printf("test arrayops\n");

  //
  // Make sure that integer operators on the RHS work properly
  //
  if (jabbarString[3] != 't') {
    printf("0. string index failed\n");
  }

  // make sure than in index operation does not cause a copy
  if (jabbarRefStr.getRefCount() != 2) {
    printf("0.5. string index seems to have caused a copy\n");
  }

  for (size_t i = 0; i < len; i++) {
    char lhsCh = jabbarString[i];
    char rhsCh = jabbar[i];
    if (lhsCh != rhsCh) {
      printf("1. mismatch on rhs string index\n");
    }
  }

  // references are jabbarString, jabbarRefStr and now, "a"
  string a = jabbarString;
  if (a.getRefCount() != 3)
    printf("2. reference count is wrong\n");

  a(9) = 'e';
  a(10) = 'r';

  // The string has been changed, so a "copy on write" should 
  // have taken place.  Now there is a single copy, with the
  // change.
  if (a.getRefCount() != 1)
    printf("3. 'a' reference count is wrong\n");

  if (jabbarString.getRefCount() != 2)
    printf("4. jabbarString reference count is wrong\n");
  
  const char *tmp = a;
  if (strcmp(tmp, newJabbar1) != 0) {
    printf("5. strings don't match: a = %s, should be %s\n",
           tmp, newJabbar1 );
  }

  // make sure that the original string is unchanged
  tmp = jabbarString;
  if (strcmp(tmp, jabbar) != 0) {
    printf("6. strings don't match: a = %s, should be %s\n",
           tmp, jabbar );
  }

  // make sure that the original string is unchanged
  a = newJabbar1;

  a(19,5) = newWord;
  tmp = a;
  if (strcmp(tmp, newJabbar2) != 0) {
    printf("7. strings don't match: a = %s, should be %s\n",
           tmp, newJabbar2 );
  }


} // test_arrayop





// test_insert()
//
// In an earlier version of the string object used an explicit insert
// function to insert a string into another string.  This is very much
// like assigning to a SubString.  In the case of an insert the
// length of the section being replaced is 0.
   
// This code attempts to not only make sure that insert works, but
// also that insert works for various corner cases (e.g., insert of an
// empty string or a null C string).

void test_insert()
{
  printf("Test string insert\n");

  const char *origA = "abcdefgh";
  const char *rslt1 = "abcd1234efgh";
  string a(origA);
  string b("1234");
  string c = a;

  // test insert of a string into a string

  // reference count should be 2, since a and c have the
  // same shared data
  if (a.getRefCount() != 2)
    printf("1. reference count is wrong\n");

  a(4,0) = b;
  // make sure a is "abcd1234efgh"
  if (a != rslt1) {
    printf("2. insert failed. a = [%s], should be [%s]\n",
           (const char *)a, rslt1 );
  }

  // a should be unique
  if (a.getRefCount() != 1)
    printf("3. reference count in a is wrong\n");

  // c should be unique
  if (c.getRefCount() != 1)
    printf("4. reference count in c is wrong\n");

  // The contents of c should be unchanged
  if (c != origA)
    printf("5. contents of c is wrong\n");

  // test insert of C-string into a string
  string d = c;
  d(4,0) = "1234";
  // make sure d is "abcd1234efgh"
  if (d != rslt1) {
    printf("6. insert failed. d = [%s], should be [%s]\n",
           (const char *)d, rslt1 );
  }  

  string c_prime = c;  // reference count is 2

  // test insert of a zero length C string into a string
  // (remember, c is still "abcdefgh")
  c(4, 0) = "";
  // make sure c is "abcdefgh" (e.g., nothing happened)
  if (c != origA) {
    printf("7. insert failed. c = [%s], should be [%s]\n",
           (const char *)c, origA );
  }

  // Insert a null string into a string object (should do nothing)
  c_prime(4, 0) = (const char *)0;
  // make sure that reference count is still 2
  if (c.getRefCount() != 2)
    printf("8. c.getRefCount() = %d, should be 2\n", c.getRefCount());

  if (c_prime.getRefCount() != 2)
    printf("9. c_prime.getRefCount() = %d, should be 2\n", c_prime.getRefCount());

  if (c_prime != origA) {
    printf("10. insert failed. c_prime = [%s], should be [%s]\n",
           (const char *)c_prime, origA );
  }

  // test insert of an empty string into "c", an non-empty string.
  string emptyString;

  // try insert at index 0
  c(0,0) = emptyString;
  // make sure c is "abcdefgh" (e.g., nothing happened)
  if (c != origA) {
    printf("11. insert failed. c = [%s], should be [%s]\n",
           (const char *)c, origA );
  }

  // try insert at index 4
  c(4,0) = emptyString;
  // make sure c is "abcdefgh" (e.g., nothing happened)
  if (c != origA) {
    printf("12. insert failed. c = [%s], should be [%s]\n",
           (const char *)c, origA );
  }
} // test_insert


//
void test_substr_func_valarg( const substring &sub, size_t errorNum )
{
  if (sub.getRefCount() != 1) {
    printf("%d. sub.getRefCount() = %d, should be 1\n", 
           errorNum, sub.getRefCount() );
  }

  if (sub != "de") {
    printf("%d. sub.string = %s, should be \"de\"\n",
           errorNum + 1, (const char *)((const string)sub) );
  }
} // test_substr_func_valarg



// test_substring
//
// Test operations on SubStrings via the string() operator.
// These test overlap the SubString class tests.  They
// are here for historical reasons.

void test_substring()
{
  printf("Test sub-string operations\n");
  const char *init_a = "abcdefgh";
  string a(init_a);

  if (a(3, 4).getRefCount() != 1) {
    printf("1. reference count is wrong\n");
  }

  test_substr_func_valarg( a(3, 2), 2 );

  string b = a;

  // b is "abcdefgh
  // insert "1234" at position 3
  b(3, 4) = "1234";

  if (a != init_a) {
    printf("3. \"a\" was altered when b(3, 4) was changed\n");
  }

  if (b != "abc1234h") {
    printf("4. b = %s, should be \"abc1234h\"\n", (const char *)b);
  }

  if (a.getRefCount() != 1 || b.getRefCount() != 1) {
    printf("5. a.getRefCount() = %d, b.getRefCount() = %d (both should be 1)\n",
           a.getRefCount(), b.getRefCount() );
  }

  string c;
  c = b(3, 4);
  if (c != "1234") {
    printf("6. c = %s, should be \"1234\"\n", (const char *)c);
  }

  if (c.length() != 4) {
    printf("7. c.length() = %d, should be 4\n", c.length());
  }

  string d("1234abcdefgh");
  string e;

  e = d(4, 8) + d(0, 4); // e = d << 4
  if (e != "abcdefgh1234") {
    printf("8. e = %s, should be \"abcdefgh1234\"\n", (const char *)e );
  }

  if (d != "1234abcdefgh") {
    printf("9. d was changed by the SubString operations\n");
  }

  if (d.getRefCount() != 1) {
    printf("10. d.getRefCount() = %d, it should be 1\n", d.getRefCount());
  }

  //
  // According to 10.4.10 Temporary Objects in "The C++ Programming
  // Language", Third Edition, by Stroustrup:
  //
  //   Unless bound to a reference or used to initialize a named
  //   object, a temporary object is destroyed at the end of teh full
  //   expression in which it was created.  A full expression is an
  //   expression that is not a subexpression of some other
  //   expression.
  //
  // Stroustrup goes on to provide an example using the Standard
  // Template Library (STL) "string" class.  Here the c_str()
  // function returns a "C" language string.
  //
  //    const char* cs = (s1 + s2).c_str().
  //
  //    A temporary object of class string is created to hold s1+s2.
  //    Next, a pointer to a C-style string is extracted from the
  //    object.  Then - at the end of the expression - the temporary
  //    object is deleted.  Now, where was the C-style string
  //    allocated?  Probably as part of the temporary object holding
  //    s1+s2, and that storage is not guaranteed to exist after that
  //    temporary is destroyted.  Consequently, cs points to
  //    deallocated storage.
  //
  // In the case of the string container, the expression
  //
  //     e = d(4, 8) + d(0, 4);
  // 
  // creates a string object temporary, which is assigned to "e"
  //
  //   <allocate string temporary object CompilerTemp>
  //   <CompilerTemp> = d(4, 8) + d(0, 4);
  //   e = <CompilerTemp>     Note: at this point getRefCount = 2
  //   <destructor called for CompilerTemp> Note: refCnt decremented
  //   next statement
  //
  // By the time we reach "next statement" the destructor is called
  // and the reference counter for "e" is 1, which is what we would
  // expect.
  //
  // If this example does not convience you that C++ is a complicated
  // language, nothing will.
  //
  if (e.getRefCount() != 1) {
    printf("11. e.getRefCount() = %d, it should be 1\n", e.getRefCount());
  }

  //
  // Note that the SubString object created by the () operator is
  // a new object and so has a reference count of 1, although the
  // string associated with it has a reference count of two.
  string z = "lost Z-man";
  string y = z; // refCnt is now 2
  if (z(5, 5).getRefCount() != 1) {
    printf("12. z(8, 4).getRefCount() = %d, should be 1, !!changed behaviour here!!\n", z(8, 4).getRefCount() );
  }

  string f("chopsock");

  f(4, 4) = f(0, 4);
  
  if (f != "chopchop") {
    printf("13. f = %s, should be \"chopchop\"\n", (const char *)f );
  }
} // test_substring



//
// test_resize
//
// Test the string object resize function.

void test_resize()
{
  const char *init_a = "01234567890123456789";
  string a( init_a );
  string b = a;
  const char *tmp;

  printf("Test resize\n");

  b.truncate(10);  // set size of string b to 10
  if (b.length() != 10)
    printf("1. b.length() = %d, should be 10\n", b.length() );

  if (b != "0123456789") {
    tmp = b;
    printf("2. b = %s, should be \"0123456789\"\n", tmp );
  }

  if (a != init_a)
    printf("3. a was improperly modified by resizing b\n");

  if (a.length() != 20)
    printf("4. a.length() = %d, should be 20\n", a.length() );

  b.truncate(20);
  if (b != "0123456789          ") {
    tmp = b;
    printf("5. b = %s, should be \"0123456789          \"\n", tmp );
  }
  if (b.length() != 20)
    printf("6. b.length() = %d, should be 20\n", b.length() );

  if (a != init_a)
    printf("8. resizing b modified a\n");

  b.truncate( 0 );

  string empty;

  if (b != empty)
    printf("9. b is not the same as the empty string\n");

  if (a != init_a)
    printf("10. resizing b modified a\n");

  if (b.length() != 0)
    printf("11. b.length() = %d, should be 0\n", b.length() );

  if (b != "") {
    printf("12. b should be the same as the empty string\n");
  }

} // test_resize


int stringtest()
{
	test_constructors();
	test_char_cast();
	test_assign();
	test_plus_equal();
	test_plus();
	test_relops();
	test_arrayop();
	test_insert();
	test_substring();
	test_resize();
	return 0;
}

