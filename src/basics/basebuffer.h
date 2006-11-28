#ifndef __BASEBUFFER__
#define __BASEBUFFER__

///////////////////////////////////////////////////////////////////////////////
// 
// 
///////////////////////////////////////////////////////////////////////////////
#include "basememory.h"
#include "baseinet.h"
#include "basesync.h"
#include "baseexceptions.h"


//## TODO: rewrite with the new memory allocation


NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// test functions
void test_buffer();




///////////////////////////////////////////////////////////////////////////////
// predeclares
class streamable;
class buffer;








///////////////////////////////////////////////////////////////////////////////
// 
template < class E=elaborator_st<unsigned char>, class A=allocator_rw_dy<unsigned char> >
class _basebuffer : public A, public E
{
protected:
	_basebuffer()	{}
	_basebuffer(unsigned char* buf, size_t sz) : A(buf,sz)	{}
	_basebuffer(size_t sz) : A(sz)	{}
public:
	~_basebuffer()	{}

	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator <<(const unsigned char ch)
	{
		if( this->checkwrite(1) )
		{
			*this->cWpp++ = ch;
		}
		return *this;
	}
	_basebuffer& operator <<(const char ch)
	{	
		if( this->checkwrite(1) )
		{
			*this->cWpp++ = (unsigned char)ch;
		}
		return *this;
	}
	bool assign(const unsigned char ch)
	{
		if( this->checkwrite(1) )
		{
			*this->cWpp++ = ch;
			return true;
		}
		else
			return false;
	}
	bool assign(const char ch)
	{
		if( this->checkwrite(1) )
		{
			*this->cWpp++ = (unsigned char)ch;
			return true;
		}
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator >>(unsigned char& ch)
	{
		if( this->checkread(1) )
			ch = *this->ccRpp++;
		else
			ch = 0;
		return *this;
	}
	_basebuffer& operator >>(char& ch)
	{	
		if( this->checkread(1) )
			ch = *this->ccRpp++;
		else
			ch = 0;
		return *this;
	}
	operator unsigned char () const
	{	
		if( this->checkread(1) )
			return *this->ccRpp++;
		else
			return 0;
	}
	operator char() const
	{	
		if( this->checkread(1) )
			return *this->ccRpp++;
		else
			return 0;
	}
	bool get(unsigned char& ch) const
	{	
		if( this->checkread(1) )
		{
			ch = *this->ccRpp++;
			return true;
		}
		else
			return false;
	}
	bool get(char& ch) const
	{	
		if( this->checkread(1) )
		{
			ch = *this->ccRpp++;
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator <<(const unsigned short sr)
	{	// implement little endian buffer format
		if( this->checkwrite(2) )
		{
			*this->cWpp++ = (unsigned char)(sr         ); 
			*this->cWpp++ = (unsigned char)(sr  >> 0x08);
		}
		return *this;
	}
	_basebuffer& operator <<(const short sr)
	{	// implement little endian buffer format
		if( this->checkwrite(2) )
		{
			*this->cWpp++ = (unsigned char)(sr         ); 
			*this->cWpp++ = (unsigned char)(sr  >> 0x08);
		}
		return *this;
	}
	bool assign(const unsigned short sr)
	{
		if( this->checkwrite(2) )
		{
			*this->cWpp++ = (unsigned char)(sr         ); 
			*this->cWpp++ = (unsigned char)(sr  >> 0x08);
			return true;
		}
		else
			return false;
	}
	bool assign(const short sr)
	{
		if( this->checkwrite(2) )
		{
			*this->cWpp++ = (unsigned char)(sr         ); 
			*this->cWpp++ = (unsigned char)(sr  >> 0x08);
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator >>(unsigned short& sr)
	{
		if( this->checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
		}
		else
			sr = 0;
		return *this;
	}
	_basebuffer& operator >>(short& sr)
	{	
		if( this->checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
		}
		else
			sr = 0;
		return *this;
	}
	operator unsigned short() const
	{	// implement little endian buffer format
		unsigned short sr=0;
		if( this->checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
		}
		return sr;
	}
	operator short () const
	{	// implement little endian buffer format
		short sr=0;
		if( this->checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
		}
		return sr;
	}
	bool get(unsigned short& sr) const
	{
		if( this->checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
			return true;
		}
		else
			return false;
	}
	bool get(short& sr) const
	{
		if( this->checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator <<(const uint32 ln)
	{	// implement little endian buffer format
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return *this;
	}
	_basebuffer& operator <<(const sint32 ln)
	{	// implement little endian buffer format
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return *this;
	}
	bool assign(const uint32 ln)
	{
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			return true;
		}
		else
			return false;
	}
	bool assign(const sint32 ln)
	{
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator >>(uint32& ln)
	{
		if( this->checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
		}
		else
			ln = 0;
		return *this;
	}
	_basebuffer& operator >>(sint32& ln)
	{	
		if( this->checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
		}
		else
			ln = 0;
		return *this;
	}
	operator uint32 () const
	{	// implement little endian buffer format
		unsigned long ln=0;
		if( this->checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
		}
		return ln;
	}
	operator sint32 () const
	{	// implement little endian buffer format
		long ln=0;
		if( this->checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
		}
		return ln;
	}
	bool get(uint32& ln) const
	{
		if( this->checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
			return true;
		}
		else
			return false;
	}
	bool get(sint32& ln) const
	{
		if( this->checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator <<(const unsigned long ln)
	{	// implement little endian buffer format
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x20 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x28 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x30 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x38 );
		}
#else
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
#endif
		return *this;
	}
	_basebuffer& operator <<(const long ln)
	{	// implement little endian buffer format
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x20 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x28 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x30 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x38 );
		}
#else
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
#endif
		return *this;
	}
	bool assign(const unsigned long ln)
	{
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x20 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x28 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x30 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x38 );
			return true;
		}
		else
			return false;
#else
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			return true;
		}
		else
			return false;
#endif
	}
	bool assign(const long ln)
	{
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x20 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x28 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x30 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x38 );
			return true;
		}
		else
			return false;
#else
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			return true;
		}
		else
			return false;
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator >>(unsigned long& ln)
	{
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkread(8) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			ln |= ((unsigned long)(*this->cRpp++) << 0x20);
			ln |= ((unsigned long)(*this->cRpp++) << 0x28);
			ln |= ((unsigned long)(*this->cRpp++) << 0x30);
			ln |= ((unsigned long)(*this->cRpp++) << 0x38);
		}
		else
			ln = 0;
#else
		if( this->checkread(4) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
		}
		else
			ln = 0;
#endif
		return *this;
	}
	_basebuffer& operator >>(long& ln)
	{	
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkread(8) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			ln |= ((unsigned long)(*this->cRpp++) << 0x20);
			ln |= ((unsigned long)(*this->cRpp++) << 0x28);
			ln |= ((unsigned long)(*this->cRpp++) << 0x30);
			ln |= ((unsigned long)(*this->cRpp++) << 0x38);
		}
		else
			ln = 0;
#else
		if( this->checkread(4) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
		}
		else
			ln = 0;
#endif
		return *this;
	}
	operator unsigned long () const
	{	// implement little endian buffer format
		unsigned long ln=0;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkread(8) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			ln |= ((unsigned long)(*this->cRpp++) << 0x20);
			ln |= ((unsigned long)(*this->cRpp++) << 0x28);
			ln |= ((unsigned long)(*this->cRpp++) << 0x30);
			ln |= ((unsigned long)(*this->cRpp++) << 0x38);
		}
#else
		if( this->checkread(4) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
		}
#endif
		return ln;
	}
	operator long () const
	{	// implement little endian buffer format
		long ln=0;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkread(8) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			ln |= ((unsigned long)(*this->cRpp++) << 0x20);
			ln |= ((unsigned long)(*this->cRpp++) << 0x28);
			ln |= ((unsigned long)(*this->cRpp++) << 0x30);
			ln |= ((unsigned long)(*this->cRpp++) << 0x38);
		}
#else
		if( this->checkread(4) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
		}
#endif
		return ln;
	}
	bool get(unsigned long& ln) const
	{
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkread(8) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			ln |= ((unsigned long)(*this->cRpp++) << 0x20);
			ln |= ((unsigned long)(*this->cRpp++) << 0x28);
			ln |= ((unsigned long)(*this->cRpp++) << 0x30);
			ln |= ((unsigned long)(*this->cRpp++) << 0x38);
			return true;
		}
		else
			return false;
#else
		if( this->checkread(4) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			return true;
		}
		else
			return false;
#endif
	}
	bool get(long& ln) const
	{
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( this->checkread(8) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			ln |= ((unsigned long)(*this->cRpp++) << 0x20);
			ln |= ((unsigned long)(*this->cRpp++) << 0x28);
			ln |= ((unsigned long)(*this->cRpp++) << 0x30);
			ln |= ((unsigned long)(*this->cRpp++) << 0x38);
			return true;
		}
		else
			return false;
#else
		if( this->checkread(4) )
		{	
			ln  = ((unsigned long)(*this->cRpp++)        ); 
			ln |= ((unsigned long)(*this->cRpp++) << 0x08);
			ln |= ((unsigned long)(*this->cRpp++) << 0x10);
			ln |= ((unsigned long)(*this->cRpp++) << 0x18);
			return true;
		}
		else
			return false;
#endif
	}

	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator <<(const ipaddress ip)
	{	// implement little endian buffer format
		// IPs are given in network byte order to the buffer
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ip[3]);
			*this->cWpp++ = (unsigned char)(ip[2]);
			*this->cWpp++ = (unsigned char)(ip[1]);
			*this->cWpp++ = (unsigned char)(ip[0]);
		}
		return *this;
	}
	bool assign(const ipaddress ip)
	{
		if( this->checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ip[3]);
			*this->cWpp++ = (unsigned char)(ip[2]);
			*this->cWpp++ = (unsigned char)(ip[1]);
			*this->cWpp++ = (unsigned char)(ip[0]);
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator >>(ipaddress& ip)
	{
		if( this->checkread(4) )
		{	
			ip = ipaddress( this->cRpp[0],this->cRpp[1],this->cRpp[2],this->cRpp[3] );
			this->cRpp+=4;
		}
		else
			ip = INADDR_ANY;
		return *this;
	}
	operator ipaddress () const
	{	// implement little endian buffer format
		if( this->checkread(4) )
		{
			ipaddress ip( this->cRpp[0],this->cRpp[1],this->cRpp[2],this->cRpp[3] );
			this->cRpp+=4;
			return  ip;
		}
		return INADDR_ANY;
	}
	bool get(ipaddress& ip) const
	{
		if( this->checkread(4) )
		{	
			ip = ipaddress( this->cRpp[0],this->cRpp[1],this->cRpp[2],this->cRpp[3] );
			this->cRpp+=4;
			return true;
		}
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator <<(const sint64 lx)
	{	// implement little endian buffer format
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(lx          );
			*this->cWpp++ = (unsigned char)(lx  >> 0x08 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x10 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x18 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x20 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x28 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x30 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return *this;
	}
	_basebuffer& operator <<(const uint64 lx)
	{	// implement little endian buffer format
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(lx          );
			*this->cWpp++ = (unsigned char)(lx  >> 0x08 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x10 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x18 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x20 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x28 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x30 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return *this;
	}
	bool assign(const uint64 lx)
	{
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(lx          );
			*this->cWpp++ = (unsigned char)(lx  >> 0x08 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x10 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x18 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x20 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x28 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x30 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x38 );
			return true;
		}
		else
			return false;
	}
	bool assign(const sint64 lx)
	{
		if( this->checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(lx          );
			*this->cWpp++ = (unsigned char)(lx  >> 0x08 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x10 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x18 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x20 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x28 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x30 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x38 );
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator >>(uint64& lx)
	{
		if( this->checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
		}
		else
			lx = 0;
		return *this;
	}
	_basebuffer& operator >>(sint64& lx)
	{
		if( this->checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
		}
		else
			lx = 0;
		return *this;
	}
	operator sint64 () const
	{	// implement little endian buffer format
		sint64 lx=0;
		if( this->checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
		}
		return lx;
	}
	operator uint64 () const
	{	// implement little endian buffer format
		uint64 lx=0;
		if( this->checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
		}
		return lx;
	}
	bool get(uint64& lx)
	{
		if( this->checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
			return true;
		}
		else
			return false;
	}
	bool get(sint64& lx)
	{
		if( this->checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
			return true;
		}
		else
			return false;

	}
	///////////////////////////////////////////////////////////////////////////
	// direct assignements of strings to buffers should be avoided
	// if a fixed buffer scheme is necessary;
	// use str2buffer / buffer2str instead
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator <<(const char * c)
	{	
		size_t sz= (c) ? hstrlen(c)+1 : 1;
		if( this->checkwrite(sz) )
		{
			if( c )
			{
				memcpy(this->cWpp, c, sz);
				this->cWpp+=sz;
			}
			else
				*this->cWpp++=0;
		}
		return *this;
	}
	bool assign(const char * c)
	{
		unsigned char* tmp=this->cWpp;
		if( !c && this->checkwrite(1) )
		{
			*this->cWpp++=0;
			return true;
		}
		else
		{
			while( this->checkwrite(1) )
			{
				*this->cWpp++ = *c++;
				if( !*c )
					return true;
			}
			this->cWpp = tmp;
			return false;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	_basebuffer& operator >>(char *c)
	{
		if(c)
		{
			unsigned char *ix = this->cRpp;
			char *pc = c;

			while( ix<this->cWpp && *ix )
				*pc++ = *ix++;
			if( ix<this->cWpp )
			{	// copy the EOS
				*pc++ = *ix++;

				this->cRpp = ix;
				return true;
			}
			else
			{	// no eos, so there is no string
				*c = 0;
				return false;
			}
		}
	}
	operator const char*() const
	{	// find the EOS
		// cannot do anything else then going through the array
		unsigned char *ix = this->cRpp;
		while( this->cRpp<this->cWpp && *this->cRpp );
		if(this->cRpp<this->cWpp)
		{	// skip the eos in the buffer
			// it belongs to the string
			this->cRpp++;
			return (char*)ix;
		}
		else
		{	// no eos, so there is no string
			this->cRpp = ix;
			return NULL;
		}
	}
	// no get() here
	///////////////////////////////////////////////////////////////////////////
	// string access with fixed string size
	///////////////////////////////////////////////////////////////////////////
	bool str2buffer(const char *c, size_t sz)
	{
		if( this->checkwrite(sz) )
		{
			if( c )
			{
				size_t cpsz = ( cpsz > strlen(c)+1 ) ? sz : strlen(c)+1;
				memcpy(this->cWpp, c, cpsz);
				this->cWpp[cpsz-1] = 0;	// force an EOS
			}
			else
				memset(this->cWpp, 0, sz);
			this->cWpp+=sz;
			return true;
		}
		return false;
	}
	bool buffer2str(char *c, size_t sz) const
	{
		if( this->checkread(sz) )
		{
			if( c )
			{	
				memcpy(c, this->cRpp, sz);
				c[sz-1] = 0;	// force an EOS
				this->cRpp+=sz;
				return true;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// read/write access for unsigned char arrays
	///////////////////////////////////////////////////////////////////////////
	bool write(const unsigned char *c, size_t sz)
	{
		if( this->checkwrite(sz) )
		{
			if( c )
				memcpy(this->cWpp, c, sz);
			this->cWpp+=sz;
			return true;
		}
		return false;
	}
	bool read(unsigned char *c, size_t sz)
	{
		if( this->checkread(sz) )
		{
			if( c )
			{	
				memcpy(c, this->cRpp, sz);
				this->cRpp+=sz;
				return true;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer and "manual" contol
	// use with care
	///////////////////////////////////////////////////////////////////////////
	unsigned char* operator()(size_t offset=0, size_t size=0)
	{
		return this->get_rptr(offset); 
	}
	unsigned char* get_rptr(size_t offset=0, size_t size=0)
	{
		if( !this->checkread(offset+size) )
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("Buffer GetReadpointer out of bound");
#else
			return NULL;
#endif
		return this->cRpp+offset; 
	}
	unsigned char* get_wptr(size_t offset=0, size_t size=0)
	{
		if( !this->checkwrite(offset+size) )
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("Buffer GetWritepointer out of bound");
#else
			return NULL;
#endif
		return this->cWpp+offset; 
	}
	bool step_rptr(int i)
	{	// can go backwards with negative offset
		if(this->cRpp+i <= this->cWpp && this->cRpp+i >= this->cBuf)
		{
			this->cRpp+=i;
			return true;
		}
		return false;
	}
	bool step_wptr(int i)
	{	// can go backwards with negative offset
		if(this->cWpp+i <= this->cEnd && this->cWpp+i >= this->cRpp)
		{
			this->cWpp+=i;
			return true;
		}
		return false;
	}

	
	///////////////////////////////////////////////////////////////////////////
	// the next will combine the buffer_iterator with the 
	// virtual streamable class allowing to derive
	// classes that are auto-assignable to buffers
	///////////////////////////////////////////////////////////////////////////
	const streamable& operator = (const streamable& s);


	///////////////////////////////////////////////////////////////////////////
	// I explicitely exclude int operators since the usage of ints 
	// should be generally banned from any portable storage scheme
	//
	// assigning an int will now cause an ambiquity 
	// which has to be solved by casting to a proper type with fixed size
	///////////////////////////////////////////////////////////////////////////
/*
	_basebuffer& operator <<(const int i)
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:	// 8bit systems should be quite rare
			(*this) = (char)i;
			break;
		case 2:	// 16bits might be not that common
			(*this) = (short)i;
			break;
		case 4:	// 32bits (still) exist
			(*this) = (sint32)i;
			break;
		case 8:	// 64bits are rising
			(*this) = (sint64)i;
			break;
				// the rest is far future
		}
		return *this;
	}
	operator int () const
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (char)(*this);
		case 2:
			return (short)(*this);
		case 4:
			return (sint32)(*this);
		case 8:
			return (sint64)(*this);
		}
	}
	_basebuffer& operator <<(const unsigned int i)
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			(*this) = (uchar)i;
			break;
		case 2:
			(*this) = (ushort)i;
			break;
		case 4:
			(*this) = (uint32)i;
			break;
		case 8:
			(*this) = (uint64)i;
			break;
		}
		return *this;
	}
	operator unsigned int () const
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (uchar)(*this);
		case 2:
			return (ushort)(*this);
		case 4:
			return (uint32)(*this);
		case 8:
			return (uint64)(*this);
		}
	}
*/
};

class _fixbuffer : public _basebuffer< elaborator_st<unsigned char>, allocator_rw_st< unsigned char> >
{
public:
	_fixbuffer(unsigned char*buf, size_t sz) : _basebuffer< elaborator_st<unsigned char>, allocator_rw_st< unsigned char> >(buf,sz) {}
	virtual~_fixbuffer() {}
};

class _bufferaccess;

class _buffer : public _basebuffer< elaborator_st<unsigned char>, allocator_rw_dy< unsigned char> >, public Mutex
{
	friend class _bufferaccess;
	_bufferaccess* cAccess;
public:
	_buffer() : cAccess(NULL) {}
	_buffer(size_t sz) : _basebuffer< elaborator_st<unsigned char>, allocator_rw_dy< unsigned char> >(sz), cAccess(NULL) {}
	virtual~_buffer() {}
};


class _bufferaccess : public noncopyable, public nonallocable
{

	class _bufferwriter
	{
		friend class _bufferaccess;
		_buffer*	cBuffer;
		size_t		cOffset;

		_bufferwriter(_buffer* b, size_t o) : cBuffer(b), cOffset(o) {}
	public:
		~_bufferwriter()	{}


		template<class T> const _bufferwriter& operator =(const T& t)
		{
			if(cBuffer)
				_fixbuffer( cBuffer->get_wptr(cOffset, sizeof(T)), sizeof(T)) << t;
			return *this;
		}
	};


	_buffer* cBuffer;
public:
	_bufferaccess() : cBuffer(NULL)	{}
	_bufferaccess(_buffer& b, size_t sz=0, bool init=true) : cBuffer(NULL)	{ this->aquire(b, sz, init); }
	~_bufferaccess()	{ this->release(0); }

	bool aquire(_buffer& b, size_t sz=0, bool init=true)
	{
		b.Mutex::enter();
		if( !b.cAccess )
		{
			this->cBuffer = &b;
			b.cAccess = this;
			if(sz)
			{
				b.checkwrite(sz);
				if(init) memset(this->cBuffer->cWpp, 0, sz*sizeof(unsigned char));
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	bool release(size_t sz)
	{
		if(this->cBuffer)
		{
			this->cBuffer->step_wptr(sz);

			this->cBuffer->Mutex::leave();
			this->cBuffer->cAccess = NULL;
			this->cBuffer = NULL;
			return true;
		}
		else
			return false;
	}
	bool isvalid()	{ return NULL!=cBuffer; }



	_bufferwriter operator[](size_t offs)
	{
		return _bufferwriter(cBuffer, offs);
	}
};

































///////////////////////////////////////////////////////////////////////////////
// uchar buffer access
//
// uses seperated read and write pointer; pointers do not go rounds, 
// so it needs an explicit sync at r/w operations
//
///////////////////////////////////////////////////////////////////////////////
class buffer_iterator : public global
{
	friend class buffer;

protected:
	mutable unsigned char *cRpp;	// read pointer inside the buffer
	mutable unsigned char *cWpp;	// write pointer inside the buffer
	unsigned char *start;		// pointer to the start of the buffer
	unsigned char *end;			// pointer to the end of the buffer


	buffer_iterator()
		: cRpp(NULL), cWpp(NULL), start(NULL), end(NULL)
	{}

	// don't define a copy constructor
	buffer_iterator(const buffer_iterator& bi);
public:
	buffer_iterator(unsigned char* b, size_t sz)
	{
		this->cRpp  =b;
		this->cWpp  =b;
		this->start=b;
		this->end  =b+sz;
	}
	virtual ~buffer_iterator()
	{
		this->cRpp=this->cWpp=this->start=this->end=NULL;
	}

	// assignment
	const buffer_iterator& operator=(const buffer_iterator& bi)
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
		return *this;
	}

	// access functions
	bool eof() const		{ return !(this->cWpp && this->cWpp<this->end); }
	size_t size() const		{ return this->end-this->start; }
	size_t datasize() const	{ return this->cWpp-this->cRpp; }
	size_t freesize() const	{ return this->end-this->cWpp; }


	///////////////////////////////////////////////////////////////////////////
	virtual bool checkwrite(size_t addsize)
	{
		if( this->cWpp+addsize > this->end && this->start < this->cRpp )
		{	// move the current buffer data when necessary and possible 
			memmove(this->start, this->cRpp, this->cWpp-this->cRpp);
			this->cWpp = this->start+(this->cWpp-this->cRpp);
			this->cRpp = this->start;
		}
		return ( this->cWpp && this->cWpp+addsize<=this->end );
	}
	virtual bool checkread(size_t addsize) const
	{

		return ( this->cRpp && this->cRpp+addsize<=this->cWpp );
	}

	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		if( checkwrite(1) )
		{
			*this->cWpp++ = ch;
		}
		return ch;
	}
	char operator = (const char ch)
	{	
		if( checkwrite(1) )
		{
			*this->cWpp++ = (unsigned char)ch;
		}
		return ch;
	}
	bool assign_char(const unsigned char ch)
	{
		if( checkwrite(1) )
		{
			*this->cWpp++ = ch;
			return true;
		}
		return false;
	}
	bool assign_char(const char ch)
	{
		if( checkwrite(1) )
		{
			*this->cWpp++ = (unsigned char)ch;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	operator unsigned char () const
	{	
		if( checkread(1) )
			return *this->cRpp++;
		return 0;
	}
	operator char() const
	{	
		if( checkread(1) )
			return *this->cRpp++;
		return 0;

	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{	// implement little endian buffer format
		if( checkwrite(2) )
		{
			*this->cWpp++ = (unsigned char)(sr         ); 
			*this->cWpp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	short operator = (const short sr)
	{	// implement little endian buffer format
		if( checkwrite(2) )
		{
			*this->cWpp++ = (unsigned char)(sr         ); 
			*this->cWpp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short() const
	{	// implement little endian buffer format
		unsigned short sr=0;
		if( checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
		}
		return sr;
	}
	operator short () const
	{	// implement little endian buffer format
		short sr=0;
		if( checkread(2) )
		{	
			sr  = ((unsigned short)(*this->cRpp++)        ); 
			sr |= ((unsigned short)(*this->cRpp++) << 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	uint32 operator = (const uint32 ln)
	{	// implement little endian buffer format
		if( checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	sint32 operator = (const sint32 ln)
	{	// implement little endian buffer format
		if( checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	operator uint32 () const
	{	// implement little endian buffer format
		unsigned long ln=0;
		if( checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
		}
		return ln;
	}
	operator sint32 () const
	{	// implement little endian buffer format
		long ln=0;
		if( checkread(4) )
		{	
			ln  = ((uint32)(*this->cRpp++)        ); 
			ln |= ((uint32)(*this->cRpp++) << 0x08);
			ln |= ((uint32)(*this->cRpp++) << 0x10);
			ln |= ((uint32)(*this->cRpp++) << 0x18);
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{	// implement little endian buffer format
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x20 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x28 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x30 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x38 );
		}
#else
		if( checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
#endif
		return ln;
	}
	long operator = (const long ln)
	{	// implement little endian buffer format
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x20 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x28 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x30 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x38 );
		}
#else
		if( checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ln          );
			*this->cWpp++ = (unsigned char)(ln  >> 0x08 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x10 );
			*this->cWpp++ = (unsigned char)(ln  >> 0x18 );
		}
#endif
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long () const
	{	// implement little endian buffer format
		unsigned long lx=0;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( checkread(8) )
		{	
			lx  = ((unsigned long)(*this->cRpp++)        ); 
			lx |= ((unsigned long)(*this->cRpp++) << 0x08);
			lx |= ((unsigned long)(*this->cRpp++) << 0x10);
			lx |= ((unsigned long)(*this->cRpp++) << 0x18);
			lx |= ((unsigned long)(*this->cRpp++) << 0x20);
			lx |= ((unsigned long)(*this->cRpp++) << 0x28);
			lx |= ((unsigned long)(*this->cRpp++) << 0x30);
			lx |= ((unsigned long)(*this->cRpp++) << 0x38);
		}
#else
		if( checkread(4) )
		{	
			lx  = ((unsigned long)(*this->cRpp++)        ); 
			lx |= ((unsigned long)(*this->cRpp++) << 0x08);
			lx |= ((unsigned long)(*this->cRpp++) << 0x10);
			lx |= ((unsigned long)(*this->cRpp++) << 0x18);
		}
#endif
		return lx;
	}
	operator long () const
	{	// implement little endian buffer format
		long lx=0;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
		if( checkread(8) )
		{	
			lx  = ((unsigned long)(*this->cRpp++)        ); 
			lx |= ((unsigned long)(*this->cRpp++) << 0x08);
			lx |= ((unsigned long)(*this->cRpp++) << 0x10);
			lx |= ((unsigned long)(*this->cRpp++) << 0x18);
			lx |= ((unsigned long)(*this->cRpp++) << 0x20);
			lx |= ((unsigned long)(*this->cRpp++) << 0x28);
			lx |= ((unsigned long)(*this->cRpp++) << 0x30);
			lx |= ((unsigned long)(*this->cRpp++) << 0x38);
		}
#else
		if( checkread(4) )
		{	
			lx  = ((unsigned long)(*this->cRpp++)        ); 
			lx |= ((unsigned long)(*this->cRpp++) << 0x08);
			lx |= ((unsigned long)(*this->cRpp++) << 0x10);
			lx |= ((unsigned long)(*this->cRpp++) << 0x18);
		}
#endif
		return lx;
	}
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{	// implement little endian buffer format
		// IPs are given in network byte order to the buffer
		if( checkwrite(4) )
		{
			*this->cWpp++ = (unsigned char)(ip[3]);
			*this->cWpp++ = (unsigned char)(ip[2]);
			*this->cWpp++ = (unsigned char)(ip[1]);
			*this->cWpp++ = (unsigned char)(ip[0]);
		}
		return ip;
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress () const
	{	// implement little endian buffer format
		if( checkread(4) )
		{
			ipaddress ip( this->cRpp[0],this->cRpp[1],this->cRpp[2],this->cRpp[3] );
			this->cRpp+=4;
			return  ip;
		}
		return (uint32)INADDR_ANY;
	}
	///////////////////////////////////////////////////////////////////////////
	sint64 operator = (const sint64 lx)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(lx          );
			*this->cWpp++ = (unsigned char)(lx  >> 0x08 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x10 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x18 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x20 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x28 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x30 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	uint64 operator = (const uint64 lx)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->cWpp++ = (unsigned char)(lx          );
			*this->cWpp++ = (unsigned char)(lx  >> 0x08 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x10 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x18 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x20 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x28 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x30 );
			*this->cWpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	///////////////////////////////////////////////////////////////////////////
	operator sint64 () const
	{	// implement little endian buffer format
		sint64 lx=0;
		if( checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
		}
		return lx;
	}
	operator uint64 () const
	{	// implement little endian buffer format
		uint64 lx=0;
		if( checkread(8) )
		{	
			lx  = ((uint64)(*this->cRpp++)        ); 
			lx |= ((uint64)(*this->cRpp++) << 0x08);
			lx |= ((uint64)(*this->cRpp++) << 0x10);
			lx |= ((uint64)(*this->cRpp++) << 0x18);
			lx |= ((uint64)(*this->cRpp++) << 0x20);
			lx |= ((uint64)(*this->cRpp++) << 0x28);
			lx |= ((uint64)(*this->cRpp++) << 0x30);
			lx |= ((uint64)(*this->cRpp++) << 0x38);
		}
		return lx;
	}

	///////////////////////////////////////////////////////////////////////////
	// direct assignements of strings to buffers should be avoided
	// if a fixed buffer scheme is necessary;
	// use str2buffer / buffer2str instead
	///////////////////////////////////////////////////////////////////////////
	const char* operator = (const char * c)
	{	
		size_t sz= (c) ? hstrlen(c)+1 : 1;
		if( checkwrite(sz) )
		{
			if( c )
			{
				memcpy(this->cWpp, c, sz);
				this->cWpp+=sz;
			}
			else
				*this->cWpp++=0;
		}
		return c;
	}
	operator const char*() const
	{	// find the EOS
		// cannot do anything else then going through the array
		unsigned char *ix = this->cRpp;
		while( this->cRpp<this->cWpp && this->cRpp );
		if(this->cRpp<this->cWpp)
		{	// skip the eos in the buffer
			// it belongs to the string
			this->cRpp++;
			return (char*)ix;
		}
		else
		{	// no eos, so there is no string
			this->cRpp = ix;
			return NULL;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// string access with fixed string size
	///////////////////////////////////////////////////////////////////////////
	bool str2buffer(const char *c, size_t sz)
	{
		if( checkwrite(sz) )
		{
			if( c )
			{
				size_t cpsz = ( cpsz > hstrlen(c)+1 ) ? sz : hstrlen(c)+1;
				memcpy(this->cWpp, c, cpsz);
				this->cWpp[cpsz-1] = 0;	// force an EOS
			}
			else
				memset(this->cWpp, 0, sz);
			this->cWpp+=sz;
			return true;
		}
		return false;
	}
	bool buffer2str(char *c, size_t sz) const
	{
		if( checkread(sz) )
		{
			if( c )
			{	
				memcpy(c, this->cRpp, sz);
				c[sz-1] = 0;	// force an EOS
				this->cRpp+=sz;
				return true;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// read/write access for unsigned char arrays
	///////////////////////////////////////////////////////////////////////////
	bool write(const unsigned char *c, size_t sz)
	{
		if( checkwrite(sz) )
		{
			if( c )
				memcpy(this->cWpp, c, sz);
			this->cWpp+=sz;
			return true;
		}
		return false;
	}
	bool read(unsigned char *c, size_t sz)
	{
		if( checkread(sz) )
		{
			if( c )
			{	
				memcpy(c, this->cRpp, sz);
				this->cRpp+=sz;
				return true;
			}
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer and "manual" contol
	// use with care
	///////////////////////////////////////////////////////////////////////////
	unsigned char* operator()(size_t offset=0, size_t size=0)
	{
		return get_rptr(offset); 
	}
	unsigned char* get_rptr(size_t offset=0, size_t size=0)
	{
		if( !checkread(offset+size) )
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("Buffer GetReadpointer out of bound");
#else
			return NULL;
#endif
		return this->cRpp+offset; 
	}
	unsigned char* get_wptr(size_t offset=0, size_t size=0)
	{
		if( !checkwrite(offset+size) )
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("Buffer GetWritepointer out of bound");
#else
			return NULL;
#endif
		return this->cWpp+offset; 
	}
	bool step_rptr(int i)
	{	// can go backwards with negative offset
		if(this->cRpp+i <= this->cWpp && this->cRpp+i >= this->start)
		{
			this->cRpp+=i;
			return true;
		}
		return false;
	}
	bool step_wptr(int i)
	{	// can go backwards with negative offset
		if(this->cWpp+i <= this->end && this->cWpp+i >= this->cRpp)
		{
			this->cWpp+=i;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// returning readable/writable and size checked portions of the buffer
	// that behave like a fixed buffer for inserting packets directly
	///////////////////////////////////////////////////////////////////////////
	buffer_iterator get_readbuffer(size_t sz)
	{
		if( checkread(sz) )
			return buffer_iterator(this->cRpp, sz);
		else
			return buffer_iterator();
	}
	buffer_iterator get_writebuffer(size_t sz)
	{
		if( checkwrite(sz) )
			return buffer_iterator(this->cWpp, sz);
		else
			return buffer_iterator();
	}
	///////////////////////////////////////////////////////////////////////////
	// the next will combine the buffer_iterator with the 
	// virtual streamable class allowing to derive
	// classes that are auto-assignable to buffers
	///////////////////////////////////////////////////////////////////////////
	const streamable& operator = (const streamable& s);


	///////////////////////////////////////////////////////////////////////////
	// I explicitely exclude int operators since the usage of ints 
	// should be generally banned from any portable storage scheme
	//
	// assigning an int will now cause an ambiquity 
	// which has to be solved by casting to a proper type with fixed size
	///////////////////////////////////////////////////////////////////////////
/*
	int operator = (const int i)
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:	// 8bit systems should be quite rare
			(*this) = (char)i;
			break;
		case 2:	// 16bits might be not that common
			(*this) = (short)i;
			break;
		case 4:	// 32bits (still) exist
			(*this) = (sint32)i;
			break;
		case 8:	// 64bits are rising
			(*this) = (sint64)i;
			break;
				// the rest is far future
		}
		return i;
	}
	operator int () const
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (char)(*this);
		case 2:
			return (short)(*this);
		case 4:
			return (sint32)(*this);
		case 8:
			return (sint64)(*this);
		}
	}
	unsigned int operator = (const unsigned int i)
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			(*this) = (uchar)i;
			break;
		case 2:
			(*this) = (ushort)i;
			break;
		case 4:
			(*this) = (uint32)i;
			break;
		case 8:
			(*this) = (uint64)i;
			break;
		}
		return i;
	}
	operator unsigned int () const
	{	// implement little endian buffer format
		switch(sizeof(int))
		{
		case 1:
			return (uchar)(*this);
		case 2:
			return (ushort)(*this);
		case 4:
			return (uint32)(*this);
		case 8:
			return (uint64)(*this);
		}
	}
*/
};



template <size_t SZ> class buffer_fixed : public buffer_iterator
{
	unsigned char cBuf[ (SZ<=16) ? 16 : (SZ<=32) ? 32 : (SZ<=2048) ? (SZ+63)&(~63) : (SZ+4095)&(~4095) ];
public:
	buffer_fixed() : buffer_iterator(cBuf, (SZ<=16) ? 16 : (SZ<=32) ? 32 : (SZ<=2048) ? (SZ+63)&(~63) : (SZ+4095)&(~4095))
	{}
	virtual ~buffer_fixed()
	{}

	// copy/assignment
	buffer_fixed(const buffer_fixed& bi) : buffer_iterator()
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
	}
	const buffer_fixed& operator=(const buffer_fixed& bi)
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
		return *this;
	}

	// baseclass copy/assignment
	buffer_fixed(const buffer_iterator& bi) : buffer_iterator()
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
	}
	const buffer_fixed& operator=(const buffer_iterator& bi)
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
		return *this;
	}



	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	char operator = (const char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned char () const
	{
		return this->buffer_iterator::operator unsigned char();
	}
	operator char() const
	{
		return this->buffer_iterator::operator char();
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	short operator = (const short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short() const
	{
		return this->buffer_iterator::operator unsigned short();
	}
	operator short () const
	{
		return this->buffer_iterator::operator short();
	}
	///////////////////////////////////////////////////////////////////////////
	uint32 operator = (const uint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	sint32 operator = (const sint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator uint32 () const
	{
		return this->buffer_iterator::operator uint32();
	}
	operator sint32 () const
	{
		return this->buffer_iterator::operator sint32();
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	long operator = (const long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long () const
	{
		return this->buffer_iterator::operator unsigned long();
	}
	operator long () const
	{
		return this->buffer_iterator::operator long();
	}
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{
		return this->buffer_iterator::operator=(ip);
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress () const
	{
		return this->buffer_iterator::operator ipaddress();
	}
	///////////////////////////////////////////////////////////////////////////
	sint64 operator = (const sint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	uint64 operator = (const uint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	///////////////////////////////////////////////////////////////////////////
	operator sint64 () const
	{
		return this->buffer_iterator::operator uint64();
	}
	operator uint64 () const
	{
		return this->buffer_iterator::operator sint64();
	}

	///////////////////////////////////////////////////////////////////////////
	// direct assignements of strings to buffers should be avoided
	// if a fixed buffer scheme is necessary;
	// use str2buffer / buffer2str instead
	///////////////////////////////////////////////////////////////////////////
	const char* operator = (const char * c)
	{
		return this->buffer_iterator::operator=(c);
	}
	operator const char*() const
	{
		return this->buffer_iterator::operator const char*();
	}
};



///////////////////////////////////////////////////////////////////////////////
// uchar 
class buffer : public buffer_iterator
{
public:
	buffer() : buffer_iterator()
	{}
	virtual ~buffer()
	{
		if(this->start) delete[] this->start;
		this->cRpp=this->cWpp=this->start=this->end=NULL;
	}

	// copy/assignment
	buffer(const buffer& bi) : buffer_iterator()
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
	}
	const buffer& operator=(const buffer& bi)
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
		return *this;
	}

	// baseclass copy/assignment
	buffer(const buffer_iterator& bi) : buffer_iterator()
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
	}
	const buffer& operator=(const buffer_iterator& bi)
	{
		write(bi.cRpp, bi.cWpp-bi.cRpp);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	virtual bool checkwrite(size_t addsize)
	{
		static const size_t quant1 = (size_t)(   64);
		static const size_t qmask1 = (size_t)(  ~63);
		static const size_t quant2 = (size_t)( 4096);
		static const size_t qmask2 = (size_t)(~4095);

		size_t sz = this->cWpp-this->cRpp + addsize;

		if( this->start+sz > this->end ||
			(this->start+8092 < this->end && this->start+4*sz < this->end) )
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quater of the buffer
			if( sz <= 16 )
				sz = 16;
			else if( sz <= 32 )
				sz = 32;
			else if( sz <= 2048 )
				sz = (sz + quant1 - 1) & qmask1;
			else
				sz = (sz + quant2 - 1) & qmask2;

			unsigned char *tmp = new unsigned char[sz];
			if(this->start)
			{
				memcpy(tmp, this->cRpp, this->cWpp-this->cRpp);
				delete[] this->start;
			}
			this->end = tmp+sz;
			this->cWpp = tmp+(this->cWpp-this->cRpp);
			this->cRpp = tmp;
			this->start = tmp;
		}
		else if( this->cWpp+addsize > this->end )
		{	// moving the current buffer data is sufficient
			memmove(this->start, this->cRpp, this->cWpp-this->cRpp);
			this->cWpp = this->start+(this->cWpp-this->cRpp);
			this->cRpp = this->start;
		}
		// else everything ok
		return (this->end > this->start);
	}

	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	char operator = (const char ch)
	{
		return this->buffer_iterator::operator=(ch);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned char () const
	{
		return this->buffer_iterator::operator unsigned char();
	}
	operator char() const
	{
		return this->buffer_iterator::operator char();
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	short operator = (const short sr)
	{
		return this->buffer_iterator::operator=(sr);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short() const
	{
		return this->buffer_iterator::operator unsigned short();
	}
	operator short () const
	{
		return this->buffer_iterator::operator short();
	}
	///////////////////////////////////////////////////////////////////////////
	uint32 operator = (const uint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	sint32 operator = (const sint32 ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator uint32 () const
	{
		return this->buffer_iterator::operator uint32();
	}
	operator sint32 () const
	{
		return this->buffer_iterator::operator sint32();
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	long operator = (const long ln)
	{
		return this->buffer_iterator::operator=(ln);
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long () const
	{
		return this->buffer_iterator::operator unsigned long();
	}
	operator long () const
	{
		return this->buffer_iterator::operator long();
	}
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{
		return this->buffer_iterator::operator=(ip);
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress () const
	{
		return this->buffer_iterator::operator ipaddress();
	}
	///////////////////////////////////////////////////////////////////////////
	sint64 operator = (const sint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	uint64 operator = (const uint64 lx)
	{
		return this->buffer_iterator::operator=(lx);
	}
	///////////////////////////////////////////////////////////////////////////
	operator sint64 () const
	{
		return this->buffer_iterator::operator uint64();
	}
	operator uint64 () const
	{
		return this->buffer_iterator::operator sint64();
	}

	///////////////////////////////////////////////////////////////////////////
	// direct assignements of strings to buffers should be avoided
	// if a fixed buffer scheme is necessary;
	// use str2buffer / buffer2str instead
	///////////////////////////////////////////////////////////////////////////
	const char* operator = (const char * c)
	{
		return this->buffer_iterator::operator=(c);
	}
	operator const char*() const
	{
		return this->buffer_iterator::operator const char*();
	}

	///////////////////////////////////////////////////////////////////////////
	// returning readable/writable and size checked portions of the buffer
	// that behave like a fixed buffer for inserting packets directly
	///////////////////////////////////////////////////////////////////////////
	buffer_iterator get_readbuffer(size_t sz)
	{
		if( checkread(sz) )
			return buffer_iterator(this->cRpp, sz);
		else
			return buffer_iterator();
	}
	buffer_iterator get_writebuffer(size_t sz)
	{
		if( checkwrite(sz) )
			return buffer_iterator(this->cWpp, sz);
		else
			return buffer_iterator();
	}
};


///////////////////////////////////////////////////////////////////////////////
// 
//
///////////////////////////////////////////////////////////////////////////////
class streamable
{
public:
	streamable()			{}
	virtual ~streamable()	{}

	buffer_iterator& operator=(buffer_iterator& bi)
	{
		if( bi.checkread(this->BufferSize()) )
			this->fromBuffer(bi);
		return bi;
	}

	//////////////////////////////////////////////////////////////////////
	// return necessary size of buffer
	virtual size_t BufferSize() const =0;
	//////////////////////////////////////////////////////////////////////
	// writes content to buffer
	virtual bool toBuffer(buffer_iterator& bi) const   = 0;
	//////////////////////////////////////////////////////////////////////
	// reads content from buffer
	virtual bool fromBuffer(const buffer_iterator& bi) = 0;
};



inline const streamable& buffer_iterator::operator = (const streamable& s)
{
	if( checkwrite(s.BufferSize()) )
		s.toBuffer(*this);
	return s;
}


NAMESPACE_END(basics)

#endif//__BASEBUFFER__
