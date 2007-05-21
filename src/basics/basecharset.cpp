
#include "basetypes.h"
#include "baseswap.h"
#include "baseobjects.h"
#include "basecharset.h"
#include "basealgo.h"
#include "basetime.h"
#include "basememory.h"
#include "basestring.h"

NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
// 4bit number <-> hex character
///////////////////////////////////////////////////////////////////////////////
template <typename T>
uchar char2hex(T c) 
{
	if( c>='a' && c<='f')
		return uchar(c - 'a' + 10);
	else if( c >= 'A' && c <= 'F')
		return uchar(c - 'A' + 10);
	else if( c >= '0' && c <= '9')
		return uchar(c - '0');
	else
		return 0;
}
// explicit instantiation
template uchar char2hex<char>(char c);
template uchar char2hex<wchar_t>(wchar_t c);


template <typename T>
uchar str2hex(const T*& p) 
{
	uchar ret = *p;
	if (ret == _csetesc)
	{
		p++;
		ret = *p;
		if ((ret >= '0' && ret <= '9') || (ret >= 'a' && ret <= 'f') || (ret >= 'A' && ret <= 'F')) {
			ret = char2hex<T>(ret);
			p++;
			if (*p != 0)
				ret = uchar((ret << 4) | char2hex<T>(*p));
		}
	}
	return ret;
}
// explicit instantiation
template uchar str2hex<char>(const char*& p);
template uchar str2hex<wchar_t>(const wchar_t*& p);


template <typename T>
T hex2char(uchar c)
{
	c &= 0x0f;
	if( c < 10 )
		return char(c + '0');
	else 
		return char(c - 10 + 'a');
}
// explicit instantiation
template char hex2char<char>(uchar c);
template wchar_t hex2char<wchar_t>(uchar c);




//////////////////////////////////////////////////////////////////////////
// ansi character set
//////////////////////////////////////////////////////////////////////////
static const uchar lbitmask[8] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};
static const uchar rbitmask[8] = {0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

void charset::include(char min, char max)
{
	if( uchar(min) > uchar(max) )
		swap(min, max);
	int lidx = uchar(min) >>3;	// /8
	int ridx = uchar(max) >>3;
	uchar lbits = lbitmask[ min&0x07 ];	// %8
	uchar rbits = rbitmask[ max&0x07 ];

	if (lidx == ridx) 
	{
		bytedata[lidx] |= lbits & rbits;
	}
	else 
	{
		bytedata[lidx] |= lbits;
		for (int i = lidx + 1; i < ridx; ++i)
			bytedata[i] = uchar(-1);
		bytedata[ridx] |= rbits;
	}
}
void charset::exclude(char min, char max)
{
	if( uchar(min) > uchar(max) )
		swap(min, max);
	int lidx = uchar(min) >>3;	// /8
	int ridx = uchar(max) >>3;	// /8
	uchar lbits = lbitmask[ min&0x07 ];
	uchar rbits = rbitmask[ max&0x07 ];

	if (lidx == ridx) 
	{
		bytedata[lidx] &= ~(lbits & rbits);
	}
	else 
	{
		bytedata[lidx] |= lbits;
		for (int i = lidx + 1; i < ridx; ++i)
			bytedata[i] = uchar(0);
		bytedata[ridx] &= ~rbits;
	}
}


void charset::assign(const char* p) 
{
	if (*p == '*' && *(p + 1) == 0)
		fill();
	else 
	{
		clear();
		for (; *p != 0; ++p) {
			uchar left = str2hex<char>(p);
			if (*(p + 1) == '-') {
				p += 2;
				uchar right = str2hex<char>(p);
				include(left, right);
			}
			else
				include(left);
		}
	}
}


void charset::unite(const charset& s) 
{
	for(int i = 0; i < _csetwords; ++i) 
		worddata[i] |= s.worddata[i];
}


void charset::subtract(const charset& s) 
{
	for(int i = 0; i < _csetwords; ++i) 
		worddata[i] &= ~s.worddata[i];
}


void charset::intersect(const charset& s) 
{
	for(int i = 0; i < _csetwords; ++i) 
		worddata[i] &= s.worddata[i];
}


void charset::invert() 
{
	for(int i = 0; i < _csetwords; ++i) 
		worddata[i] = ~worddata[i];
}


bool charset::le(const charset& s) const 
{
	for (int i = 0; i < _csetwords; ++i) 
	{
		ulong w1 = worddata[i];
		ulong w2 = s.worddata[i];
		if ((w2 | w1) != w2)
			return false;
	}
	return true;
}


inline bool isprintable(uchar c) 
{
	return ((c >= ' ') && (c < 127));
}

template<typename T>
static size_t showmember(uchar c, T *buffer) 
{
	if( (c == '-') || (c == '~') )
	{
		buffer[0] = '~';
		buffer[1] = c;
		buffer[2] = 0;
		return 2;
	}
	else if( isprintable(c) )
	{
		buffer[0] = c;
		buffer[1] = 0;
		return 1;
	}
	else 
	{
		buffer[0] = '~';
		buffer[1] = hex2char<T>(uchar(c >> 4));
		buffer[2] = hex2char<T>(uchar(c & 0x0f));
		buffer[3] = 0;
		return 3;
	}
}

string<char   >& tostring(string<char   >& str, const charset& s)
{
	int i, l = -1, r = -1;
	str.clear();
	for(i=0; i<=_csetbits; ++i) 
	{
		if( i<_csetbits && (uchar(i)&s) ) 
		{
			if (l == -1)
				l = i;
			else
				r = i;
		}
		else if (l != -1) 
		{	
			char buffer[16];
			size_t sz = showmember(uchar(l), buffer);
			str.append(buffer,sz);
			if (r != -1)
			{
				if (r > l + 1) 
					str.append('-');
				sz = showmember(uchar(r), buffer);
				str.append(buffer,sz);
			}
			l = -1;
			r = -1;
		}
	}
	return str;
}
string<wchar_t>& tostring(string<wchar_t>& str, const charset& s)
{
	int i, l = -1, r = -1;
	str.clear();
	for(i=0; i<=_csetbits; ++i) 
	{
		if( i<_csetbits && (uchar(i)&s) ) 
		{
			if (l == -1)
				l = i;
			else
				r = i;
		}
		else if (l != -1) 
		{	
			wchar_t buffer[16];
			size_t sz = showmember(uchar(l), buffer);
			str.append(buffer,sz);
			if (r != -1)
			{
				if (r > l + 1) 
					str.append('-');
				sz = showmember(uchar(r), buffer);
				str.append(buffer,sz);
			}
			l = -1;
			r = -1;
		}
	}
	return str;
}


string<char> tostring(const charset& s)
{
	string<char> ret;
	tostring(ret,s);
	return ret;
}





void test_charset()
{
	charset ca("~61-~67"), cb("xyz"), cc("fgklmnopx"), cx;

	printf( tostring(ca) ); printf("\n");
	printf( tostring(cb) ); printf("\n");
	printf( tostring(cc) ); printf("\n");

	cx = ca+cc;			printf( tostring(cx) ); printf("\n");
	cx = ca*cc;			printf( tostring(cx) ); printf("\n");
	cx = ca-cc;			printf( tostring(cx) ); printf("\n");
	cx = ca-"abc";		printf( tostring(cx) ); printf("\n");
	cx = !(ca+cb-cc);	printf( tostring(cx) ); printf("\n");

	printf("%i\n", ca.contains("hallo") );
	printf("%i\n", ca.containsall("hallo") );
}


NAMESPACE_END(basics)
