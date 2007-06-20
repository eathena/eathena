#ifndef __BASEPACKET_H__
#define __BASEPACKET_H__

#include "basetypes.h"
#include "basearray.h"


///////////////////////////////////////////////////////////////////////////////
namespace NSocket {
///////////////////////////////////////////////////////////////////////////////

/*

	<< Packets >>

Concept:
	A packet has a set of fields.
	Every field of the packet is a public object in the packet.
	A field is identified by id and is linked internally to a field handler.
	The field handler has the offsets and lengths of every field (set of field 
	positions) and a buffer handler (handles and probably owns the data).

	Current implementations of the set of positions use ids as indexes.

           .---.  .---.  .---.  .-----.  .-----.  .-----.
.----------| B |--| W |--| L |--| Str |--| Arr |--| ... |--.
| IPacket  `---'  `---'  `---'  `-----'  `-----'  `-----'  |
|             \      \      \     /        /        /      |
|              \______\______\___/________/________/       |
|                              |                           |
|                             \|/                          |
|                              `                           |
|             .-------------------------------------.      |
|             | IFieldHandler                       |      |
|             |                                     |      |
|             |   .-----------------------------.   |      |
|             |   |  set of field positions     |   |      |
|             |   `-----------------------------'   |      |
|             |                                     |      |
|             |   .-----------------------------.   |      |
|             |   |  buffer handler and owner   |   |      |
|             |   `-----------------------------'   |      |
|             |                                     |      |
|             `-------------------------------------'      |
|                                                          |
`----------------------------------------------------------'

	IPacket - public packet interface

	B,W,L,Str,Arr,... - public fields, linked internally to the field handler

	IFieldHandler - field handler





Composite field:
	TODO

                      .---------------.
.---------------------| sub-fields... |----.
| ACompositeField     `---------------'    |
|                             |            |
|                            \|/           |
|                             `            |
|   .----------------------------------.   |
|   | CChildIdxFieldHandler            |   |
|   |                                  |   |
|   |   .--------------------------.   |   |
|   |   |  set of field positions  |   |   |
|   |   `--------------------------'   |   |
|   |                                  |   |
|   |   .--------------------------.   |   |
|   |   | CChildBuffer             |   |   |
|   |   `--------------------------'   |   |
|   |                                  |   |
|   `----------------------------------'   |
|                                          |
`------------------------------------------'

	ACompositeField - a field that is composed of other sub-fields

	sub-fields... - public fields, linked internaly to the field handler of ACompositeField

	CChildIdxFieldHandler - an IFieldHandler that uses a CChildBuffer for buffer

	CChildBuffer - buffer that is linked to another IFieldHandler for data

--------------------------------------------------

	TODO

	move the code of CNumField specializations outside the definitions

*/



/// test packet functions
void test_packet(void);

/// The client can handle packet sizes up to 20480, 20k.
#define CLIENTPACKET_MAX 20480





///////////////////////////////////////////////////////////////////////////////
// Field handler
///////////////////////////////////////////////////////////////////////////////
namespace NFieldHandler {





///////////////////////////////////////////////////////////////////////////////
/// Field handler interface
/// Ids can be indexes, offsets, or anything we want. Depends on the implementation.
///
/// @interface
class IFieldHandler
{
public:
	virtual ~IFieldHandler()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Sets the offset and length of the target id.
	virtual void setup(size_t id, size_t off, size_t len)=0;

	/// Returns the data of the target id.
	virtual const uint8* data(size_t id) const=0;
	virtual uint8* data(size_t id)=0;

	/// Returns the offset of the target id.
	virtual size_t offset(size_t id) const=0;

	/// Returns the length of the target id.
	virtual size_t length(size_t id) const=0;

	/// Returns the capcity of the target id.
	virtual size_t capacity(size_t id) const=0;

	/// Inserts n bytes in the offset of the target id.
	/// On success, out contains n.
	/// On failure, out contains the max it can grow.
	virtual bool insert(size_t id, size_t off, size_t n, size_t& out)=0;

	/// Removes n bytes in the offset of the target id.
	/// On success, out contains n.
	/// On failure, out contains the max it can shrink.
	virtual bool remove(size_t id, size_t off, size_t n, size_t& out)=0;

	/// Resizes the target id.
	/// On success out contains sz.
	/// On failure out contains the maximum size if increasing or
	/// the minimum size if decreasing.
	virtual bool resize(size_t id, size_t sz, size_t& out)=0;
};



/// Position of a field in the buffer
struct SPos
{
	SPos():off(0),len(0){}

	/// End of this position.
	inline operator size_t() const{ return this->off + this->len; }

	size_t off;
	size_t len;
};



///////////////////////////////////////////////////////////////////////////////
/// Fixed set of positions.
template<size_t SZ>
class CFixPosSet
{
public:
	CFixPosSet();
	~CFixPosSet();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns if id is included in this set.
	bool contains(size_t id) const;

	/// Adds id to this set.
	bool add(size_t id);

	/// Array access.
	const SPos& operator [](size_t id) const;
	SPos& operator [](size_t id);

	/// List access.
	const SPos& operator ()(size_t id) const;
	SPos& operator ()(size_t id);

	/// Forward id iteration.
	size_t start_id() const;
	size_t next_id(size_t id) const;
	size_t end_id() const;

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of positions
	SPos _arr[SZ];
};

template<size_t SZ>
CFixPosSet<SZ>::CFixPosSet()
{}

template<size_t SZ>
CFixPosSet<SZ>::~CFixPosSet()
{}

/// Returns if id is included in this set.
template<size_t SZ>
bool CFixPosSet<SZ>::contains(size_t id) const
{
	return (id < SZ);
}

/// Adds id to this set.
template<size_t SZ>
bool CFixPosSet<SZ>::add(size_t id)
{
	if( id >= SZ )
		return false;
	this->_arr[id].off = 0;
	this->_arr[id].len = 0;
	return true;
}

/// Array access.
template<size_t SZ>
const SPos& CFixPosSet<SZ>::operator [](size_t id) const
{
	return this->_arr[id];
}
template<size_t SZ>
SPos& CFixPosSet<SZ>::operator [](size_t id)
{
	return this->_arr[id];
}

/// List access.
template<size_t SZ>
const SPos& CFixPosSet<SZ>::operator ()(size_t id) const
{
	return this->_arr[id];
}
template<size_t SZ>
SPos& CFixPosSet<SZ>::operator ()(size_t id)
{
	return this->_arr[id];
}

/// Forward id iteration.
template<size_t SZ>
size_t CFixPosSet<SZ>::start_id() const
{
	return 0;
}
template<size_t SZ>
size_t CFixPosSet<SZ>::next_id(size_t id) const
{
	return id + 1;
}
template<size_t SZ>
size_t CFixPosSet<SZ>::end_id() const
{
	return SZ;
}



///////////////////////////////////////////////////////////////////////////////
/// Dynamic set of positions.
class CDynPosSet
{
public:
	CDynPosSet();
	~CDynPosSet();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns if id is included in this set.
	bool contains(size_t id) const;

	/// Adds id to this set.
	bool add(size_t id);

	/// Array access.
	const SPos& operator [](size_t id) const;
	SPos& operator [](size_t id);

	/// List access.
	const SPos& operator ()(size_t id) const;
	SPos& operator ()(size_t id);

	/// Forward id iteration.
	size_t start_id() const;
	size_t next_id(size_t id) const;
	size_t end_id() const;

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of positions
	basics::vector<SPos> _arr;
};



///////////////////////////////////////////////////////////////////////////////
/// Buffer with a fixed size.
template<size_t SZ>
class CFixBuffer
{
public:
	CFixBuffer();
	CFixBuffer(const uint8* buf, size_t sz);
	~CFixBuffer();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the data of this buffer.
	uint8* data(size_t off=0);
	const uint8* data(size_t off=0) const;

	/// Returns the length of this buffer.
	size_t length() const;

	/// Returns the capacity of this buffer.
	size_t capacity() const;

	/// Inserts n bytes in off.
	/// If successfull out contains n.
	/// Otherwise out contains the maximum n that can inserted in off.
	bool insert(size_t off, size_t n, size_t& out);

	/// Removes n bytes from off. [off,off+n[
	/// If successfull out contains n.
	/// Otherwise out contains the maximum n that can be removed from off.
	bool remove(size_t off, size_t n, size_t& out);

	/// Resizes the buffer to sz.
	/// If successfull out contains sz.
	/// Otherwise out contains the maximum size if increasing 
	/// or the minimum size if decreasing.
	bool resize(size_t sz, size_t& out);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of bytes.
	uint8 _buf[SZ];

	/// Current length.
	size_t _len;
};

template<size_t SZ>
CFixBuffer<SZ>::CFixBuffer()
:	_len(0)
{}

template<size_t SZ>
CFixBuffer<SZ>::CFixBuffer(const uint8* buf, size_t sz)
:	_len(basics::min<size_t>(sz, SZ))
{
	if( buf )
		memcpy(this->_buf, buf, _len);
	else
		memset(this->_buf, 0, _len);
}

template<size_t SZ>
CFixBuffer<SZ>::~CFixBuffer()
{}

/// Returns the data of this buffer.
template<size_t SZ>
const uint8* CFixBuffer<SZ>::data(size_t off) const
{
	return (const uint8*)this->_buf + off;
}
template<size_t SZ>
uint8* CFixBuffer<SZ>::data(size_t off)
{
	return (uint8*)this->_buf + off;
}

/// Returns the length of this buffer.
template<size_t SZ>
size_t CFixBuffer<SZ>::length() const
{
	return this->_len;
}

/// Returns the capacity of this buffer.
template<size_t SZ>
size_t CFixBuffer<SZ>::capacity() const
{
	return SZ;
}

/// Inserts n bytes in off.
/// If successfull out contains n.
/// Otherwise out contains the maximum n that can inserted in off.
template<size_t SZ>
bool CFixBuffer<SZ>::insert(size_t off, size_t n, size_t& out)
{
	if( this->capacity() - this->length() < n )
	{// failed
		out = this->capacity() - this->length();
		return false;
	}
	memmove(this->data(off + n), this->data(off), this->length() - off);
	memset(this->data(off), 0, n);
	this->_len += n;
	out = n;
	return true;
}

/// Removes n bytes from off. [off,off+n[
/// If successfull out contains n.
/// Otherwise out contains the maximum n that can be removed from off.
template<size_t SZ>
bool CFixBuffer<SZ>::remove(size_t off, size_t n, size_t& out)
{
	if( this->length() < off + n )
	{// failed
		out = this->length() - off;
		return false;
	}
	memmove(this->data(off), this->data(off + n), this->length() - off - n);
	this->_len -= n;
	out = n;
	return true;
}

/// Resizes the buffer to sz.
/// If successfull out contains sz.
/// Otherwise out contains the maximum size if increasing 
/// or the minimum size if decreasing.
template<size_t SZ>
bool CFixBuffer<SZ>::resize(size_t sz, size_t& out)
{
	if( this->capacity() < sz )
	{// failed
		out = this->capacity();
		return false;
	}
	if( sz > this->length() )// increasing
		memset(this->data(this->length()), 0, sz - this->length());
	this->_len = out = sz;
	return true;
}



///////////////////////////////////////////////////////////////////////////////
/// Buffer with a dynamic size.
class CDynBuffer
{
public:
	CDynBuffer();
	CDynBuffer(const uint8* buf, size_t sz);
	~CDynBuffer();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the data of this buffer.
	const uint8* data(size_t off=0) const;
	uint8* data(size_t off=0);

	/// Returns the length of this buffer.
	size_t length() const;

	/// Returns the capacity of this buffer.
	size_t capacity() const;

	/// Inserts n bytes in off.
	/// If successfull out contains n.
	/// Otherwise out contains the maximum n that can inserted in off.
	bool insert(size_t off, size_t n, size_t& out);

	/// Removes n bytes from off. [off,off+n[
	/// If successfull out contains n.
	/// Otherwise out contains the maximum n that can be removed from off.
	bool remove(size_t off, size_t n, size_t& out);

	/// Resizes the buffer to sz.
	/// If successfull out contains sz.
	/// Otherwise out contains the maximum size if increasing 
	/// or the minimum size if decreasing.
	bool resize(size_t sz, size_t& out);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Vector of bytes.
	basics::vector<uint8> _buf;
};



///////////////////////////////////////////////////////////////////////////////
/// Buffer backed by a field handler.
class CChildBuffer
{
public:
	CChildBuffer();
	~CChildBuffer();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes this child buffer.
	void Init(IFieldHandler* h, size_t id, size_t off, size_t len);

	/// Returns the data of this buffer.
	const uint8* data(size_t off=0) const;
	uint8* data(size_t off=0);

	/// Returns the length of this buffer.
	size_t length() const;

	/// Returns the capacity of this buffer.
	size_t capacity() const;

	/// Inserts n bytes in off.
	/// If successfull out contains n.
	/// Otherwise out contains the maximum n that can inserted in off.
	bool insert(size_t off, size_t n, size_t& out);

	/// Removes n bytes from off. [off,off+n[
	/// If successfull out contains n.
	/// Otherwise out contains the maximum n that can be removed from off.
	bool remove(size_t off, size_t n, size_t& out);

	/// Resizes the buffer to sz.
	/// If successfull out contains sz.
	/// Otherwise out contains the maximum size if increasing 
	/// or the minimum size if decreasing.
	bool resize(size_t sz, size_t& out);

	/// Returns the id of this child buffer in the parent.
	size_t id() const;

	/// Returns the parent of this child buffer.
	const IFieldHandler* parent() const;
	IFieldHandler* parent();

private:
	///////////////////////////////////////////////////////////////////////////
	/// Id in the parent.
	size_t _id;

	/// Parent field handler (where the data is located).
	IFieldHandler* _h;
};



///////////////////////////////////////////////////////////////////////////////
/// Field handler where ids are offsets in the buffer.
/// If used Only use in fixed-size packets with fixed-size fields.
///
/// @param B Buffer
template<class B>
class COffFieldHandler : public IFieldHandler
{
public:
	COffFieldHandler();
	COffFieldHandler(const uint8* buf, size_t sz);
	virtual ~COffFieldHandler();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Ensures that the buffer is big enough.
	virtual void setup(size_t id, size_t off, size_t len);

	/// Returns the data of the target id.
	virtual const uint8* data(size_t id) const;
	virtual uint8* data(size_t id);

	/// Returns the offset of the target id.
	virtual size_t offset(size_t id) const;

	/// Returns the length of the target id.
	virtual size_t length(size_t id) const;

	/// Returns the capacity of the target id.
	virtual size_t capacity(size_t id) const;

	/// Inserts n bytes in the offset of the target id.
	/// On success, out contains n.
	/// On failure, out contains the max it can grow.
	virtual bool insert(size_t id, size_t off, size_t n, size_t& out);

	/// Removes n bytes in the offset of the target id.
	/// On success, out contains sz.
	/// On failure, out contains the max it can shrink.
	virtual bool remove(size_t id, size_t off, size_t n, size_t& out);

	/// Tries to resize the target id.
	/// On success out contains the new size.
	/// On failure out contains the maximum size if increasing or
	/// the minimum size if decreasing.
	virtual bool resize(size_t id, size_t sz, size_t& out);

	/// Returns the data of this buffer.
	const uint8* data() const;
	uint8* data();

	/// Returns the length of this buffer.
	size_t length() const;

protected:
	///////////////////////////////////////////////////////////////////////////
	/// Buffer
	B _buf;
};

template<class B>
COffFieldHandler<B>::COffFieldHandler()
:	_buf()
{}
template<class B>
COffFieldHandler<B>::COffFieldHandler(const uint8* buf, size_t sz)
:	_buf(buf, sz)
{}
template<class B>
COffFieldHandler<B>::~COffFieldHandler()
{}

/// Ensures that the buffer is big enough.
template<class B>
void COffFieldHandler<B>::setup(size_t id, size_t off, size_t len)
{
	size_t maxlen;
	if( this->_buf.length() < id + len && !this->_buf.resize(id + len, maxlen) )
	{
		//## critical error, bad packet definition?
		printf("[Debug] resize failed - bad packet definition? at COffFieldHandler::setup(%u,%u,%u)\n", (uint)id, (uint)off, (uint)len);
	}
}

/// Returns the data of the target id.
template<class B>
const uint8* COffFieldHandler<B>::data(size_t id) const
{
	return this->_buf.data(id);
}
template<class B>
uint8* COffFieldHandler<B>::data(size_t id)
{
	return this->_buf.data(id);
}

/// Returns the offset of the target id.
template<class B>
size_t COffFieldHandler<B>::offset(size_t id) const
{
	return id;
}

/// Returns the length of the target id.
template<class B>
size_t COffFieldHandler<B>::length(size_t id) const
{
	if( this->length() < id )
		return 0;
	return this->length() - id;
}

/// Returns the capacity of the target id.
template<class B>
size_t COffFieldHandler<B>::capacity(size_t id) const
{
	if( this->_buf.capacity() < id )
		return 0;
	return this->_buf.capacity() - id;
}

/// Inserts n bytes in the offset of the target id.
/// On success, out contains n.
/// On failure, out contains the max it can grow.
template<class B>
bool COffFieldHandler<B>::insert(size_t id, size_t off, size_t n, size_t& out)
{
	return this->_buf.insert(id + off, n, out);
}

/// Removes n bytes in the offset of the target id.
/// On success, out contains sz.
/// On failure, out contains the max it can shrink.
template<class B>
bool COffFieldHandler<B>::remove(size_t id, size_t off, size_t n, size_t& out)
{
	return this->_buf.remove(id + off, n, out);
}

/// Tries to resize the target id.
/// On success out contains the new size.
/// On failure out contains the maximum size if increasing or
/// the minimum size if decreasing.
template<class B>
bool COffFieldHandler<B>::resize(size_t id, size_t sz, size_t& out)
{
	bool res = this->_buf.resize(id + sz, out);
	out -= id;
	return res;
}

/// Returns the data of the buffer.
template<class B>
const uint8* COffFieldHandler<B>::data() const
{
	return this->_buf.data();
}
template<class B>
uint8* COffFieldHandler<B>::data()
{
	return this->_buf.data();
}

/// Returns the length of the buffer.
template<class B>
size_t COffFieldHandler<B>::length() const
{
	return this->_buf.length();
}



///////////////////////////////////////////////////////////////////////////////
/// Field handler where ids are indexes in a position array.
///
/// @param P Positions
/// @param B Buffer
template<class P, class B>
class CIdxFieldHandler : public IFieldHandler
{
public:
	CIdxFieldHandler();
	CIdxFieldHandler(const uint8* buf, size_t sz);
	virtual ~CIdxFieldHandler();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Sets the offset and length of the target id.
	virtual void setup(size_t id, size_t off, size_t len);

	/// Returns the data of the target id.
	virtual const uint8* data(size_t id) const;
	virtual uint8* data(size_t id);

	/// Returns the offset of the target id.
	virtual size_t offset(size_t id) const;

	/// Returns the length of the target id.
	virtual size_t length(size_t id) const;

	/// Returns the capacity of the target id.
	virtual size_t capacity(size_t id) const;

	/// Inserts n bytes in the offset of the target id.
	/// On success, out contains n.
	/// On failure, out contains the max it can grow.
	virtual bool insert(size_t id, size_t off, size_t n, size_t& out);

	/// Removes n bytes in the offset of the target id.
	/// On success, out contains sz.
	/// On failure, out contains the max it can shrink.
	virtual bool remove(size_t id, size_t off, size_t n, size_t& out);

	/// Tries to resize the target id.
	/// On success out contains the new size.
	/// On failure out contains the maximum size if increasing or
	/// the minimum size if decreasing.
	virtual bool resize(size_t id, size_t sz, size_t& out);

	/// Returns the data of the buffer.
	const uint8* data() const;
	uint8* data();

	/// Returns the length of the buffer.
	size_t length() const;

protected:
	///////////////////////////////////////////////////////////////////////////
	/// Positions
	P _pos;

	/// Buffer
	B _buf;
};

template<class P, class B>
CIdxFieldHandler<P,B>::CIdxFieldHandler()
:	_pos()
,	_buf()
{}
template<class P, class B>
CIdxFieldHandler<P,B>::CIdxFieldHandler(const uint8* buf, size_t sz)
:	_pos()
,	_buf(buf, sz)
{}
template<class P, class B>
CIdxFieldHandler<P,B>::~CIdxFieldHandler()
{}

/// Sets the offset and length of the target id.
template<class P, class B>
void CIdxFieldHandler<P,B>::setup(size_t id, size_t off, size_t len)
{
	if( !this->_pos.contains(id) && !this->_pos.add(id) )
		return;
	this->_pos(id).off = off;
	this->_pos(id).len = len;
	size_t end = off + len;
	if( this->_buf.length() < end && !this->_buf.resize(end, end) )
	{
		//## critical error, bad packet definition?
		printf("[Debug] resize failed (maxlen=%u)- bad packet definition? at CIdxFieldHandler::setup(%u,%u,%u)\n", (uint)end, (uint)id, (uint)off, (uint)len);
		this->_pos(id).off = 0;
		this->_pos(id).len = 0;
	}
}

/// Returns the data of the target id.
template<class P, class B>
const uint8* CIdxFieldHandler<P,B>::data(size_t id) const
{
	if( !this->_pos.contains(id) )
		return NULL;
	return this->_buf.data(_pos(id).off);
}
template<class P, class B>
uint8* CIdxFieldHandler<P,B>::data(size_t id)
{
	if( !this->_pos.contains(id) )
		return NULL;
	return this->_buf.data(_pos(id).off);
}

/// Returns the offset of the target id.
template<class P, class B>
size_t CIdxFieldHandler<P,B>::offset(size_t id) const
{
	if( !this->_pos.contains(id) )
		return 0;
	return this->_pos(id).off;
}

/// Returns the length of the target id.
template<class P, class B>
size_t CIdxFieldHandler<P,B>::length(size_t id) const
{
	if( !this->_pos.contains(id) )
		return 0;
	return this->_pos(id).len;
}

/// Returns the capacity of the target id.
template<class P, class B>
size_t CIdxFieldHandler<P,B>::capacity(size_t id) const
{
	if( !this->_pos.contains(id) )
		return 0;
	return this->_pos(id).len + this->_buf.capacity() - this->_buf.length();
}

/// Inserts n bytes in the offset of the target id.
/// On success, out contains n.
/// On failure, out contains the max it can grow.
template<class P, class B>
bool CIdxFieldHandler<P,B>::insert(size_t id, size_t off, size_t n, size_t& out)
{
	if( !this->_pos.contains(id) )
		return false;
#if defined(DEBUG)
	if( off > this->_pos(id).len )// out of range
		printf("[Debug] off is out of range (len=%u) at CIdxFieldHandler::insert(%u,%u,%u,-)\n", (uint)this->_pos(id).len, (uint)id, (uint)off, (uint)n);
#endif//DEBUG
	off += this->_pos(id).off;
	if( !this->_buf.insert(off, n, out) )
		return false;
	this->_pos(id).len += n;
	// adjust positions
	size_t i;
	for( i = this->_pos.start_id(); i != this->_pos.end_id(); i = this->_pos.next_id(i) )
		if( this->_pos(i).off >= off && i != id )
			this->_pos(i).off += n;
	return true;
}

/// Removes n bytes in the offset of the target id.
/// On success, out contains sz.
/// On failure, out contains the max it can shrink.
template<class P, class B>
bool CIdxFieldHandler<P,B>::remove(size_t id, size_t off, size_t n, size_t& out)
{
	if( !this->_pos.contains(id) )
		return false;
#if defined(DEBUG)
	if( off + n > this->_pos(id).len )// out of range
		printf("[Debug] off+n is out of range at CIdxFieldHandler::remove(%u,%u,%u,-)\n", (uint)id, (uint)off, (uint)n);
#endif//DEBUG
	off += this->_pos(id).off;
	if( !this->_buf.remove(off, n, out) )
		return false;
	this->_pos(id).len -= n;
	// adjust positions
	size_t i;
	for( i = this->_pos.start_id(); i != this->_pos.end_id(); i = this->_pos.next_id(i) )
		if( this->_pos(i).off >= off && i != id )
			this->_pos(i).off -= n;
	return true;
}

/// Tries to resize the target id.
/// On success out contains the new size.
/// On failure out contains the maximum size if increasing or
/// the minimum size if decreasing.
template<class P, class B>
bool CIdxFieldHandler<P,B>::resize(size_t id, size_t sz, size_t& out)
{
	if( !this->_pos.contains(id) )
	{
		out = 0;
		return false;
	}
	size_t old_len = this->_pos(id).len;
	if( sz > old_len )
	{// increase size
		size_t inc = sz - old_len;
		if( !this->insert(id, old_len, inc, out) )
		{
			out += old_len;
			return false;
		}
	}
	else if( sz < old_len )
	{// decrease size
		size_t dec = old_len - sz;
		if( !this->remove(id, old_len - dec, dec, out) )
		{// failed
			out = old_len - out;
			return false;
		}
	}
	this->_pos(id).len = out = sz;
	return true;
}

/// Returns the data of the buffer.
template<class P, class B>
const uint8* CIdxFieldHandler<P,B>::data() const
{
	return this->_buf.data();
}
template<class P, class B>
uint8* CIdxFieldHandler<P,B>::data()
{
	return this->_buf.data();
}

/// Returns the length of the buffer.
template<class P, class B>
size_t CIdxFieldHandler<P,B>::length() const
{
	return this->_buf.length();
}



///////////////////////////////////////////////////////////////////////////////
/// Field buffer that uses another field buffer for data.
/// Ids are indexes in the position array.
///
/// @param P Positions
template<class P>
class CChildIdxFieldBuffer : public CIdxFieldHandler<P,CChildBuffer>
{
public:
	CChildIdxFieldBuffer();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes this child buffer.
	void Init(IFieldHandler* h, size_t id, size_t off, size_t len);

	/// Normalizes the size of this field buffer.
	/// Invoke after initializing fields and restricting size.
	void normalize();

	/// Sets the offset and length of the target id.
	virtual void setup(size_t id, size_t off, size_t len);
};

template<class P>
CChildIdxFieldBuffer<P>::CChildIdxFieldBuffer()
:	CIdxFieldHandler<P,CChildBuffer>()
{}

/// Initializes this child buffer.
template<class P>
void CChildIdxFieldBuffer<P>::Init(IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_buf.Init(h, id, off, len);
}

/// Normalizes the size of this field buffer.
/// Invoke after initializing fields and restricting size.
template<class P>
void CChildIdxFieldBuffer<P>::normalize()
{
	size_t len = 0;
	size_t i;
	for( i = this->_pos.start_id(); i != this->_pos.end_id(); i = this->_pos.next_id(i) )
		len = basics::max<size_t>(len, this->_pos(i));

	IFieldHandler* parent = this->_buf.parent();
	if( parent == NULL )
	{
#if defined(DEBUG)
		printf("[Debug] buffer has no parent - bug in the source? at CChildIdxFieldBuffer::normalize()\n");
#endif
		return;
	}
	size_t id = this->_buf.id();
	size_t off = parent->offset(id);
	this->_buf.Init(parent, id, off, len);
}

/// Sets the offset and length of the target id.
template<class P>
void CChildIdxFieldBuffer<P>::setup(size_t id, size_t off, size_t len)
{
	if( !this->_pos.contains(id) && !this->_pos.add(id) )
		return;
	this->_pos(id).off = off;
	this->_pos(id).len = len;
}



///////////////////////////////////////////////////////////////////////////////
/// Automatic registry of fields as indexes in a field handler.
class CIdxAutoRegistry
{
public:
	CIdxAutoRegistry(IFieldHandler& h)
	:	_h(h)
	,	_idx(0)
	,	_off(0)
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Registers a field at the next index.
	template<typename T>
	CIdxAutoRegistry& operator <<(T& f)
	{
		return this->append(f);
	}

	/// Registers a field at the next index with the target maximum length.
	template<typename T>
	CIdxAutoRegistry& append(T& f, size_t maxlen=0)
	{
		f.Init(&this->_h, this->_idx, this->_off, maxlen);
		this->_off += this->_h.length(this->_idx);
		++this->_idx;
		return *this;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// Target field handler
	IFieldHandler& _h;
	/// Index of the next field
	size_t _idx;
	/// Offset of the next field
	size_t _off;
};



///////////////////////////////////////////////////////////////////////////////
/// Field handler to get the values passed on to the setup method.
class CSetupFieldHandler : public IFieldHandler
{
public:
	/// Values of the setup method.
	size_t id;
	size_t off;
	size_t len;
public:
	CSetupFieldHandler()
	:	id(0)
	,	off(0)
	,	len(0)
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Records the setup values.
	virtual void setup(size_t id, size_t off, size_t len)
	{
		this->id = id;
		this->off = off;
		this->len = len;
	}

	/// Not Supported
	virtual const uint8* data(size_t) const					{ return NULL; }
	virtual uint8* data(size_t)								{ return NULL; }
	virtual size_t offset(size_t) const						{ return 0; }
	virtual size_t length(size_t) const						{ return 0; }
	virtual size_t capacity(size_t) const					{ return 0; }
	virtual bool insert(size_t, size_t, size_t, size_t&)	{ return false; }
	virtual bool remove(size_t, size_t, size_t, size_t&)	{ return false; }
	virtual bool resize(size_t, size_t, size_t&)			{ return false; }
};





}// end namespace NFieldHandler
///////////////////////////////////////////////////////////////////////////////
// Packets
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
/// Packet interface
///
/// @interface
class IPacket
{
public:
	virtual ~IPacket()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the data of this packet.
	virtual const uint8* data() const=0;

	/// Returns the length of this packet.
	virtual size_t length() const=0;
};



///////////////////////////////////////////////////////////////////////////////
/// Abstract packet.
///
/// @param T Field handler
template<class T>
class APacket : public IPacket
{
protected:
	APacket()
	:	_h()
	{}
	APacket(const uint8* buf, size_t sz)
	:	_h(buf, sz)
	{}
public:
	virtual ~APacket()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the data of this packet.
	virtual const uint8* data() const
	{
		return this->_h.data();
	}

	/// Returns the length of the packet.
	virtual size_t length() const
	{
		return this->_h.length();
	}

protected:
	///////////////////////////////////////////////////////////////////////////
	/// Field handler
	T _h;
};



///////////////////////////////////////////////////////////////////////////////
/// Fixed-size packet with an offset-based field handler.
/// Must contain fixed-size fields only.
///
/// @param SZ Size of the buffer
template<size_t SZ>
class CFixOffPacket : public APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CFixBuffer<SZ> > >
{
protected:
	CFixOffPacket<SZ>()
	:	APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CFixBuffer<SZ> > >(NULL, SZ)
	{}
	CFixOffPacket<SZ>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CFixBuffer<SZ> > >(NULL, SZ)
	{
		if( buf )
			memcpy(this->_h.data(), buf, basics::min<size_t>(sz, SZ));
	}
};



///////////////////////////////////////////////////////////////////////////////
/// Dynamic-size packet with a limited buffer and an offset-based field handler.
/// There can be only one dynamic-size field and it must be the last one.
///
/// @param SZ Size of the buffer
template<size_t SZ>
class CLimOffPacket : public APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CFixBuffer<SZ> > >
{
protected:
	CLimOffPacket<SZ>()
	:	APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CFixBuffer<SZ> > >()
	{}
	CLimOffPacket<SZ>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CFixBuffer<SZ> > >(buf,sz)
	{}
};



///////////////////////////////////////////////////////////////////////////////
/// Dynamic-size packet with an offset-based field handler.
/// There can be only one dynamic-size field and it must be the last one.
class CDynOffPacket : public APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CDynBuffer> >
{
protected:
	CDynOffPacket()
	:	APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CDynBuffer> >()
	{}
	CDynOffPacket(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::COffFieldHandler<NFieldHandler::CDynBuffer> >(buf,sz)
	{}
};



///////////////////////////////////////////////////////////////////////////////
/// Fixed-size packet with an index-based field handler.
/// Must contain fixed-size fields only.
///
/// @param NUM Number of fields 
/// @param SZ Size of the buffer
template<size_t NUM,size_t SZ>
class CFixIdxPacket : public APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >
{
protected:
	CFixIdxPacket<NUM,SZ>()
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >(NULL, SZ)
	{}
	CFixIdxPacket<NUM,SZ>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >(NULL, SZ)
	{
		if( buf )
			memcpy(this->_h.data(), buf, basics::min<size_t>(sz, SZ));
	}
};



///////////////////////////////////////////////////////////////////////////////
/// Dynamic-size packet with a limited buffer and an index-based field handler.
///
/// @param NUM Number of fields 
/// @param SZ Size of the buffer
template<size_t NUM,size_t SZ>
class CLimIdxPacket : public APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >
{
protected:
	CLimIdxPacket<NUM,SZ>()
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >()
	{}
	CLimIdxPacket<NUM,SZ>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >(buf,sz)
	{}
};



///////////////////////////////////////////////////////////////////////////////
/// Dynamic-size packet with an index-based field handler.
///
/// @param NUM Number of fields
template<size_t NUM>
class CDynIdxPacket : public APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CDynBuffer> >
{
protected:
	CDynIdxPacket<NUM>()
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CDynBuffer> >()
	{}
	CDynIdxPacket<NUM>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CDynBuffer> >(buf,sz)
	{}
};





///////////////////////////////////////////////////////////////////////////////
// Fields
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
/// Abstract field
class AField
{
protected:
	AField()
	:	_h(NULL)
	,	_id(~size_t(0))
	{}
	~AField()
	{
		this->_h = NULL;
		this->_id = ~size_t(0);
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	/// Every field needs to implement this function.
	///
	/// @param maxlen Maximum length the field can take
	//void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t maxlen)

protected:
	///////////////////////////////////////////////////////////////////////////
	/// Field handler
	NFieldHandler::IFieldHandler* _h;

	/// Id of this field
	size_t _id;
};



///////////////////////////////////////////////////////////////////////////////
/// Field with a fixed size.
template<size_t SZ>
class AFixField : public AField
{
protected:
	AFixField()
	:	AField()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length the field can have
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t maxlen=SZ)
	{
		this->_id = id;
		this->_h = h;
		if( h )
			h->setup(id, off, SZ);// fixed length
	}
};



///////////////////////////////////////////////////////////////////////////////
/// Abstract composite index-based field
///
/// @param P Positions
template<class P>
class ACompositeIdxField
{
protected:
	ACompositeIdxField()
	:	_h()
	{}
	~ACompositeIdxField()
	{
		_h.Init(NULL, 0, 0, 0);
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	/// Every composite field needs to implement this function.
	///
	/// @param maxlen Maximum length of the composite field
	//void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t maxlen)

protected:
	///////////////////////////////////////////////////////////////////////////
	/// Field handler
	NFieldHandler::CChildIdxFieldBuffer<P> _h;
};



/// Numeric field. Error for unsupported types.
template<typename T>
class CNumField
{};



typedef class CNumField<uint8> CFieldB;// Byte field
typedef class CNumField<uint16> CFieldW;// Word field
typedef class CNumField<uint32> CFieldL;// Long field
typedef class CNumField<uint64> CFieldLL;// Long Long field



///////////////////////////////////////////////////////////////////////////////
// uint8/byte field
template<>
class CNumField<uint8> : public AFixField<1>
{
public:
	CNumField<uint8>()
	:	AFixField<1>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the value of this field.
	operator uint8() const
	{
		return this->operator()();
	}
	uint8 operator ()() const
	{
		if( this->_h )
			return this->_h->data(this->_id)[0];
		return 0;
	}

	/// Sets the value of this field.
	CNumField<uint8>& operator =(const CNumField<uint8>& f)
	{
		return this->operator =(f());
	}
	CNumField<uint8>& operator =(uint8 val)
	{
		if( this->_h )
			this->_h->data(this->_id)[0] = val;
		return *this;
	}
};



///////////////////////////////////////////////////////////////////////////////
// uint16/word field
template<>
class CNumField<uint16> : public AFixField<2>
{
public:
	CNumField<uint16>()
	:	AFixField<2>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the value of this field.
	operator uint16() const
	{
		return this->operator ()();
	}
	uint16 operator ()() const
	{
		if( this->_h )
		{
			const uint8* buf = this->_h->data(this->_id);
			return
				( uint16(buf[0])         )|
				( uint16(buf[1]) << 0x08 );
		}
		return 0;
	}

	/// Sets the value of this field.
	CNumField<uint16>& operator =(const CNumField<uint16>& f)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			if( f._h )
				memcpy(buf, f._h->data(f._id), 2);
			else
				memset(buf, 0, 2);
		}
		return *this;
	}
	CNumField<uint16>& operator =(uint16 val)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			buf[0] = uint8( (val & 0x00FF)         );
			buf[1] = uint8( (val & 0xFF00) >> 0x08 );
		}
		return *this;
	}
};



///////////////////////////////////////////////////////////////////////////////
// uint32/dword/long field
template<>
class CNumField<uint32> : public AFixField<4>
{
public:
	CNumField<uint32>()
	:	AFixField<4>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the value of this field.
	operator uint32() const
	{
		return this->operator ()();
	}
	uint32 operator ()() const
	{
		if( this->_h )
		{
			const uint8* buf = this->_h->data(this->_id);
			return
				( uint32(buf[0])         )|
				( uint32(buf[1]) << 0x08 )|
				( uint32(buf[2]) << 0x10 )|
				( uint32(buf[3]) << 0x18 );
		}
		return 0;
	}

	/// Sets the value of this field.
	CNumField<uint32>& operator =(const CNumField<uint32>& f)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			if( f._h )
				memcpy(buf, f._h->data(f._id), 4);
			else
				memset(buf, 0, 4);
		}
		return *this;
	}
	CNumField<uint32>& operator =(uint32 val)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			buf[0] = uint8( (val & 0x000000FF)         );
			buf[1] = uint8( (val & 0x0000FF00) >> 0x08 );
			buf[2] = uint8( (val & 0x00FF0000) >> 0x10 );
			buf[3] = uint8( (val & 0xFF000000) >> 0x18 );
		}
		return *this;
	}
};



///////////////////////////////////////////////////////////////////////////////
// uint64/qword/long long field
template<>
class CNumField<uint64> : public AFixField<8>
{
public:
	CNumField<uint64>()
	:	AFixField<8>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the value of this field.
	operator uint64() const
	{
		return this->operator ()();
	}
	uint64 operator ()() const
	{
		if( this->_h )
		{
			const uint8* buf = this->_h->data(this->_id);
			return
				( uint64(buf[0])         )|
				( uint64(buf[1]) << 0x08 )|
				( uint64(buf[2]) << 0x10 )|
				( uint64(buf[3]) << 0x18 )|
				( uint64(buf[4]) << 0x20 )|
				( uint64(buf[5]) << 0x28 )|
				( uint64(buf[6]) << 0x30 )|
				( uint64(buf[7]) << 0x38 );
		}
		return 0;
	}

	/// Sets the value of this field.
	CNumField<uint64>& operator =(const CNumField<uint64>& f)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			if( f._h )
				memcpy(buf, f._h->data(f._id), 8);
			else
				memset(buf, 0, 8);
		}
		return *this;
	}
	CNumField<uint64>& operator =(uint64 val)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			buf[0] = uint8( (val & LLCONST(0x00000000000000FF))         );
			buf[1] = uint8( (val & LLCONST(0x000000000000FF00)) >> 0x08 );
			buf[2] = uint8( (val & LLCONST(0x0000000000FF0000)) >> 0x10 );
			buf[3] = uint8( (val & LLCONST(0x00000000FF000000)) >> 0x18 );
			buf[4] = uint8( (val & LLCONST(0x000000FF00000000)) >> 0x20 );
			buf[5] = uint8( (val & LLCONST(0x0000FF0000000000)) >> 0x28 );
			buf[6] = uint8( (val & LLCONST(0x00FF000000000000)) >> 0x30 );
			buf[7] = uint8( (val & LLCONST(0xFF00000000000000)) >> 0x38 );
		}
		return *this;
	}
};



///////////////////////////////////////////////////////////////////////////////
/// Fixed-size string.
/// Warning: not nul-terminated if length equals capacity (SZ).
///
/// @param SZ Size of the string
template<size_t SZ>
class CFieldFixString : public AFixField<SZ>
{
public:
	CFieldFixString();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Converts this string to data.
	operator const char*() const;

	/// Returns the data of this string.
	const char* operator ()() const;
	const char* data() const;

	/// Assigns data to this string.
	template<size_t SZ2>
	CFieldFixString<SZ>& operator =(const CFieldFixString<SZ2>& f)
	{
		return this->assign(f.data(), SZ2);
	}
	CFieldFixString<SZ>& operator =(const char* str);
	CFieldFixString<SZ>& assign(const char* str, size_t sz);

	/// Returns the length of this string.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity of this string.
	size_t capacity() const;
};

template<size_t SZ>
CFieldFixString<SZ>::CFieldFixString()
:	AFixField<SZ>()
{}

/// Converts this string to data.
template<size_t SZ>
CFieldFixString<SZ>::operator const char*() const
{
	return this->data();
}

/// Returns the data of this string.
template<size_t SZ>
const char* CFieldFixString<SZ>::operator ()() const
{
	return this->data();
}
template<size_t SZ>
const char* CFieldFixString<SZ>::data() const
{
	if( this->_h )
		return (const char*)this->_h->data(this->_id);
	return "";
}

/// Assigns data to this string.
template<size_t SZ>
CFieldFixString<SZ>& CFieldFixString<SZ>::operator =(const char* str)
{
	return this->assign(str, ~size_t(0));
}
template<size_t SZ>
CFieldFixString<SZ>& CFieldFixString<SZ>::assign(const char* str, size_t sz)
{
	if( this->_h )
	{
		if( str == NULL )
			str = "";
		size_t len = strnlen(str, basics::min<size_t>(sz, SZ));
		char* buf = (char*)this->_h->data(this->_id);
		memcpy(buf, str, len);
		if( len < SZ )
			memset(buf + len, 0, SZ - len);
	}
	return *this;
}

/// Returns the length of this string.
template<size_t SZ>
size_t CFieldFixString<SZ>::size() const
{
	return this->length();
}
template<size_t SZ>
size_t CFieldFixString<SZ>::length() const
{
	return strnlen(this->data(), SZ);
}

/// Returns the capacity of this string.
template<size_t SZ>
size_t CFieldFixString<SZ>::capacity() const
{
	return SZ;
}



///////////////////////////////////////////////////////////////////////////////
/// Dynamic-size nul-terminated string.
class CFieldCString : public AField
{
public:
	CFieldCString();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes this field.
	///
	/// @param len Maximum length of the field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0);

	/// Converts this c-string to data.
	operator const char*() const;

	/// Returns the data of this c-string.
	const char* operator ()() const;
	const char* data() const;

	/// Assigns data to this c-string.
	CFieldCString& operator =(const CFieldCString& f);
	CFieldCString& operator =(const char* str);
	CFieldCString& assign(const char* str, size_t sz);

	/// Returns the length of this c-string.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity of this c-string.
	size_t capacity() const;
};



///////////////////////////////////////////////////////////////////////////////
/// Fixed-size blob of data.
///
/// @param SZ Size of the string
template<size_t SZ>
class CFieldFixBlob : public AFixField<SZ>
{
public:
	CFieldFixBlob();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Array access.
	const uint8& operator [](size_t idx) const;
	uint8& operator [](size_t idx);

	/// List access.
	const uint8& operator ()(size_t idx) const;
	uint8& operator ()(size_t idx);

	/// Converts this blob to data.
	operator const uint8*() const;
	operator uint8*();

	/// Returns the data of this blob.
	const uint8* data() const;
	uint8* data();
	const uint8* operator ()() const;
	uint8* operator ()();

	/// Assigns data to this blob.
	template<size_t SZ2>
	CFieldFixBlob<SZ>& operator =(const CFieldFixBlob<SZ2>& f)
	{
		return this->assign(f.data(), SZ2);
	}
	CFieldFixBlob<SZ>& operator =(const uint8* data);
	CFieldFixBlob<SZ>& assign(const uint8* data, size_t sz);

	/// Returns the length of this blob.
	size_t size() const;
	size_t length() const;

	/// Resizes this blob.
	bool resize(size_t len);
};

template<size_t SZ>
CFieldFixBlob<SZ>::CFieldFixBlob()
:	AFixField<SZ>()
{}

/// Array access.
template<size_t SZ>
const uint8& CFieldFixBlob<SZ>::operator [](size_t idx) const
{
	return *(this->data() + idx);
}
template<size_t SZ>
uint8& CFieldFixBlob<SZ>::operator [](size_t idx)
{
	return *(this->data() + idx);
}

/// List access.
template<size_t SZ>
const uint8& CFieldFixBlob<SZ>::operator ()(size_t idx) const
{
	return *(this->data() + idx);
}
template<size_t SZ>
uint8& CFieldFixBlob<SZ>::operator ()(size_t idx)
{
	return *(this->data() + idx);
}

/// Converts this blob to data.
template<size_t SZ>
CFieldFixBlob<SZ>::operator const uint8*() const
{
	return this->data();
}
template<size_t SZ>
CFieldFixBlob<SZ>::operator uint8*()
{
	return this->data();
}

/// Returns the data of this blob.
template<size_t SZ>
const uint8* CFieldFixBlob<SZ>::data() const
{
	return this->_h->data(this->_id);
}
template<size_t SZ>
uint8* CFieldFixBlob<SZ>::data()
{
	return this->_h->data(this->_id);
}
template<size_t SZ>
const uint8* CFieldFixBlob<SZ>::operator ()() const
{
	return this->data();
}
template<size_t SZ>
uint8* CFieldFixBlob<SZ>::operator ()()
{
	return this->data();
}

/// Assigns data to this blob.
template<size_t SZ>
CFieldFixBlob<SZ>& CFieldFixBlob<SZ>::operator =(const uint8* data)
{
	return this->assign(data, SZ);
}
template<size_t SZ>
CFieldFixBlob<SZ>& CFieldFixBlob<SZ>::assign(const uint8* data, size_t sz)
{
	if( this->_h )
	{
		uint8* buf = this->_h->data(this->_id);
		if( sz < SZ )
		{
			memcpy(buf, data, sz);
			memset(buf + sz, 0, SZ - sz);
		}
		else
			memcpy(buf, data, SZ);
	}
	return *this;
}

/// Returns the length of this blob.
template<size_t SZ>
size_t CFieldFixBlob<SZ>::size() const
{
	return SZ;
}
template<size_t SZ>
size_t CFieldFixBlob<SZ>::length() const
{
	return SZ;
}

/// Resizes this blob.
template<size_t SZ>
bool CFieldFixBlob<SZ>::resize(size_t len)
{
	return false;// not supported
}



///////////////////////////////////////////////////////////////////////////////
/// Dynamic-size blob of data.
///
/// @param SZ Size of the string
class CFieldDynBlob : public AField
{
public:
	CFieldDynBlob();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length the field can have
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t maxlen=0);

	/// Array access.
	const uint8& operator [](size_t idx) const;
	uint8& operator [](size_t idx);

	/// List access.
	const uint8& operator ()(size_t idx) const;
	uint8& operator ()(size_t idx);

	/// Converts this blob to data.
	operator const uint8*() const;
	operator uint8*();

	/// Returns the data of this blob.
	const uint8* data() const;
	uint8* data();
	const uint8* operator ()() const;
	uint8* operator ()();

	/// Assigns data to this blob.
	CFieldDynBlob& operator =(const CFieldDynBlob& f);
	CFieldDynBlob& assign(const uint8* data, size_t sz);

	/// Returns the length of this blob.
	size_t size() const;
	size_t length() const;

	/// Resizes this blob.
	bool resize(size_t len);
};



///////////////////////////////////////////////////////////////////////////////
/// Array with a fixed number of sub-fields.
/// Can ONLY be used in offset-based packets.
/// Must contain fixed-size sub-fields only.
///
/// @param T Type of sub-field
/// @param SZ Number of sub-fields
template<class T,size_t SZ>
class CFieldFixOffArray : public AField
{
public:
	CFieldFixOffArray();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param h Parent field handler (COffFieldHandler)
	/// @param id Offset in the parent field handler
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0);

	/// Array access.
	const T& operator [](size_t idx) const;
	T& operator [](size_t idx);

	/// List access.
	const T& operator ()(size_t idx) const;
	T& operator ()(size_t idx);

	/// Returns the number of items.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity.
	size_t capacity() const;

	/// Resizes this array.
	bool resize(size_t len);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	T _arr[SZ];
};

template<class T,size_t SZ>
CFieldFixOffArray<T,SZ>::CFieldFixOffArray()
:	AField()
{}

/// Initializes the field.
///
/// @param len Maximum length of this field
template<class T,size_t SZ>
void CFieldFixOffArray<T,SZ>::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_h = h;
	this->_id = id;
#if defined(DEBUG)
	if( off != 0 )
		printf("[Debug] unexpected argument off=%u at CFieldFixOffArray::Init(%p,%u,%u,%u)\n", (uint)off, h, (uint)id, (uint)off, (uint)len);
#endif

	if( h )
	{
		// get minimum field length
		NFieldHandler::CSetupFieldHandler field;
		this->_arr[0].Init(&field, 0, 0, 0);

		// initialize sub-fields
		size_t i;
		for( i = 0; i < SZ; ++i )
			this->_arr[i].Init(h, id + i*field.len, 0, 0);
	}
}

/// Array access.
template<class T,size_t SZ>
const T& CFieldFixOffArray<T,SZ>::operator [](size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldFixOffArray<T,SZ>::operator [](size_t idx)
{
	return this->_arr[idx];
}

/// List access.
template<class T,size_t SZ>
const T& CFieldFixOffArray<T,SZ>::operator ()(size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldFixOffArray<T,SZ>::operator ()(size_t idx)
{
	return this->_arr[idx];
}

/// Returns the number of items.
template<class T,size_t SZ>
size_t CFieldFixOffArray<T,SZ>::size() const
{
	return SZ;
}
template<class T,size_t SZ>
size_t CFieldFixOffArray<T,SZ>::length() const
{
	return SZ;
}

/// Returns the capacity.
template<class T,size_t SZ>
size_t CFieldFixOffArray<T,SZ>::capacity() const
{
	return SZ;
}

/// Resizes this array.
template<class T,size_t SZ>
bool CFieldFixOffArray<T,SZ>::resize(size_t len)
{
	return false;// not supported
}



///////////////////////////////////////////////////////////////////////////////
/// Array with a limited number of sub-fields.
/// Can ONLY be used as the last field in offset-based packets.
/// Must contain fixed-size sub-fields only.
///
/// @param T Type of sub-field
/// @param SZ Maximum number of sub-fields
template<class T,size_t SZ>
class CFieldLimOffArray : public AField
{
public:
	CFieldLimOffArray();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0);

	/// Array access.
	const T& operator [](size_t idx) const;
	T& operator [](size_t idx);

	/// List access.
	const T& operator ()(size_t idx) const;
	T& operator ()(size_t idx);

	/// Returns the number of items.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity.
	size_t capacity() const;

	/// Resizes this array.
	bool resize(size_t len);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	T _arr[SZ];

	/// Length of this array
	size_t _len;
};

template<class T,size_t SZ>
CFieldLimOffArray<T,SZ>::CFieldLimOffArray()
:	AField()
{}

/// Initializes the field.
///
/// @param len Maximum length of this field
template<class T,size_t SZ>
void CFieldLimOffArray<T,SZ>::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_h = h;
	this->_id = id;
#if defined(DEBUG)
	if( off != 0 )
		printf("[Debug] unexpected argument off=%u at CFieldLimOffArray::Init(%p,%u,%u,%u)\n", (uint)off, h, (uint)id, (uint)off, (uint)len);
#endif

	if( h )
	{
		// get field length
		NFieldHandler::CSetupFieldHandler field;
		this->_arr[0].Init(&field, 0, 0, 0);

		// clear sub-fields
		size_t i;
		for( i = 0; i < SZ; ++i )
			this->_arr[i].Init(NULL, 0, 0, 0);

		// resize array
		this->_len = len / field.len;

		// initialize sub-fields
		for( i = 0; i < this->length(); ++i )
			this->_arr[i].Init(h, id + i*field.len, 0, 0);
	}
}

/// Array access.
template<class T,size_t SZ>
const T& CFieldLimOffArray<T,SZ>::operator [](size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldLimOffArray<T,SZ>::operator [](size_t idx)
{
	return this->_arr[idx];
}

/// List access.
template<class T,size_t SZ>
const T& CFieldLimOffArray<T,SZ>::operator ()(size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldLimOffArray<T,SZ>::operator ()(size_t idx)
{
	return this->_arr[idx];
}

/// Returns the number of items.
template<class T,size_t SZ>
size_t CFieldLimOffArray<T,SZ>::size() const
{
	return this->_len;
}
template<class T,size_t SZ>
size_t CFieldLimOffArray<T,SZ>::length() const
{
	return this->_len;
}

/// Returns the capacity.
template<class T,size_t SZ>
size_t CFieldLimOffArray<T,SZ>::capacity() const
{
	return SZ;
}

/// Resizes this array.
template<class T,size_t SZ>
bool CFieldLimOffArray<T,SZ>::resize(size_t len)
{
	if( len > this->capacity() )
		return false;// not enough array space

	if( this->_h == NULL )
		return false;// no field handler

	// get field length
	NFieldHandler::CSetupFieldHandler field;
	this->_arr[0].Init(&field, 0, 0, 0);
	this->_arr[0].Init(this->_h, this->_id, 0, 0);

	// resize buffer
	size_t out;
	if( !this->_h->resize(this->_id, len*field.len, out) )
		return false;// failed - not enough buffer space

	// update sub-fields
	size_t i;
	size_t id;
	for( i = 0, id = this->_id; i < len; ++i, id += field.len )
		this->_arr[i].Init(this->_h, id, 0, 0);
	for( ; i < this->length(); ++i )
		this->_arr[i].Init(NULL, 0, 0, 0);

	this->_len = len;
	return true;
}



///////////////////////////////////////////////////////////////////////////////
/// Array with a dynamic number of sub-fields.
/// Can ONLY be used as the last field in offset-based packets.
/// Must contain fixed-size sub-fields only.
///
/// @param T Type of sub-field
template<class T>
class CFieldDynOffArray : public AField
{
public:
	CFieldDynOffArray();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0);

	/// Array access.
	const T& operator [](size_t idx) const;
	T& operator [](size_t idx);

	/// List access.
	const T& operator ()(size_t idx) const;
	T& operator ()(size_t idx);

	/// Returns the number of items.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity.
	size_t capacity() const;

	/// Resizes this array.
	bool resize(size_t len);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	basics::vector<T> _arr;
};

template<class T>
CFieldDynOffArray<T>::CFieldDynOffArray()
:	AField()
{}

/// Initializes the field.
///
/// @param len Maximum length of this field
template<class T>
void CFieldDynOffArray<T>::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_h = h;
	this->_id = id;
#if defined(DEBUG)
	if( off != 0 )
		printf("[Debug] unexpected argument off=%u at CFieldDynOffArray::Init(%p,%u,%u,%u)\n", (uint)off, h, (uint)id, (uint)off, (uint)len);
#endif

	if( h )
	{
		// get field length
		NFieldHandler::CSetupFieldHandler field;
		if( this->length() == 0 )
			this->_arr.resize(1);
		this->_arr[0].Init(&field, 0, 0, 0);

		// clear sub-fields
		size_t i;
		for( i = 0; i < this->length(); ++i )
			this->_arr[i].Init(NULL, 0, 0, 0);

		// resize array
		this->_arr.resize(len / field.len);

		// initialize sub-fields
		for( i = 0; i < this->length(); ++i )
			this->_arr[i].Init(h, id + i*field.len, 0, 0);
	}
}

/// Array access.
template<class T>
const T& CFieldDynOffArray<T>::operator [](size_t idx) const
{
	return this->_arr[idx];
}
template<class T>
T& CFieldDynOffArray<T>::operator [](size_t idx)
{
	return this->_arr[idx];
}

/// List access.
template<class T>
const T& CFieldDynOffArray<T>::operator ()(size_t idx) const
{
	return this->_arr[idx];
}
template<class T>
T& CFieldDynOffArray<T>::operator ()(size_t idx)
{
	return this->_arr[idx];
}

/// Returns the number of items.
template<class T>
size_t CFieldDynOffArray<T>::size() const
{
	return this->length();
}
template<class T>
size_t CFieldDynOffArray<T>::length() const
{
	return this->_arr.length();
}

/// Returns the capacity.
template<class T>
size_t CFieldDynOffArray<T>::capacity() const
{
	return this->length();
}

/// Resizes this array.
template<class T>
bool CFieldDynOffArray<T>::resize(size_t len)
{
	if( this->_h == NULL )
		return false;// no field handler

	// get field length
	NFieldHandler::CSetupFieldHandler field;
	if( this->length() == 0 )
	{
		this->_arr.resize(1);
		this->_arr[0].Init(&field, 0, 0, 0);
		this->_arr[0].Init(NULL, 0, 0, 0);
		this->_arr.resize(0);
	}
	else
	{
		this->_arr[0].Init(&field, 0, 0, 0);
		this->_arr[0].Init(this->_h, this->_id, 0, 0);
	}

	// clear extra sub-fields
	size_t i;
	for( i = len; i < this->length(); ++i )
		this->_arr[i].Init(NULL, 0, 0, 0);

	// resize buffer
	size_t out;
	if( !this->_h->resize(this->_id, len*field.len, out) )
	{// failed - not enough buffer space
		// initialize extra sub-fields
		for( i = len; i < this->length(); ++i )
			this->_arr[i].Init(this->_h, this->_id + i*field.len, 0, 0);
		return false;
	}

	// resize array
	this->_arr.resize(len);

	// initialize sub-fields
	for( i = 0; i < this->length(); ++i )
		this->_arr[i].Init(this->_h, this->_id + i*field.len, 0, 0);

	return true;
}



///////////////////////////////////////////////////////////////////////////////
/// Array with a fixed number of sub-fields.
///
/// @param T Type of sub-field
/// @param SZ Number of sub-fields
template<class T,size_t SZ>
class CFieldFixIdxArray : public ACompositeIdxField<NFieldHandler::CFixPosSet<SZ> >
{
public:
	CFieldFixIdxArray();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0);

	/// Array access.
	const T& operator [](size_t idx) const;
	T& operator [](size_t idx);

	/// List access.
	const T& operator ()(size_t idx) const;
	T& operator ()(size_t idx);

	/// Returns the number of items.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity.
	size_t capacity() const;

	/// Resizes this array.
	bool resize(size_t len);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	T _arr[SZ];
};

template<class T,size_t SZ>
CFieldFixIdxArray<T,SZ>::CFieldFixIdxArray()
:	ACompositeIdxField<NFieldHandler::CFixPosSet<SZ> >()
{}

/// Initializes the field.
///
/// @param len Maximum length of this field
template<class T,size_t SZ>
void CFieldFixIdxArray<T,SZ>::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_h.Init(h, id, off, len);// gain access to data
	if( h )
	{
		// initialize sub-fields
		size_t i;
		size_t suboff = 0;
		for( i = 0; i < SZ; ++i )
		{
			this->_arr[i].Init(&this->_h, i, suboff, len - basics::min<size_t>(suboff, len));
			suboff += this->_h.length(i);
		}
		// adjust to the total occupied length
		this->_h.normalize();
	}
}

/// Array access.
template<class T,size_t SZ>
const T& CFieldFixIdxArray<T,SZ>::operator [](size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldFixIdxArray<T,SZ>::operator [](size_t idx)
{
	return this->_arr[idx];
}

/// List access.
template<class T,size_t SZ>
const T& CFieldFixIdxArray<T,SZ>::operator ()(size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldFixIdxArray<T,SZ>::operator ()(size_t idx)
{
	return this->_arr[idx];
}

/// Returns the number of items.
template<class T,size_t SZ>
size_t CFieldFixIdxArray<T,SZ>::size() const
{
	return SZ;
}
template<class T,size_t SZ>
size_t CFieldFixIdxArray<T,SZ>::length() const
{
	return SZ;
}

/// Returns the capacity.
template<class T,size_t SZ>
size_t CFieldFixIdxArray<T,SZ>::capacity() const
{
	return SZ;
}

/// Resizes this array.
template<class T,size_t SZ>
bool CFieldFixIdxArray<T,SZ>::resize(size_t len)
{
	return false;// not supported
}



///////////////////////////////////////////////////////////////////////////////
/// Array with a limited number of sub-fields.
///
/// @param T Type of sub-field
/// @param SZ Maximum number of sub-fields
template<class T,size_t SZ>
class CFieldLimIdxArray : public ACompositeIdxField<NFieldHandler::CFixPosSet<SZ> >
{
public:
	CFieldLimIdxArray();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0);

	/// Array access.
	const T& operator [](size_t idx) const;
	T& operator [](size_t idx);

	/// List access.
	const T& operator ()(size_t idx) const;
	T& operator ()(size_t idx);

	/// Returns the number of items.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity.
	size_t capacity() const;

	/// Resizes this array.
	bool resize(size_t len);

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	T _arr[SZ];

	/// Length of this array
	size_t _len;
};

template<class T,size_t SZ>
CFieldLimIdxArray<T,SZ>::CFieldLimIdxArray()
:	ACompositeIdxField<NFieldHandler::CFixPosSet<SZ> >()
{}

/// Initializes the field.
///
/// @param len Maximum length of this field
template<class T,size_t SZ>
void CFieldLimIdxArray<T,SZ>::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_h.Init(h, id, off, len);// gain access to data
	if( h )
	{
		// get minimum field length
		NFieldHandler::CSetupFieldHandler min;
		this->_arr[0].Init(&min, 0, 0, 0);
		this->_arr[0].Init(NULL, 0, 0, 0);

		// initialize sub-fields
		size_t i;
		size_t suboff = 0;
		for( i = 0; i < SZ && suboff + min.len <= len; ++i )
		{
			this->_arr[i].Init(&this->_h, i, suboff, len - suboff);
			if( this->_h.length(i) == 0 )
				break;// no length - stop (and clear the field)
			suboff += this->_h.length(i);
		}
		this->_len = i;

		// clear the rest of the sub-fields
		for( ; i < SZ; ++i )
		{
			this->_arr[i].Init(NULL, i, 0, 0);
			this->_h.setup(i, 0, 0);
		}

		// adjust to the total occupied length
		this->_h.normalize();
	}
}

/// Array access.
template<class T,size_t SZ>
const T& CFieldLimIdxArray<T,SZ>::operator [](size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldLimIdxArray<T,SZ>::operator [](size_t idx)
{
	return this->_arr[idx];
}

/// List access.
template<class T,size_t SZ>
const T& CFieldLimIdxArray<T,SZ>::operator ()(size_t idx) const
{
	return this->_arr[idx];
}
template<class T,size_t SZ>
T& CFieldLimIdxArray<T,SZ>::operator ()(size_t idx)
{
	return this->_arr[idx];
}

/// Returns the number of items.
template<class T,size_t SZ>
size_t CFieldLimIdxArray<T,SZ>::size() const
{
	return this->_len;
}
template<class T,size_t SZ>
size_t CFieldLimIdxArray<T,SZ>::length() const
{
	return this->_len;
}

/// Returns the capacity.
template<class T,size_t SZ>
size_t CFieldLimIdxArray<T,SZ>::capacity() const
{
	return SZ;
}

/// Resizes this array.
template<class T,size_t SZ>
bool CFieldLimIdxArray<T,SZ>::resize(size_t len)
{
	if( len > this->capacity() )
		return false;// not enough space

	if( len > this->length() )
	{// increasing
		size_t id = this->length();
		size_t off = this->_h.length();

		// get minimum field length
		NFieldHandler::CSetupFieldHandler min;
		this->_arr[id].Init(&min, 0, 0, 0);
		this->_arr[id].Init(NULL, 0, 0, 0);

		// resize buffer
		this->_h.setup(id, off, 0);
		size_t out;
		if( !this->_h.resize(id, (len - id)*min.len, out) )
		{// failed - not enough buffer space
			this->_h.setup(id, 0, 0);
			return false;
		}

		// update sub-fields
		for( ; id < len; ++id )
		{
			this->_arr[id].Init(&this->_h, id, off, this->_min);
			off += this->_min;
		}
	}
	else if( len < this->length() )
	{// decreasing
		size_t id = len;
		size_t id_off = this->_h.offset(id);
		size_t id_len = this->_h.length(id);

		// resize buffer
		size_t out;
		this->_h.setup(id, id_off, this->_h.length() - id_off);
		if( !this->_h.resize(id, 0, out) )
		{// failed - ???unknown cause
#if defined(DEBUG)
			printf("[Debug] failed to resize (length=%u) at CFieldLimArray<T,SZ>::resize(%u)\n", (uint)this->length(), (uint)len);
#endif
			this->_h.setup(id, id_off, id_len);
			return false;
		}

		// update sub-fields
		for( ; id < this->length(); ++id )
			this->_arr[id].Init(NULL, id, 0, 0);
	}
	this->_len = len;
	return true;
}



///////////////////////////////////////////////////////////////////////////////
/// Array with a dynamic number of sub-fields.
///
/// @param T Type of sub-field
template<class T>
class CFieldDynIdxArray : public ACompositeIdxField<NFieldHandler::CDynPosSet>
{
public:
	CFieldDynIdxArray();

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0);

	/// Array access.
	const T& operator [](size_t idx) const;
	T& operator [](size_t idx);

	/// List access.
	const T& operator ()(size_t idx) const;
	T& operator ()(size_t idx);

	/// Returns the number of items.
	size_t size() const;
	size_t length() const;

	/// Returns the capacity.
	size_t capacity() const;

	/// Resizes this array.
	bool resize(size_t len);

private:
	/// Reconnects the fields in this array.
	/// Call this after setting up _h and resizing _arr.
	void _reconnect();

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	basics::vector<T> _arr;
};

template<class T>
CFieldDynIdxArray<T>::CFieldDynIdxArray()
:	ACompositeIdxField<NFieldHandler::CDynPosSet>()
{}

/// Initializes the field.
///
/// @param len Maximum length of this field
template<class T>
void CFieldDynIdxArray<T>::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_h.Init(h, id, off, len);// gain access to data
	if( h )
	{
		// get minimum field length
		NFieldHandler::CSetupFieldHandler min;
		if( this->length() == 0 )
			this->_arr.resize(1);
		this->_arr[0].Init(&min, 0, 0, 0);
		this->_arr[0].Init(NULL, 0, 0, 0);

		// initialize sub-fields
		size_t i;
		size_t suboff = 0;
		for( i = 0; suboff + min.len <= len; ++i )
		{
			if( this->length() < i + 1 )
				this->_arr.resize(i + 1);
			this->_arr[i].Init(&this->_h, i, off, len - off);
			if( this->_h.length(i) == 0 )
				break;// no length - stop (and clear the field)
			suboff += this->_h.length(i);
		}

		// clear the rest of the sub-fields
		while( this->length() > i )
		{
			this->_arr[i].Init(NULL, i, 0, 0);
			this->_h.setup(i, 0, 0);
			this->_arr.resize(this->length() - 1);
		}

		// adjust to the total occupied length
		this->_h.normalize();
		this->_reconnect();
	}
}

/// Array access.
template<class T>
const T& CFieldDynIdxArray<T>::operator [](size_t idx) const
{
	return this->_arr[idx];
}
template<class T>
T& CFieldDynIdxArray<T>::operator [](size_t idx)
{
	return this->_arr[idx];
}

/// List access.
template<class T>
const T& CFieldDynIdxArray<T>::operator ()(size_t idx) const
{
	return this->_arr[idx];
}
template<class T>
T& CFieldDynIdxArray<T>::operator ()(size_t idx)
{
	return this->_arr[idx];
}

/// Returns the number of items.
template<class T>
size_t CFieldDynIdxArray<T>::size() const
{
	return this->length();
}
template<class T>
size_t CFieldDynIdxArray<T>::length() const
{
	return this->_arr.length();
}

/// Returns the capacity.
template<class T>
size_t CFieldDynIdxArray<T>::capacity() const
{
	return this->length();
}

/// Resizes this array.
template<class T>
bool CFieldDynIdxArray<T>::resize(size_t len)
{
	if( len > this->length() )
	{// increasing
		size_t id = this->length();
		size_t off = this->_h.length();

		// get minimum field length
		NFieldHandler::CSetupFieldHandler min;
		this->_arr.resize(id + 1);
		this->_arr[id].Init(&min, 0, 0, 0);
		this->_arr[id].Init(NULL, 0, 0, 0);

		// resize buffer
		this->_h.setup(id, off, 0);
		size_t out;
		if( !this->_h.resize(id, (len - id)*min.len, out) )
		{// failed - not enough buffer space
			this->_h.setup(id, 0, 0);
			this->_arr.resize(id);
			this->_reconnect();
			return false;
		}

		// update sub-fields
		this->_arr.resize(len);
		for( ; id < len; ++id )
		{
			this->_h.setup(id, off, min.len);
			off += min.len;
		}
		this->_reconnect();
	}
	else if( len < this->length() )
	{// decreasing
		size_t id = len;
		size_t id_off = this->_h.offset(id);
		size_t id_len = this->_h.length(id);

		// resize buffer
		size_t out;
		this->_h.setup(id, id_off, this->_h.length() - id_off);
		if( !this->_h.resize(id, 0, out) )
		{// failed - ???unknown cause
#if defined(DEBUG)
			printf("[Debug] failed to resize (length=%u) at CFieldLimArray<T,SZ>::resize(%u)\n", (uint)this->length(), (uint)len);
#endif
			this->_h.setup(id, id_off, id_len);
			return false;
		}

		// update sub-fields
		for( ; id < this->length(); ++id )
			this->_arr[id].Init(NULL, id, 0, 0);
		this->_arr.resize(len);
		this->_reconnect();
	}
	return true;
}

/// Reconnects the fields in this array.
/// Call this after setting up _h and resizing _arr.
template<class T>
void CFieldDynIdxArray<T>::_reconnect()
{
	size_t id;
	for( id = 0; id < this->length(); ++id )
	{
		size_t old_len = this->_h.length(id);
		this->_arr[id].Init(&this->_h, id, this->_h.offset(id), old_len);
#if defined(DEBUG)
		if( old_len != this->_h.length(id) )
			printf("[Debug] sub-field length changed (%u -> %u) at CFieldDynArray::_reconnect()\n", old_len, this->_h.length(id));
#endif
	}
}





///////////////////////////////////////////////////////////////////////////////
}// end namespace NSocket
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
	virtual uint8* aquire(size_t pos, size_t sz)=0;
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
	uint8 data[SZ];	///< the buffer
	///////////////////////////////////////////////////////////////////////////
	/// aquire a block of sz bytes starting from pos inside the buffer.
	/// return NULL when accessing out-of-range, overloadable
	virtual uint8* aquire(size_t pos, size_t sz)
	{
		return (pos+sz<=this->length())?const_cast<uint8*>(this->operator()()+pos):NULL;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	virtual ~packet_fixed()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// Length of the packet
	virtual size_t length() const			{ return SZ; }
	///////////////////////////////////////////////////////////////////////////
	/// Data of the packet
	virtual const uint8* operator()() const	{ return this->data; }
};



///////////////////////////////////////////////////////////////////////////////
/// dynamic size packet.
class packet_dynamic : public packet
{
protected:
	basics::vector<uint8> data;	///< the buffer
	///////////////////////////////////////////////////////////////////////////
	/// aquire a block of sz bytes starting from pos inside the buffer.
	/// resize the vector when accessing out-of-range
	virtual uint8* aquire(size_t pos, size_t sz)
	{
		if( pos+sz>this->data.size() )
			this->data.resize(pos+sz);
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


} // end namespace example_code


///////////////////////////////////////////////////////////////////////////////

#endif//__BASEPACKET_H__


