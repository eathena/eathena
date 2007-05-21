#ifndef __BASEPACKET_H__
#define __BASEPACKET_H__

#include "basetypes.h"
#include "basearray.h"
#include "basestring.h"


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////

/*

	<< Packets >>

Concept:
	A packet has an internal buffer with data and a set of fields.
	Every field of the packet is a public object in the packet.
	These objects point to their corresponding offsets in the field buffer.

	The number of fields is fixed by design.
	When there is a dynamic number of fields in a packet, they are grouped 
	together as a dynamic field.
	ex: an array of inventory items

	The packet contains one or more internal field buffers and, if required, 
	a field buffer controller.

	Resizing a dynamic field:
	- the data of the field is being changed so the field packs it's data if necessary
	  ex: removing an entry in the middle of an array
	- the field requests the internal IFieldBuffer to resize
	- IFieldBuffer applies restrictions to the size and requests IFieldBufferController to resize
	- IFieldBufferController applies more restrictions to the size and resizes IFieldBuffer
	- the field does final changes to the data

              ,---.  ,---.  ,---.  ,-----.  ,-----.  ,-----.
,-------------| B |--| W |--| L |--| Str |--| Arr |--| ... |--.
| IPacket     `---'  `---'  `---'  `-----'  `-----'  `-----'  |
|               /      /      /       /        /        /     |
|          ,.../....../....../......./......../......../      |
|          |                                                  |
|         \|/                                                 |
|          `                                                  |
|   ,--------------.      _                                   |
|   | ,--------------.  .:....................                |
|   `-| ,--------------. `-                  |                |
|     `-| IFieldBuffer |                     |                |
|       `--------------'                    \|/               |
|              |                             `                |
|              |                 ,------------------------.   |
|              |                 | IFieldBufferController |   |
|             \|/                `------------------------'   |
|              `                                              |
|  ,-------------------------------------------------------.  |
|  | internal buffer                                       |  |
|  `-------------------------------------------------------'  |
`-------------------------------------------------------------'

	IPacket - public packet interface

	B,W,L,Str,Arr,... - public fields, linked internally to one or more field buffers

	IFieldBuffer - updatable buffer used internally by the public fields

	IFieldBufferController - updates and controls the field buffers
		Handles internal buffer changes like reallocating or resizing.
		It is invoked by an IFieldBuffer when it wants to resize.
		Only needed for dynamic packets/fields.

	internal buffer - where the data is actually stored
		It is probably owned by the buffer controller.

*/



/// test packet functions
void test_packet(void);

/// The client can handle packet sizes up to 20480, 20k.
#define CLIENTPACKET_MAX 20480






///////////////////////////////////////////////////////////////////////////////
// Abstract base definitions
///////////////////////////////////////////////////////////////////////////////





/// Packet
class IPacket
{
public:
	virtual ~IPacket()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// Returns the length of this packet.
	virtual size_t length() const=0;

	/// Returns the data of this packet.
	virtual const uint8* buffer() const=0;
};



/// Field buffer
class IFieldBuffer
{
public:
	virtual ~IFieldBuffer()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// Returns the length of this buffer.
	virtual size_t length() const=0;

	/// Returns the data of this buffer.
	virtual uint8* buffer() const=0;

	/// Tries to resize the buffer, applying restrictions to the size.
	/// There is no warranty that the length will change to the requested 
	/// value or change at all.
	/// Returns if the buffer size changed.
	virtual bool do_resize(size_t sz)=0;

	/// Sets the length of the buffer
	virtual void length(size_t sz)=0;

	/// Sets the data of this buffer.
	virtual void buffer(uint8* buf)=0;
};



/// FieldBuffer controler
class IFieldBufferController
{
public:
	virtual ~IFieldBufferController()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// Changes the size of the target buffer. There is no warranty that the 
	/// size will change to the requested value or change at all.
	/// Returns if the buffer size changed.
	virtual bool do_resize(IFieldBuffer& buf, size_t new_len)=0;
};





///////////////////////////////////////////////////////////////////////////////
// Fixed-size packets and fields
///////////////////////////////////////////////////////////////////////////////





/// Field buffer with a fixed internal buffer.
/// Does not support resizing or being managed by a field buffer controller.
///
/// @param SZ Size of the buffer
template<size_t SZ>
class CBufFieldBuffer : public IFieldBuffer
{
public:
	CBufFieldBuffer();

public:
	virtual ~CBufFieldBuffer()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// Returns the length of this buffer.
	virtual size_t length() const;

	/// Returns the data of this buffer.
	virtual uint8* buffer() const;

	/// Returns false.
	/// Resizing is not supported.
	virtual bool do_resize(size_t sz);

	/// Does nothing.
	/// Resizing is not supported.
	virtual void length(size_t sz);

	/// Does nothing.
	/// An internal buffer is always used.
	virtual void buffer(uint8* buf);

private:
	/// Internal buffer
	uint8 _buf[SZ];
};



/// Fixed-size packet.
///
/// @param SZ Size of the buffer
template<size_t SZ=CLIENTPACKET_MAX>
class CFixPacket : public IPacket
{
protected:
	CFixPacket();
	CFixPacket(uint8* buf, size_t sz=SZ);
public:
	virtual ~CFixPacket();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the length of this packet.
	virtual size_t length() const;

	/// Returns the data of this packet.
	virtual const uint8* buffer() const;

protected:
	/// Packet data
	CBufFieldBuffer<SZ> _buf;
};



/// Byte field
class CFieldB
{
public:
	CFieldB(IFieldBuffer& buf, size_t off=0);

	operator uint8() const;
	uint8 operator()() const;

	CFieldB& operator=(const CFieldB& f);
	uint8 operator=(uint8 val);

private:
	IFieldBuffer& _buf;
	size_t _off;
};



/// Word field
class CFieldW
{
public:
	CFieldW(IFieldBuffer& buf, size_t off=0);

	operator uint16() const;
	uint16 operator()() const;

	CFieldW& operator=(const CFieldW& f);
	uint16 operator=(uint16 val);

private:
	IFieldBuffer& _buf;
	size_t _off;
};



/// Long field
class CFieldL
{
public:
	CFieldL(IFieldBuffer& buf, size_t off=0);

	operator uint32() const;
	uint32 operator()() const;

	CFieldL& operator=(const CFieldL& l);
	uint32 operator=(uint32 val);

private:
	IFieldBuffer& _buf;
	size_t _off;
};



/// Fixed-size string
///
/// @param SZ Size of the string
template<size_t SZ>
class CFieldStringFix
{
public:
	CFieldStringFix(IFieldBuffer& buf, size_t off=0);

	/// Value of the string
	operator const char*();
	const char* operator()() const;

	/// Length of the string.
	size_t length() const;

	/// Size of this field
	size_t capacity() const;

	CFieldStringFix<SZ>& operator=(const CFieldStringFix<SZ>& str);
	const char* operator=(const char* str);

private:
	IFieldBuffer& _buf;
	size_t _off;
};



/// Static array of fixed-size fields
///
/// @param T Type of field with a (IFieldBuffer& buf, size_t off) constructor
/// @param TSZ Size of each field
/// @param LEN Size of the array (number of sub-fields)
template<class T, size_t TSZ, size_t LEN>
class CFieldStaticArrayFix
{
public:
	CFieldStaticArrayFix(IFieldBuffer& buf, size_t off=0);

	/// Returns the element at the target index.
	T operator[](size_t idx);

	/// Returns the number of elements in this array.
	size_t length() const;

private:
	IFieldBuffer& _buf;
	size_t _off;
};





///////////////////////////////////////////////////////////////////////////////
// Dynamic packets and fields
///////////////////////////////////////////////////////////////////////////////





//## TODO





///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////






#include "baseinet.h"
#include "basearray.h"

///////////////////////////////////////////////////////////////////////////////
// example code from caldon, adopted version provided by Hinoko
// I just put it here as reference; 
// as the implementation above is almost identical it might be not 
// necessary to have adoptions, but maybe the question with the 
// dynamic buffer can be answered by example here

namespace example_socket {


///////////////////////////////////////////////////////////////////////////////
/// base class for accessing default types.
/// empty to cause compile time errors
template <typename T>
struct subscript_typed
{};

///////////////////////////////////////////////////////////////////////////////
/// type access specialisation for bytes.
template <>
struct subscript_typed<uint8>
{
	uint8* cWpp;
	subscript_typed(uint8* p) : cWpp(p)
	{}
	operator uint8() const
	{
		return (cWpp)?*cWpp:0;
	}
	const subscript_typed& operator=(uint8 v)
	{
		if(cWpp)
			*cWpp = v;
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// type access specialisation for words.
/// transfer in little endian byte order
template <>
struct subscript_typed<uint16>
{
	uint8* cWpp;
	subscript_typed(uint8* p) : cWpp(p)
	{}
	
	operator uint16() const
	{
		if(cWpp)
		{
			return	(uint16)
					 ( ((uint16)(cWpp[0]))        )
					|( ((uint16)(cWpp[1])) << 0x08);
		}
		return 0;
	}
	const subscript_typed& operator=(uint16 v)
	{
		if(cWpp)
		{
			uint8*p=cWpp;
			*p++ = (unsigned char)(v); v>>=8;
			*p   = (unsigned char)(v);
		}
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// type access specialisation for doublewords.
/// transfer in little endian byte order
template <>
struct subscript_typed<uint32>
{
	uint8* cWpp;
	subscript_typed(uint8* p) : cWpp(p)
	{}
	
	operator uint32() const
	{
		if(cWpp)
		{
			return	 ( ((uint32)(cWpp[0]))        )
					|( ((uint32)(cWpp[1])) << 0x08)
					|( ((uint32)(cWpp[2])) << 0x10)
					|( ((uint32)(cWpp[3])) << 0x18);
		}
		return 0;
	}
	const subscript_typed& operator=(uint32 v)
	{
		if(cWpp)
		{
			uint8*p=cWpp;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p   = (unsigned char)(v);
		}
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// type access specialisation for quadwords.
/// transfer in little endian byte order
template <>
struct subscript_typed<uint64>
{
	uint8* cWpp;
	subscript_typed(uint8* p) : cWpp(p)
	{}
	operator uint64() const
	{
		if(cWpp)
		{
			return	 ( ((uint64)(cWpp[0]))        )
					|( ((uint64)(cWpp[1])) << 0x08)
					|( ((uint64)(cWpp[2])) << 0x10)
					|( ((uint64)(cWpp[3])) << 0x18)
					|( ((uint64)(cWpp[4])) << 0x20)
					|( ((uint64)(cWpp[5])) << 0x28)
					|( ((uint64)(cWpp[6])) << 0x30)
					|( ((uint64)(cWpp[7])) << 0x38);
		}
		return 0;
	}
	const subscript_typed& operator=(uint64 v)
	{
		if(cWpp)
		{
			uint8*p=cWpp;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p++ = (unsigned char)(v); v>>=8;
			*p   = (unsigned char)(v);
		}
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// type access specialisation for cstring.
/// does never return null pointer
template <>
struct subscript_typed<const char*>
{
	uint8* cWpp;
	size_t sz;
	subscript_typed(uint8* p, size_t s) : cWpp(p), sz(s)
	{}
	operator const char*() const
	{
		if(cWpp&&sz)
		{
			cWpp[sz-1]=0;// force eos
			return	(const char*)(cWpp);
		}
		return "";
	}
	const subscript_typed& operator=(const char* v)
	{
		if(cWpp&&v&&sz)
		{
			memcpy(cWpp, v, sz);
			cWpp[sz-1]=0;
		}
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// type access specialisation for byte buffers.
/// returns null pointer when accessing out-of-bound
template <>
struct subscript_typed<const unsigned char*>
{
	uint8* cWpp;
	size_t sz;
	subscript_typed(uint8* p, size_t s) : cWpp(p), sz(s)
	{}
	operator const unsigned char*() const
	{
		return (const unsigned char*)cWpp;
	}
	const subscript_typed& operator=(const char* v)
	{
		if(cWpp&&v&&sz)
			memcpy(cWpp, v, sz);
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// type access specialisation for basics::ipaddess.
/// transfer ipaddresses as big endian (network byte order)
template <>
struct subscript_typed<basics::ipaddress>
{
	uint8* cWpp;
	subscript_typed(uint8* p) : cWpp(p)
	{}
	operator basics::ipaddress() const
	{
		if(cWpp)
		{
			return	 ( ((unsigned long)(cWpp[3]))        )
					|( ((unsigned long)(cWpp[2])) << 0x08)
					|( ((unsigned long)(cWpp[1])) << 0x10)
					|( ((unsigned long)(cWpp[0])) << 0x18);
		}
		return basics::ipaddress();
	}
	const subscript_typed& operator=(const basics::ipaddress& v)
	{
		if(cWpp)
		{
			cWpp[3] = (unsigned char)((v & 0x000000FF)          );
			cWpp[2] = (unsigned char)((v & 0x0000FF00)  >> 0x08 );
			cWpp[1] = (unsigned char)((v & 0x00FF0000)  >> 0x10 );
			cWpp[0] = (unsigned char)((v & 0xFF000000)  >> 0x18 );
		}
		return *this;
	}
};


///////////////////////////////////////////////////////////////////////////////
/// abstract subscript type.
/// handles type specification for accessing the parent object 
/// by aquiring a piece of buffer space inside the parent
template <typename Parent>
struct subscript
{
	Parent& parent;
	size_t  index;
	subscript(Parent& p, size_t inx) : parent(p),index(inx)
	{}
	~subscript()
	{}
	subscript_typed<uint8> u8()
	{
		return subscript_typed<uint8>(parent.aquire(index,1));
	}
	subscript_typed<uint16> u16()
	{
		return subscript_typed<uint16>(parent.aquire(index,2));
	}
	subscript_typed<uint32> u32()
	{
		return subscript_typed<uint32>(parent.aquire(index,4));
	}
	subscript_typed<uint64> u64()
	{
		return subscript_typed<uint64>(parent.aquire(index,8));
	}
	subscript_typed<const char*> string(size_t sz)
	{
		return subscript_typed<const char*>(parent.aquire(index,sz),sz);
	}
	subscript_typed<const unsigned char*> blob(size_t sz)
	{
		return subscript_typed<const unsigned char*>(parent.aquire(index,sz),sz);
	}
	subscript_typed<basics::ipaddress> ip()
	{
		return subscript_typed<basics::ipaddress>(parent.aquire(index,4));
	}
};


///////////////////////////////////////////////////////////////////////////////
/// base packet.
class packet
{
protected:
	friend struct subscript<packet>;
	///////////////////////////////////////////////////////////////////////////
	/// aquire a block of sz bytes starting from pos inside the buffer.
	/// return NULL when accessing out-of-range, overloadable
	virtual uint8* aquire(size_t pos, size_t sz)
	{
		return (pos+sz<=this->length())?const_cast<uint8*>(this->operator()()+pos):NULL;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~packet()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// access to typed elements
	subscript<packet> operator[](unsigned int inx)
	{
		return subscript<packet>(*this, inx);
	}
	///////////////////////////////////////////////////////////////////////////
	/// Length of the packet.
	virtual size_t length() const=0;
	///////////////////////////////////////////////////////////////////////////
	/// Data of the packet.
	virtual const uint8* operator()() const=0;
};


///////////////////////////////////////////////////////////////////////////////
/// fixed size packet.
template<size_t SZ>
class packet_fixed : public packet
{
protected:
	uint8 data[SZ];
public:
	///////////////////////////////////////////////////////////////////////////
	virtual ~packet_fixed()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// Length of the packet
	virtual size_t length() const			{ return SZ; }
	///////////////////////////////////////////////////////////////////////////
	/// Data of the packet
	virtual const uint8* operator()() const	{ return data; }
};



///////////////////////////////////////////////////////////////////////////////
/// dynamic size packet.
class packet_dynamic : public packet
{
protected:
	basics::vector<uint8> data;	/// the buffer
	///////////////////////////////////////////////////////////////////////////
	/// aquire a block of sz bytes starting from pos inside the buffer.
	/// resize the vector when accessing out-of-range
	virtual uint8* aquire(size_t pos, size_t sz)
	{
		if( pos+sz>this->data.size() )
			data.resize(pos+sz);
		return this->data.begin()+pos;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	virtual ~packet_dynamic()
	{}

	///////////////////////////////////////////////////////////////////////////
	/// Length of the packet
	virtual size_t length() const			{ return this->data.size(); }
	///////////////////////////////////////////////////////////////////////////
	/// Data of the packet
	virtual const uint8* operator()() const	{ return this->data.begin(); }
};





inline void send(const packet& pk)
{
	// would do sending here in a real implementation
	// pk() and pk.size() give access to the necessary internals
}


} // end namespace example_socket
///////////////////////////////////////////////////////////////////////////////



namespace example_code {

// some packet
struct my_packet12345
	: public example_socket::packet_fixed<21>	// derive from fixed size packet as we might know that size is fixed
{
public:
	my_packet12345(int a, int b, int c)
	{
		// does whatever to put a,b,c into correct positions of this->data
	}
};
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4189) // variable declared but not used
#endif
inline void usage_example()
{
	// create temporary and send it
	example_socket::send( my_packet12345(1,2,3) );

	// create explicit var
	my_packet12345 pk(1,2,3);
	// and send it
	example_socket::send( pk );


	// free read/write from packets
	{
		example_socket::packet_dynamic pk1;

		pk1[5].u16() = 5;						// put 5 as ushort to position 5
		uint64 ss  = pk1[5].u32();				// read a uint32 from position 5, then cast it to uint64
		char ss1 = pk1[5].u32();				// read a byte from position 5, then cast it to char
		pk1[10].string(10) = "hallo";			// write a string with 10 chars max to position 10
		const char* str = pk1[12].string(5);	// read a string with 5 chars max from position 12
		// prevent unused variable warning
		ss1+=ss++;
		str = " ";
		str++;
	}
	{
		example_socket::packet_fixed<12> pk1;

		pk1[5].u16() = 5;						// put 5 as ushort to position 5
		uint64 ss  = pk1[5].u32();				// read a uint32 from position 5, then cast it to uint64
		char ss1 = pk1[5].u32();				// read a byte from position 5, then cast it to char
		pk1[10].string(10) = "hallo";			// write a string with 10 chars max to position 10
		const char* str = pk1[12].string(5);	// read a string with 5 chars max from position 12
		// prevent unused variable warning
		ss1+=ss++;
		str = " ";
		str++;
	}
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif


}; // end namespace example_code


///////////////////////////////////////////////////////////////////////////////

#endif//__BASEPACKET_H__


