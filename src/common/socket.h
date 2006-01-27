// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_SOCKET_H_
#define _SOCKET_H_

#include "base.h"
#include "malloc.h"
#include "timer.h"


extern time_t last_tick;





///////////////////////////////////////////////////////////////////////////////
// predeclares
class streamable;








//////////////////////////////////////////////////////////////////////////
// uchar buffer access
//
// uses seperated read and write pointer; pointers do not go rounds, 
// so it needs an explicit sync at r/w operations
//
//////////////////////////////////////////////////////////////////////////
class buffer_iterator : public global
{
	friend class buffer;

protected:
	mutable unsigned char *rpp;	// read pointer inside the buffer
	mutable unsigned char *wpp;	// write pointer inside the buffer
	unsigned char *start;		// pointer to the start of the buffer
	unsigned char *end;			// pointer to the end of the buffer


	buffer_iterator()
		: rpp(NULL), wpp(NULL), start(NULL), end(NULL)
	{}

	// don't define a copy constructor
	buffer_iterator(const buffer_iterator& bi);
public:
	buffer_iterator(unsigned char* b, size_t sz)
	{
		this->rpp  =b;
		this->wpp  =b;
		this->start=b;
		this->end  =b+sz;
	}
	virtual ~buffer_iterator()
	{
		this->rpp=this->wpp=this->start=this->end=NULL;
	}

	// assignment
	const buffer_iterator& operator=(const buffer_iterator& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	// access functions
	bool eof() const		{ return !(this->wpp && this->wpp<this->end); }
	size_t size() const		{ return this->end-this->start; }
	size_t datasize() const	{ return this->wpp-this->rpp; }
	size_t freesize() const	{ return this->end-this->wpp; }


	///////////////////////////////////////////////////////////////////////////
	virtual bool checkwrite(size_t addsize)
	{
		if( this->wpp+addsize > this->end && this->start < this->rpp )
		{	// move the current buffer data when necessary and possible 
			memmove(this->start, this->rpp, this->wpp-this->rpp);
			this->wpp = this->start+(this->wpp-this->rpp);
			this->rpp = this->start;
		}
		return ( this->wpp && this->wpp+addsize<=this->end );
	}
	virtual bool checkread(size_t addsize) const
	{

		return ( this->rpp && this->rpp+addsize<=this->wpp );
	}

	///////////////////////////////////////////////////////////////////////////
	unsigned char operator = (const unsigned char ch)
	{
		if( checkwrite(1) )
		{
			*this->wpp++ = ch;
		}
		return ch;
	}
	char operator = (const char ch)
	{	
		if( checkwrite(1) )
		{
			*this->wpp++ = (unsigned char)ch;
		}
		return ch;
	}
	bool assign_char(const unsigned char ch)
	{
		if( checkwrite(1) )
		{
			*this->wpp++ = ch;
			return true;
		}
		return false;
	}
	bool assign_char(const char ch)
	{
		if( checkwrite(1) )
		{
			*this->wpp++ = (unsigned char)ch;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	operator unsigned char () const
	{	
		if( checkread(1) )
			return *this->rpp++;
		return 0;
	}
	operator char() const
	{	
		if( checkread(1) )
			return *this->rpp++;
		return 0;

	}
	///////////////////////////////////////////////////////////////////////////
	unsigned short operator = (const unsigned short sr)
	{	// implement little endian buffer format
		if( checkwrite(2) )
		{
			*this->wpp++ = (unsigned char)(sr         ); 
			*this->wpp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	short operator = (const short sr)
	{	// implement little endian buffer format
		if( checkwrite(2) )
		{
			*this->wpp++ = (unsigned char)(sr         ); 
			*this->wpp++ = (unsigned char)(sr  >> 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned short() const
	{	// implement little endian buffer format
		unsigned short sr=0;
		if( checkread(2) )
		{	
			sr  = ((unsigned short)(*this->rpp++)        ); 
			sr |= ((unsigned short)(*this->rpp++) << 0x08);
		}
		return sr;
	}
	operator short () const
	{	// implement little endian buffer format
		short sr=0;
		if( checkread(2) )
		{	
			sr  = ((unsigned short)(*this->rpp++)        ); 
			sr |= ((unsigned short)(*this->rpp++) << 0x08);
		}
		return sr;
	}
	///////////////////////////////////////////////////////////////////////////
	uint32 operator = (const uint32 ln)
	{	// implement little endian buffer format
		if( checkwrite(4) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	sint32 operator = (const sint32 ln)
	{	// implement little endian buffer format
		if( checkwrite(4) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	operator uint32 () const
	{	// implement little endian buffer format
		unsigned long ln=0;
		if( checkread(4) )
		{	
			ln  = ((uint32)(*this->rpp++)        ); 
			ln |= ((uint32)(*this->rpp++) << 0x08);
			ln |= ((uint32)(*this->rpp++) << 0x10);
			ln |= ((uint32)(*this->rpp++) << 0x18);
		}
		return ln;
	}
	operator sint32 () const
	{	// implement little endian buffer format
		long ln=0;
		if( checkread(4) )
		{	
			ln  = ((uint32)(*this->rpp++)        ); 
			ln |= ((uint32)(*this->rpp++) << 0x08);
			ln |= ((uint32)(*this->rpp++) << 0x10);
			ln |= ((uint32)(*this->rpp++) << 0x18);
		}
		return ln;
	}
// 64bit unix defines long/ulong as 64bit
#if (defined __64BIT__)
	///////////////////////////////////////////////////////////////////////////
	unsigned long operator = (const unsigned long ln)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
			*this->wpp++ = (unsigned char)(ln  >> 0x20 );
			*this->wpp++ = (unsigned char)(ln  >> 0x28 );
			*this->wpp++ = (unsigned char)(ln  >> 0x30 );
			*this->wpp++ = (unsigned char)(ln  >> 0x38 );
		}
		return ln;
	}
	long operator = (const long ln)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(ln          );
			*this->wpp++ = (unsigned char)(ln  >> 0x08 );
			*this->wpp++ = (unsigned char)(ln  >> 0x10 );
			*this->wpp++ = (unsigned char)(ln  >> 0x18 );
			*this->wpp++ = (unsigned char)(ln  >> 0x20 );
			*this->wpp++ = (unsigned char)(ln  >> 0x28 );
			*this->wpp++ = (unsigned char)(ln  >> 0x30 );
			*this->wpp++ = (unsigned char)(ln  >> 0x38 );
		}
		return ln;
	}
	///////////////////////////////////////////////////////////////////////////
	operator unsigned long () const
	{	// implement little endian buffer format
		unsigned long lx=0;
		if( checkread(8) )
		{	
			lx  = ((unsigned long)(*this->rpp++)        ); 
			lx |= ((unsigned long)(*this->rpp++) << 0x08);
			lx |= ((unsigned long)(*this->rpp++) << 0x10);
			lx |= ((unsigned long)(*this->rpp++) << 0x18);
			lx |= ((unsigned long)(*this->rpp++) << 0x20);
			lx |= ((unsigned long)(*this->rpp++) << 0x28);
			lx |= ((unsigned long)(*this->rpp++) << 0x30);
			lx |= ((unsigned long)(*this->rpp++) << 0x38);
		}
		return lx;
	}
	operator long () const
	{	// implement little endian buffer format
		long lx=0;
		if( checkread(8) )
		{	
			lx  = ((unsigned long)(*this->rpp++)        ); 
			lx |= ((unsigned long)(*this->rpp++) << 0x08);
			lx |= ((unsigned long)(*this->rpp++) << 0x10);
			lx |= ((unsigned long)(*this->rpp++) << 0x18);
			lx |= ((unsigned long)(*this->rpp++) << 0x20);
			lx |= ((unsigned long)(*this->rpp++) << 0x28);
			lx |= ((unsigned long)(*this->rpp++) << 0x30);
			lx |= ((unsigned long)(*this->rpp++) << 0x38);
		}
		return lx;
	}
#endif
	///////////////////////////////////////////////////////////////////////////
	ipaddress operator = (const ipaddress ip)
	{	// implement little endian buffer format
		// IPs are given in network byte order to the buffer
		if( checkwrite(4) )
		{
			*this->wpp++ = (unsigned char)(ip[3]);
			*this->wpp++ = (unsigned char)(ip[2]);
			*this->wpp++ = (unsigned char)(ip[1]);
			*this->wpp++ = (unsigned char)(ip[0]);
		}
		return ip;
	}
	///////////////////////////////////////////////////////////////////////////
	operator ipaddress () const
	{	// implement little endian buffer format
		if( checkread(4) )
		{
			ipaddress ip( this->rpp[0],this->rpp[1],this->rpp[2],this->rpp[3] );
			this->rpp+=4;
			return  ip;
		}
		return INADDR_ANY;
	}
	///////////////////////////////////////////////////////////////////////////
	sint64 operator = (const sint64 lx)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(lx          );
			*this->wpp++ = (unsigned char)(lx  >> 0x08 );
			*this->wpp++ = (unsigned char)(lx  >> 0x10 );
			*this->wpp++ = (unsigned char)(lx  >> 0x18 );
			*this->wpp++ = (unsigned char)(lx  >> 0x20 );
			*this->wpp++ = (unsigned char)(lx  >> 0x28 );
			*this->wpp++ = (unsigned char)(lx  >> 0x30 );
			*this->wpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	uint64 operator = (const uint64 lx)
	{	// implement little endian buffer format
		if( checkwrite(8) )
		{
			*this->wpp++ = (unsigned char)(lx          );
			*this->wpp++ = (unsigned char)(lx  >> 0x08 );
			*this->wpp++ = (unsigned char)(lx  >> 0x10 );
			*this->wpp++ = (unsigned char)(lx  >> 0x18 );
			*this->wpp++ = (unsigned char)(lx  >> 0x20 );
			*this->wpp++ = (unsigned char)(lx  >> 0x28 );
			*this->wpp++ = (unsigned char)(lx  >> 0x30 );
			*this->wpp++ = (unsigned char)(lx  >> 0x38 );
		}
		return lx;
	}
	///////////////////////////////////////////////////////////////////////////
	operator sint64 () const
	{	// implement little endian buffer format
		sint64 lx=0;
		if( checkread(8) )
		{	
			lx  = ((uint64)(*this->rpp++)        ); 
			lx |= ((uint64)(*this->rpp++) << 0x08);
			lx |= ((uint64)(*this->rpp++) << 0x10);
			lx |= ((uint64)(*this->rpp++) << 0x18);
			lx |= ((uint64)(*this->rpp++) << 0x20);
			lx |= ((uint64)(*this->rpp++) << 0x28);
			lx |= ((uint64)(*this->rpp++) << 0x30);
			lx |= ((uint64)(*this->rpp++) << 0x38);
		}
		return lx;
	}
	operator uint64 () const
	{	// implement little endian buffer format
		uint64 lx=0;
		if( checkread(8) )
		{	
			lx  = ((uint64)(*this->rpp++)        ); 
			lx |= ((uint64)(*this->rpp++) << 0x08);
			lx |= ((uint64)(*this->rpp++) << 0x10);
			lx |= ((uint64)(*this->rpp++) << 0x18);
			lx |= ((uint64)(*this->rpp++) << 0x20);
			lx |= ((uint64)(*this->rpp++) << 0x28);
			lx |= ((uint64)(*this->rpp++) << 0x30);
			lx |= ((uint64)(*this->rpp++) << 0x38);
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
		size_t sz= (c) ? strlen(c)+1 : 1;
		if( checkwrite(sz) )
		{
			if( c )
			{
				memcpy(this->wpp, c, sz);
				this->wpp+=sz;
			}
			else
				*this->wpp++=0;
		}
		return c;
	}
	operator const char*() const
	{	// find the EOS
		// cannot do anything else then going through the array
		unsigned char *ix = this->rpp;
		while( this->rpp<this->wpp && this->rpp );
		if(this->rpp<this->wpp)
		{	// skip the eos in the buffer
			// it belongs to the string
			this->rpp++;
			return (char*)ix;
		}
		else
		{	// no eos, so there is no string
			this->rpp = ix;
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
				size_t cpsz = ( cpsz > strlen(c)+1 ) ? sz : strlen(c)+1;
				memcpy(this->wpp, c, cpsz);
				this->wpp[cpsz-1] = 0;	// force an EOS
			}
			else
				memset(this->wpp, 0, sz);
			this->wpp+=sz;
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
				memcpy(c, this->rpp, sz);
				c[sz-1] = 0;	// force an EOS
				this->rpp+=sz;
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
				memcpy(this->wpp, c, sz);
			this->wpp+=sz;
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
				memcpy(c, this->rpp, sz);
				this->rpp+=sz;
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
		return this->rpp+offset; 
	}
	unsigned char* get_wptr(size_t offset=0, size_t size=0)
	{
		if( !checkwrite(offset+size) )
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("Buffer GetWritepointer out of bound");
#else
			return NULL;
#endif
		return this->wpp+offset; 
	}
	bool step_rptr(int i)
	{	// can go backwards with negative offset
		if(this->rpp+i <= this->wpp && this->rpp+i >= this->start)
		{
			this->rpp+=i;
			return true;
		}
		return false;
	}
	bool step_wptr(int i)
	{	// can go backwards with negative offset
		if(this->wpp+i <= this->end && this->wpp+i >= this->rpp)
		{
			this->wpp+=i;
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
			return buffer_iterator(this->rpp, sz);
		else
			return buffer_iterator();
	}
	buffer_iterator get_writebuffer(size_t sz)
	{
		if( checkwrite(sz) )
			return buffer_iterator(this->wpp, sz);
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
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer_fixed& operator=(const buffer_fixed& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	// baseclass copy/assignment
	buffer_fixed(const buffer_iterator& bi) : buffer_iterator()
	{
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer_fixed& operator=(const buffer_iterator& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
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
// 64bit unix defines long/ulong as 64bit
#if (defined __64BIT__)
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
#endif
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

class buffer : public buffer_iterator
{
public:
	buffer() : buffer_iterator()
	{}
	virtual ~buffer()
	{
		if(this->start) delete[] this->start;
		this->rpp=this->wpp=this->start=this->end=NULL;
	}

	// copy/assignment
	buffer(const buffer& bi) : buffer_iterator()
	{
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer& operator=(const buffer& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	// baseclass copy/assignment
	buffer(const buffer_iterator& bi) : buffer_iterator()
	{
		write(bi.rpp, bi.wpp-bi.rpp);
	}
	const buffer& operator=(const buffer_iterator& bi)
	{
		write(bi.rpp, bi.wpp-bi.rpp);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	virtual bool checkwrite(size_t addsize)
	{
		static const size_t quant1 = (size_t)(   64);
		static const size_t qmask1 = (size_t)(  ~63);
		static const size_t quant2 = (size_t)( 4096);
		static const size_t qmask2 = (size_t)(~4095);

		size_t sz = this->wpp-this->rpp + addsize;

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
				memcpy(tmp, this->rpp, this->wpp-this->rpp);
				delete[] this->start;
			}
			this->end = tmp+sz;
			this->wpp = tmp+(this->wpp-this->rpp);
			this->rpp = tmp;
			this->start = tmp;
		}
		else if( this->wpp+addsize > this->end )
		{	// moving the current buffer data is sufficient
			memmove(this->start, this->rpp, this->wpp-this->rpp);
			this->wpp = this->start+(this->wpp-this->rpp);
			this->rpp = this->start;
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
// 64bit unix defines long/ulong as 64bit
#if (defined __64BIT__)
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
#endif
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
			return buffer_iterator(this->rpp, sz);
		else
			return buffer_iterator();
	}
	buffer_iterator get_writebuffer(size_t sz)
	{
		if( checkwrite(sz) )
			return buffer_iterator(this->wpp, sz);
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


/*
extern inline void buffer_test()
{
	try
	{
		size_t i;

		buffer ba;
		unsigned char buf[16];
		buffer_iterator bb(buf, sizeof(buf));
		
		ba = (unsigned long)1;

		for(i=0; i<5; i++)
			bb = (unsigned short)i;

		printf("buffer\n");
		for(i=0; i<sizeof(buf); i++)
			printf("%02x ", buf[i]);
		printf("\n");

		for(i=5; i<10; i++)
			printf("%02x ", (unsigned short)bb);
		printf("\n");

		printf("buffer\n");
		for(i=0; i<sizeof(buf); i++)
			printf("%02x ", buf[i]);
		printf("\n");


		for(i=5; i<10; i++)
			bb = (unsigned short)i;

		printf("buffer\n");
		for(i=0; i<sizeof(buf); i++)
			printf("%02x ", buf[i]);
		printf("\n");

		for(i=0; i<10; i++)
			printf("%i", (unsigned short)bb);
		printf("\n");
	
	}
	catch( CException e )
	{
		printf((const char*)e);
	}
}
*/






// Class for assigning/reading words from a buffer
class objW
{
	unsigned char *ip;
public:
	objW():ip(NULL)												{}
	objW(unsigned char* x):ip(x)								{}
	objW(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objW(char* x):ip((unsigned char*)x)							{}
	objW(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objW& init(unsigned char* x)			{ip=x;return *this;}
	objW& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objW& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objW& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}

	operator unsigned short() const
	{	return this->operator()();
	}
	unsigned short operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned short)(ip[0]))        )
					|( ((unsigned short)(ip[1])) << 0x08);
			
		}
		return 0;
	}

	objW& operator=(const objW& objw)
	{
		if(ip && objw.ip)
		{	
			memcpy(ip, objw.ip, 2);
		}
		return *this;
	}
	unsigned short operator=(unsigned short valin)
	{	
		if(ip)
		{
			ip[0] = (unsigned char)((valin & 0x00FF)          );
			ip[1] = (unsigned char)((valin & 0xFF00)  >> 0x08 );
		}
		return valin;
	}
};

// Class for assigning/reading dwords from a buffer
class objL
{
	unsigned char *ip;
public:
	objL():ip(NULL)												{}
	objL(unsigned char* x):ip(x)								{}
	objL(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objL(char* x):ip((unsigned char*)x)							{}
	objL(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objL& init(unsigned char* x)			{ip=x;return *this;}
	objL& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objL& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objL& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}


	operator unsigned long() const
	{	return this->operator()();
	}
	unsigned long operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned long)(ip[0]))        )
					|( ((unsigned long)(ip[1])) << 0x08)
					|( ((unsigned long)(ip[2])) << 0x10)
					|( ((unsigned long)(ip[3])) << 0x18);
			
		}
		return 0;
	}

	objL& operator=(const objL& objl)
	{
		if(ip && objl.ip)
		{	
			memcpy(ip, objl.ip, 4);
		}
		return *this;
	}
	unsigned long operator=(unsigned long valin)
	{	
		if(ip)
		{
			ip[0] = (unsigned char)((valin & 0x000000FF)          );
			ip[1] = (unsigned char)((valin & 0x0000FF00)  >> 0x08 );
			ip[2] = (unsigned char)((valin & 0x00FF0000)  >> 0x10 );
			ip[3] = (unsigned char)((valin & 0xFF000000)  >> 0x18 );
		}
		return valin;
	}
};

// Class for assigning/reading words from a buffer 
// without changing byte order
// necessary for IP numbers, which are always in network byte order
// it is implemented this way and I currently won't interfere with that
// even if it is quite questionable

// changes: 
// transfered all IP addresses in the programms to host byte order
// since IPs are transfered in network byte order
// we cannot use objL but its complementary here
class objLIP	
{
	unsigned char *ip;
public:
	objLIP():ip(NULL)											{}
	objLIP(unsigned char* x):ip(x)								{}
	objLIP(unsigned char* x,int pos):ip(x?x+pos:NULL)			{}
	objLIP(char* x):ip((unsigned char*)x)						{}
	objLIP(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objLIP& init(unsigned char* x)			{ip=x;return *this;}
	objLIP& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objLIP& init(char* x)					{ip=(unsigned char*)x;return *this;}
	objLIP& init(char* x,int pos)			{if(x) ip=(unsigned char*)(x+pos);return *this;}

	
	operator ipaddress() const
	{
		return this->operator()();
	}
	ipaddress operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned long)(ip[3]))        )
					|( ((unsigned long)(ip[2])) << 0x08)
					|( ((unsigned long)(ip[1])) << 0x10)
					|( ((unsigned long)(ip[0])) << 0x18);


		}
		return INADDR_ANY;
	}
	objLIP& operator=(const objLIP& objl)
	{
		if(ip && objl.ip)
		{	
			memcpy(ip, objl.ip, 4);
		}
		return *this;
	}
	ipaddress operator=(ipaddress valin)
	{	
		if(ip)
		{
			ip[3] = (unsigned char)((valin & 0x000000FF)          );
			ip[2] = (unsigned char)((valin & 0x0000FF00)  >> 0x08 );
			ip[1] = (unsigned char)((valin & 0x00FF0000)  >> 0x10 );
			ip[0] = (unsigned char)((valin & 0xFF000000)  >> 0x18 );
		}
		return valin;
	}
};

class objB
{
	unsigned char *ip;
public:
	objB():ip(NULL)												{}
	objB(unsigned char* x):ip(x)								{}
	objB(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objB(char* x):ip((unsigned char*)x)							{}
	objB(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objB& init(unsigned char* x)			{ip=x;return *this;}
	objB& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objB& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objB& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}

	operator unsigned char() const
	{	return this->operator()();
	}
	unsigned char operator()()	const
	{
		if(ip)
		{	
			return ip[0];
		}
		return 0;
	}
	objB& operator=(const objB& objb)
	{
		if(ip && objb.ip)
		{	
			ip[0] = objb.ip[0];
		}
		return *this;
	}
	unsigned char operator=(unsigned char valin)
	{	
		if(ip)
		{
			ip[0] = valin;
		}
		return valin;
	}
};



// define declaration

#define RFIFOP(fd,pos) ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL))
#define RFIFOB(fd,pos) (objB( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOW(fd,pos) (objW( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOL(fd,pos) (objL( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOLIP(fd,pos) (objLIP( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) ))
#define RFIFOREST(fd) ((int)((session[fd]&&session[fd]->rdata)?(session[fd]->rdata_size-session[fd]->rdata_pos):0))
#define RFIFOSPACE(fd) ((int)((session[fd]&&session[fd]->rdata)?(session[fd]->rdata_max-session[fd]->rdata_size):0))
#define RBUFP(p,pos) (((unsigned char*)(p))+(pos))
#define RBUFB(p,pos) (*((unsigned char*)RBUFP((p),(pos))))
#define RBUFW(p,pos) (objW((p),(pos)))
#define RBUFL(p,pos) (objL((p),(pos)))
#define RBUFLIP(p,pos) (objLIP((p),(pos)))

#define WFIFOSPACE(fd) ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata_max-session[fd]->wdata_size):0)

#define WFIFOP(fd,pos) ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL))
#define WFIFOB(fd,pos) (objB( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WFIFOW(fd,pos) (objW( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WFIFOL(fd,pos) (objL( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WFIFOLIP(fd,pos) (objLIP( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) ))
#define WBUFP(p,pos) (((unsigned char*)(p))+(pos))
#define WBUFB(p,pos) (*((unsigned char*)WBUFP((p),(pos))))
#define WBUFW(p,pos) (objW((p),(pos)))
#define WBUFL(p,pos) (objL((p),(pos)))
#define WBUFLIP(p,pos) (objLIP((p),(pos)))


// Struct declaration
struct socket_data
{
	struct {
		bool connected : 1;		// true when connected
		bool remove : 1;		// true when to be removed
		bool marked : 1;		// true when deleayed removal is initiated (optional)
	}flag;

	time_t rdata_tick;			// tick of last read

	buffer rbuffer;
	buffer wbuffer;

	unsigned char *rdata;		// buffer
	size_t rdata_max;			// size of buffer
	size_t rdata_size;			// size of data
	size_t rdata_pos;			// iterator within data

	unsigned char *wdata;		// buffer
	size_t wdata_max;			// size of buffer
	size_t wdata_size;			// size of data

	ipaddress client_ip;	// just an ip in host byte order is enough (4byte instead of 16)

	int (*func_recv)(int);
	int (*func_send)(int);
	int (*func_parse)(int);
	int (*func_term)(int);
	int (*func_console)(char*);

	void* session_data;
};

// Data prototype declaration

extern struct socket_data *session[FD_SETSIZE];
extern size_t fd_max;


static inline bool session_isValid(int fd)
{
	return ( (fd>=0) && (fd<FD_SETSIZE) && (NULL!=session[fd]) );
}
static inline bool session_isActive(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.connected );
}	
	
static inline bool session_isRemoved(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.remove );
}
static inline bool session_isMarked(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.marked );
}
static inline bool session_Remove(int fd)
{	// force removal
	if( session_isValid(fd)	)
	{
		session[fd]->flag.connected = false;
		session[fd]->flag.marked	= false;
		session[fd]->flag.remove	= true;
	}
	return true;
}
static inline bool session_SetTermFunction(int fd, int (*term)(int))
{
	if( session_isValid(fd) )
		session[fd]->func_term = term;
	return true;
}

bool session_SetWaitClose(int fd, unsigned long timeoffset);
bool session_Delete(int fd);

// Function prototype declaration

int make_listen    (unsigned long ip, unsigned short port);
int make_connection(unsigned long ip, unsigned short port);

int realloc_fifo(int fd, size_t rfifo_size,size_t wfifo_size);

int WFIFOSET(int fd, size_t len);
int RFIFOSKIP(int fd, size_t len);

int do_sendrecv(int next);
void do_final_socket(void);

void socket_init(void);
void socket_final(void);

void flush_fifos();

int start_console(void);

void set_defaultparse(int (*defaultparse)(int));
void set_defaultconsoleparse(int (*defaultparse)(char*));

#endif	// _SOCKET_H_
