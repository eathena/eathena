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
	These objects point to their corresponding offsets in the packet buffer.

	The number of fields is fixed by design.
	When there is a dynamic number of fields in a packet, they are grouped 
	together as a dynamic field.
	ex: an array of inventory items

	The packet contains an array with pointers to the field objects in-order.
	Each field knows it's size, so to get the offsets we just iterate 
	the array and add-up the sizes.

	Resizing a dynamic field:
	- the data of the field is being changed so the field packs it's data if necessary
	  ex: removing an entry in the middle of an array
	- the field invokes packetbase::on_resize(packetfield& target, size_t new_len)
	  with itself and the new length.
	- the packet reallocs, moves data around and updates the buffers of the 
	  fields as necessary.
	- the field updates it's length and does final changes to the data

*/



//////////////////////////////////////////////////////////////////////////
/// test function
void test_packet(void);



///////////////////////////////////////////////////////////////////////////////
/// base packet
class packetbase
{

	friend class packetfield;

public:
	///////////////////////////////////////////////////////////////////////////
	/// Length of the packet data
	size_t length() const;
	/// Data of the packet
	const uint8* buffer() const;

protected:
	///////////////////////////////////////////////////////////////////////////
	packetbase();
	packetbase(size_t num_fields);
public:
	virtual ~packetbase();

private:
	///////////////////////////////////////////////////////////////////////////
	/// One of the fields was resized.
	/// Move data around and realloc the buffer if necessary.
	void on_resize(packetfield& field, size_t new_len);

	/// Allocate the buffer if it doesn't exist
	void init();

	// internal buffer with packet data
	uint8* _buf;
	size_t _len;
	size_t _max;

	// array of fields in-order
	ptrvector<packetfield> _fields;

};



///////////////////////////////////////////////////////////////////////////////
/// Field of a packet
class packetfield
{

	friend class packetbase;

private:
	///////////////////////////////////////////////////////////////////////////
	/// Size of this field
	virtual size_t _sizeof() const=0;

	/// Set the buffer of the field
	virtual void _setbuf(uint8* buf);

};


///////////////////////////////////////////////////////////////////////////////
// Fixed size fields
//

/// Field with a fixed size
template<size_t LENGTH>
class staticfield : public packetfield
{
protected:
	// pointer to the data
	uint8* _buf;

private:
	///////////////////////////////////////////////////////////////////////////
	/// Size of this field
	virtual size_t _sizeof() const=0;

	/// Set the buffer of the field
	virtual void _setbuf(uint8* buf);
};


/// Byte field
class field_B : public staticfield<1>
{
public:
	operator uint8() const;
	uint8 operator()() const;

	field_B& operator=(const field_B& b);
	uint8 operator=(uint8 val);
};


/// Word field
class field_W : public staticfield<2>
{
public:
	operator uint16() const;
	uint16 operator()() const;

	field_W& operator=(const field_W& w);
	uint16 operator=(uint16 val);
};


/// Long field
class field_L : public staticfield<4>
{
public:
	operator uint32() const;
	uint32 operator()() const;

	field_L& operator=(const field_L& w);
	uint32 operator=(uint32 val);
};


/// String - fixed size
template<size_t LENGTH>
class field_staticstring : public staticfield<LENGTH>
{
public:
	operator const uint8*&();
	const uint8*& operator()() const;
	size_t length() const;
	size_t capacity() const;

	field_staticstring<LENGTH>& operator=(const field_staticstring<LENGTH>& str);
	const uint8*& operator=(const uint8* str);
};

/// Array - fixed size
/// T must be derived from packetfield
template<class T, size_t SIZE>
class field_staticarray : public packetfield
{
public:
	T& operator[](size_t inx);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Size of this field
	virtual size_t _sizeof() const=0;

	/// Set the buffer of the field
	virtual void _setbuf(uint8* buf);

	/// Array of fields
	T _arr[SIZE];
};



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASEPACKET_H__
