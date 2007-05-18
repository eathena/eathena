#ifndef __BASEPACKET_H__
#define __BASEPACKET_H__

#include "basetypes.h"


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
	- the field invokes packetbase::on_resize(packetfield& field, size_t new_len)
	  with itself and the new length.
	- the packet reallocs, moves data around and updates the buffers of the 
	  fields as necessary.
	- the field updates it's length and does final changes to the data

*/


///////////////////////////////////////////////////////////////////////////////
/// base packet
class packetbase
{

	friend class packetfield;

protected:
	// internal buffer with packet data
	uint8* _buf;
	size_t _len;
	size_t _max;

	// array of fields in-order
	packetfield* _fields;
	size_t _fieldcount;

	///////////////////////////////////////////////////////////////////////////
	/// One of the fields was resized.
	/// Move data around or realloc the buffer if necessary.
	void on_resize(packetfield& field, size_t new_len);

public:
	///////////////////////////////////////////////////////////////////////////
	/// Length of the packet data
	size_t length() const;
	/// Data of the packet
	const uint8* buffer() const;
};



///////////////////////////////////////////////////////////////////////////////
// Field of a packet
class packetfield
{

	friend class packetbase;

protected:
	// pointer to the data
	uint8* _buf;

public:
	///////////////////////////////////////////////////////////////////////////
	/// Length of data
	virtual size_t length() const=0;
};



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASEPACKET_H__
