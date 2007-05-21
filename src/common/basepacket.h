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

#endif//__BASEPACKET_H__
