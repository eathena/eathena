#ifndef __BASECHARSET_H__
#define __BASECHARSET_H__

#include "basetypes.h"

NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////
/// test case
void test_charset();


///////////////////////////////////////////////////////////////////////////////
/// 4bit number <-> hex character
///////////////////////////////////////////////////////////////////////////////
template <typename T> uchar char2hex(T c);
template <typename T> uchar str2hex(const T*& p) ;
template <typename T> T hex2char(uchar c);


///////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////
const int _csetbits = 256;
const int _csetbytes = _csetbits / 8;
const int _csetwords = _csetbytes / sizeof(ulong);
const char _csetesc = '~';	// escape character in charset strings

///////////////////////////////////////////////////////////////////////////////
// predeclaration
///////////////////////////////////////////////////////////////////////////////
template <typename T> class string;


///////////////////////////////////////////////////////////////////////////////
/// ansi character set.
/// derived from PTypes (C++ Portable Types Library)
/// contains sets of characters with various math/compare operations
/// sets can be defined by strings containing the characters itself
/// or escaped ansi hex codes of the chars, ranges can be given with '-'
///
/// ie: "abcdefg" == "a-g" =="~61-~67"
///
/// locase/upcase has to be handled manually
///////////////////////////////////////////////////////////////////////////////
class charset 
{
protected:
	union
	{
		uchar	bytedata[_csetbytes];
		ulong	worddata[_csetwords];
	};
public:
	charset()									{ clear(); }
	charset(const charset& s)					{ assign(s); }
	explicit charset(const char* setinit)		{ assign(setinit); }

	void assign(const charset& s)				{ memcpy(bytedata, s.bytedata, _csetbytes); }
	void assign(const char* setinit);
	void clear()								{ memset(bytedata, 0, _csetbytes); }
	void fill()									{ memset(bytedata, -1, _csetbytes); }
	void include(char b)						{ bytedata[uchar(b)>>3] |= uchar(1 << (uchar(b)&0x07 )); }
	void include(const char *c)					{ if(c) while(*c) this->include(*c++); }
	void include(char min, char max);
	void exclude(char b)						{ bytedata[uchar(b)>>3] &= uchar(~(1 << (uchar(b)&0x07))); }
	void exclude(char min, char max);
	void exclude(const char *c)					{ if(c) while(*c) this->exclude(*c++); }
	void unite(const charset& s);
	void subtract(const charset& s);
	void intersect(const charset& s);
	void invert();
	bool contains(char b) const					{ return (bytedata[uchar(b)>>3] & (1 << (uchar(b)&0x07))) != 0; }
	bool contains(const char* c) const			{ if(c) while(*c) if( this->contains(*c++)) return true; return false; }
	bool containsall(const char* c) const		{ if(c) while(*c) if(!this->contains(*c++)) return false; return true; }
	bool eq(const charset& s) const				{ return memcmp(bytedata, s.bytedata, _csetbytes) == 0; }
	bool le(const charset& s) const;

	charset& operator= (const charset& s)		{ assign(s); return *this; }
	charset& operator= (const char* c)			{ assign(c); return *this; }
	charset& operator+= (const charset& s)		{ unite(s); return *this; }
	charset& operator+= (char b)				{ include(b); return *this; }
	charset& operator+= (const char* c)			{ include(c); return *this; }
	charset operator+ (const charset& s) const	{ charset t = *this; return t += s; }
	charset operator+ (char b) const			{ charset t = *this; return t += b; }
	charset operator+ (const char* c) const		{ charset t = *this; return t += c; }
	charset& operator-= (const charset& s)		{ subtract(s); return *this; }
	charset& operator-= (char b)				{ exclude(b); return *this; }
	charset& operator-= (const char* c)			{ exclude(c); return *this; }
	charset operator- (const charset& s) const	{ charset t = *this; return t -= s; }
	charset operator- (char b) const			{ charset t = *this; return t -= b; }
	charset operator- (const char* c) const		{ charset t = *this; return t -= c; }
	charset& operator*= (const charset& s)		{ intersect(s); return *this; }
	charset operator* (const charset& s) const	{ charset t = *this; return t*=s; }
	charset operator! () const					{ charset t = *this; t.invert(); return t; }

	bool operator== (const charset& s) const	{ return eq(s); }
	bool operator!= (const charset& s) const	{ return !eq(s); }
	bool operator<= (const charset& s) const	{ return le(s); }
	bool operator<  (const charset& s) const	{ return !s.le(*this); }
	bool operator>= (const charset& s) const	{ return s.le(*this); }
	bool operator>  (const charset& s) const	{ return !le(s); }

	bool operator== (const char* s) const		{ return  containsall(s); }
	bool operator!= (const char* s) const		{ return !containsall(s); }

	friend charset operator+ (char b, const charset& s)			{ return s + b; }
	friend charset operator+ (const char* c, const charset& s)	{ return s + c; }
	friend charset operator- (char b, const charset& s)			{ return s - b; }
	friend charset operator- (const char* c, const charset& s)	{ return s - c; }
	friend bool operator& (char b, const charset& s)			{ return s.contains(b); }
	friend bool operator& (const char* c, const charset& s)		{ return s.contains(c); }
	friend void assign(charset& s, const char* setinit)			{ s.assign(setinit); }
	friend void clear(charset& s)								{ s.clear(); }
	friend void fill(charset& s)								{ s.fill(); }
	friend void include(charset& s, char b)						{ s.include(b); }
	friend void include(charset& s, char min, char max)			{ s.include(min, max); }
	friend void include(charset& s, const char* c)				{ s.include(c); }
	friend void exclude(charset& s, char b)						{ s.exclude(b); }
	friend void exclude(charset& s, char min, char max)			{ s.exclude(min, max); }
	friend void exclude(charset& s, const char* c)				{ s.exclude(c); }

	friend string<char   >& tostring(string<char   >& str, const charset& s);
	friend string<wchar_t>& tostring(string<wchar_t>& str, const charset& s);

};


string<char   >& tostring(string<char   >& str, const charset& s);
string<wchar_t>& tostring(string<wchar_t>& str, const charset& s);

template<typename T>
inline string<T>& operator<<(string<T>& str, const charset& s)
{
	return tostring(str, s);
}

string<char> tostring(const charset& s);



NAMESPACE_END(basics)

#endif//__BASECHARSET_H__

