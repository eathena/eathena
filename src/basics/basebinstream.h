#ifndef __BASEBINSTREAM_H__
#define __BASEBINSTREAM_H__

#include "basetypes.h"
#include "basefile.h"
#include "basestring.h"
#include "basemath.h"

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////


//## NOTE: nothing exported from main repository, 
// used a modified copy here, the export would have some more dependencies [Hinoko]
struct binstream
{
	virtual ~binstream()
	{}
private:
	virtual void intern_put(const unsigned char v) = 0;
	virtual int intern_get()=0;
	virtual int intern_peek()=0;
public:
	virtual size_t read(unsigned char* buffer, size_t sz)
	{
		unsigned char* p = buffer;
		int c = this->intern_get();
		for(; c!=EOF && sz; --sz, ++p)
		{
			*p = (unsigned char)c;
			c = this->intern_get();
		}
		return p-buffer;
	}
	virtual size_t write(const unsigned char* buffer, size_t sz)
	{
		const unsigned char* p = buffer;
		if(p)
		{
			for(; sz; --sz, ++p)
				this->intern_put(*p);
		}
		return p-buffer;
	}



	template<typename T>
	void put_integer(const T& v, bool_false)
	{	// cause compile error on non-buildin types
		void *add_your_own_stream_operator_for_this_type=v;
	}
	template<typename T>
	void put_integer(const T& v, bool_true)
	{	// put numbers 7bit encoded with sign sorted
		const T mask = (T)(~((T)0x3F));
		const bool is_signed = (v >> (sizeof(T)*NBBY-1));
		T x = (T)(is_signed?~v:v);
		T y = 0;
		int digit = 0;
		while( x&mask )
		{
			y = (T)((y<<7) | (x & 0x7F));
			x >>= 7;
			++digit;
		}
		// prepare first digit
		x &= 0x3F;
		if(is_signed)
			x |= 0x40;

		for(; digit; --digit)
		{
			this->intern_put( (unsigned char)x );
			x = (T)(y & 0x7F);
			y >>= 7;
		} 
		// output last digit
		this->intern_put( (unsigned char)(x | 0x80) );
	}

	template<typename T>
	void get_integer(T& v, bool_false)
	{	// cause compile error on non-buildin types
		void *add_your_own_stream_operator_for_this_type(v);
	}
	template<typename T>
	void get_integer(T& v, bool_true)
	{
		 // first digit
		 int c = this->intern_get();
		 if(c != EOF )
		 {
			 const bool is_signed = (c&0x40)!=0;
			 v = (T)(c&0x3F);
			 while( 0==(c&0x80) )
			 {
				 c = this->intern_get();
				 v = (T)((v<<7) | (c&0x7F));
			 }
			 if( is_signed )
				 v = (T)(~v);
		 }
		 else
			 v = 0;
	}

	template<typename T>
	void put(const T& v)
	{
		typedef typename is_integral<T>::Type is_int;
		this->put_integer(v, is_int());
	}
	void put(const unsigned char v)
	{
		this->intern_put(v);
	}	
	void put(const signed char v)
	{
		this->intern_put(v);
	}
	void put(const long double v)
	{
		int exponent;
		const long double m = math::ldexp( math::frexp(v, exponent), 64);	// IEEE 754 long double mantissa (64bits)
		//exponent+=3fff; // IEEE 754 long double exponent offset makes it non-negative
		const uint64 mantissa = (uint64)(math::floor(m));
		this->put(exponent);
		this->put(mantissa);
	}
	void put(const double v)
	{
		int exponent;
		const double m = math::ldexp( math::frexp(v, exponent), 53);	// IEEE 754 double mantissa (53bits)
		//exponent+=0x3ff; // IEEE 754 double exponent offset makes it non-negative
		const uint64 mantissa = (uint64)(math::floor(m));
		this->put(exponent);
		this->put(mantissa);
	}
	void put(const float v)
	{
		int exponent;
		const double m = math::ldexp( math::frexp(v, exponent), 24);	// IEEE 754 float mantissa (53bits)
		//exponent+=0x7f; // IEEE 754 float exponent offset makes it non-negative
		const unsigned long mantissa = (unsigned long)(math::floor(m));
		this->put(exponent);
		this->put(mantissa);
	}
	void put(const char* v)
	{
		if(v)
		{
			for(; *v; ++v)
			{
				this->intern_put(*v);
			}
			this->intern_put(0);
		}
	}
	void put(const string<>& v)
	{
		this->write((const unsigned char*)v.c_str(), 1+v.size());
	}

	template<typename T>
	void get(T& v)
	{
		typedef typename is_integral<T>::Type is_int;
		this->get_integer(v, is_int());
	}
	void get(unsigned char& v)
	{
		const int c = this->intern_get();
		if(c!=EOF)
			v = (unsigned char)c;
		else
			v = 0;
		//	throw
	}	
	void get(signed char& v)
	{
		const int c = this->intern_get();
		if(c!=EOF)
			v = (signed char)c;
		else
			v = 0;
		//	throw
	}
	void get(long double& v)
	{
		int exponent;
		uint64 mantissa;
		this->get(exponent);
		this->get(mantissa);
#if defined(_MSC_VER) && !defined(__ICL)
		// does not have uint64 to double conversion
		const unsigned long v0 = (unsigned long)(mantissa&0xFFFFFFFF);
		const unsigned long v1 = (unsigned long)(mantissa>>32);
		const long double m = math::ldexp( (long double)v1, 32) + v0;
#else
		const long double m = (long double)mantissa;
#endif
		v = math::ldexp(m, -64);		// IEEE 754 long double mantissa (53bits)
		v = math::ldexp(v, exponent); // IEEE 754 long double exponent
	}
	void get(double& v)
	{
		int exponent;
		uint64 mantissa;
		this->get(exponent);
		this->get(mantissa);
#if defined(_MSC_VER) && !defined(__ICL)
		// does not have uint64 to double conversion
		const unsigned long v0 = (unsigned long)(mantissa&0xFFFFFFFF);
		const unsigned long v1 = (unsigned long)(mantissa>>32);
		const double m = math::ldexp( (double)v1, 32) + v0;
#else
		const double m = (double)mantissa;
#endif
		v = math::ldexp(m, -53);	// IEEE 754 double mantissa (53bits)
		v = math::ldexp(v, exponent); // IEEE 754 double exponent
	}
	void get(float& v)
	{
		int exponent;
		unsigned long mantissa;
		this->get(exponent);
		this->get(mantissa);
		const double tmp = ldexp((double)mantissa, -24);	// IEEE 754 float mantissa (24bits)
		v = (float)math::ldexp(tmp, exponent); // IEEE 754 double exponent
	}
	void get(string<>& v)
	{
		int c = this->intern_get();
		v.clear();
		while( c && c!=EOF )
		{
			v << ((char)c);
			c = this->intern_get();
		}
	}
public:
	int get()
	{
		return intern_get();
	}
	int peek()
	{
		return intern_peek();
	}

};

template<typename T>
binstream& operator<<(binstream& s, const T& v)	{ s.put(v); return s; }
template<typename T>
binstream& operator>>(binstream& s, T& v)		{ s.get(v); return s; }
//## did not copy the specialization for standard types, added partially as put implements



struct binmemory : public binstream
{
	vector<unsigned char> mem;

	virtual ~binmemory()
	{}

	virtual void intern_put(const unsigned char v)
	{
		this->mem.push_back(v);
	}
	virtual int intern_get()
	{
		const unsigned int v = ( this->mem.size() ) ? this->mem[0] : EOF;
		this->mem.skip(1);
		return v;
	}
	virtual int intern_peek()
	{
		const unsigned int v = ( this->mem.size() ) ? this->mem[0] : EOF;
		return v;
	}
	virtual size_t read(unsigned char* buffer, size_t sz)
	{
		if(buffer)
		{
			if(sz>this->mem.size()) sz = this->mem.size();
			memcpy(buffer, this->mem.begin(), sz);
			mem.skip(sz);
			return sz;
		}
		return 0;
	}
	virtual size_t write(const unsigned char* buffer, size_t sz)
	{
		if(buffer)
		{
			this->mem.append(buffer,sz);
			return sz;
		}
		return 0;
	}
};

struct binaryfile : public CFile, public binstream
{
	binaryfile(const char* name, const char* mode)
		: CFile()
		, binstream()
	{
		this->open(name, mode);
	}

	virtual ~binaryfile()
	{}
	using binstream::put;
	//## NOTE none exported, use the modified copy here, the export would some more dependencies [Hinoko]

	virtual void intern_put(const unsigned char v)
	{
		if( this->is_open() )
		{
			putc(v, this->cFile);
		}
	}
	virtual int intern_get()
	{
		if( this->is_open() )
		{
			return getc(this->cFile);
		}
		return EOF;
	}
	virtual int intern_peek()
	{
		if( this->is_open() )
		{
			int c=getc(this->cFile);
			if(c!=EOF)
				fseek(this->cFile, -1, SEEK_CUR);
			return c;
		}
		return EOF;
	}
	virtual size_t read(unsigned char* buffer, size_t sz)
	{
		return fread(buffer,1,sz,this->cFile);
	}
	virtual size_t write(const unsigned char* buffer, size_t sz)
	{
		return fwrite(buffer,1,sz,this->cFile);
	}
};


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASEBINSTREAM_H__

