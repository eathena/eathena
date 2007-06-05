#ifndef __BASEPACKET_H__
#define __BASEPACKET_H__

#include "basetypes.h"
#include "basearray.h"


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
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

	TESTING

	CFieldFixBlob - fixed-size blob of data
	CFieldDynBlob - fixed-size blob of data
	CFieldCString - nul-terminated dynamic-size string
	CFieldFixArray - array with a fixed number of sub-fields
	CFieldLimArray - array with a limited number of sub-fields
	CFieldDynArray - array with a dynamic number of sub-fields
	CLimPacket - dynamic-size packet with a fixed-size buffer
	CDynPacket - dynamic-size packet

*/



/// test packet functions
void test_packet(void);

/// The client can handle packet sizes up to 20480, 20k.
#define CLIENTPACKET_MAX 20480





///////////////////////////////////////////////////////////////////////////////
// Field handler
///////////////////////////////////////////////////////////////////////////////
namespace NFieldHandler {



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

	/// Returns the data of the target id.
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

	// Forward id iteration
	size_t start_id() const;
	size_t next_id(size_t id) const;
	size_t end_id() const;

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of positions
	SPos _arr[SZ];
};



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

	// Forward id iteration
	size_t start_id() const;
	size_t next_id(size_t id) const;
	size_t end_id() const;

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of positions
	vector<SPos> _arr;
};



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
	vector<uint8> _buf;
};



/// Buffer backed by a field handler.
class CChildBuffer
{
public:
	CChildBuffer()
	:	_h(NULL)
	{}
	void Init(IFieldHandler* h, size_t id, size_t off, size_t len)
	{
		this->_id = id;
		this->_h = h;
		if( h )
			h->setup(id, off, len);
	}
	const uint8* data(size_t off=0) const
	{
		if( this->_h )
			return this->_h->data(this->_id) + off;
		return NULL;
	}
	uint8* data(size_t off=0)
	{
		if( this->_h )
			return this->_h->data(this->_id) + off;
		return NULL;
	}
	size_t length() const
	{
		if( this->_h )
			return this->_h->length(this->_id);
		return 0;
	}
	size_t capacity() const
	{
		if( this->_h )
			return this->_h->capacity(this->_id);
		return 0;
	}
	bool insert(size_t off, size_t n, size_t& out)
	{
		if( this->_h )
			return this->_h->insert(this->_id, off, n, out);
		out = 0;
		return false;
	}
	bool remove(size_t off, size_t n, size_t& out)
	{
		if( this->_h )
			return this->_h->remove(this->_id, off, n, out);
		out = 0;
		return false;
	}
	bool resize(size_t sz, size_t& out)
	{
		if( this->_h )
			return this->_h->resize(this->_id, sz, out);
		return false;
	}
	size_t id() const
	{
		return this->_id;
	}
	IFieldHandler* parent()
	{
		return this->_h;
	}
private:
	size_t _id;
	IFieldHandler* _h;
};



/// Field handler where ids are indexes in a position array.
///
/// @param P Positions
/// @param B Buffer
template<class P, class B>
class CIdxFieldHandler : public IFieldHandler
{
public:
	CIdxFieldHandler()
	:	_pos()
	,	_buf()
	{}
	CIdxFieldHandler(const uint8* buf, size_t sz)
	:	_pos()
	,	_buf(buf, sz)
	{}
	virtual ~CIdxFieldHandler()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Sets the offset and length of the target id.
	virtual void setup(size_t id, size_t off, size_t len)
	{
		if( !this->_pos.contains(id) && !this->_pos.add(id) )
			return;
		this->_pos(id).off = off;
		this->_pos(id).len = len;
		size_t end = off + len;
		if( this->_buf.length() < end )
		{
			if( !this->_buf.resize(end, end) )
			{
				//## critical error, bad packet definition?
				printf("[Debug] resize failed - bad packet definition? at CIdxFieldHandler::setup(%u,%u,%u)\n", (uint)id, (uint)off, (uint)len);
				this->_pos(id).off = 0;
				this->_pos(id).len = 0;
			}
		}
	}

	/// Returns the data of the target id.
	virtual const uint8* data(size_t id) const
	{
		if( !this->_pos.contains(id) )
			return NULL;
		return this->_buf.data(_pos(id).off);
	}

	/// Returns the data of the target id.
	virtual uint8* data(size_t id)
	{
		if( !this->_pos.contains(id) )
			return NULL;
		return this->_buf.data(_pos(id).off);
	}

	/// Returns the offset of the target id.
	virtual size_t offset(size_t id) const
	{
		if( !this->_pos.contains(id) )
			return 0;
		return this->_pos(id).off;
	}

	/// Returns the length of the target id.
	virtual size_t length(size_t id) const
	{
		if( !this->_pos.contains(id) )
			return 0;
		return this->_pos(id).len;
	}

	/// Returns the capacity of the target id.
	virtual size_t capacity(size_t id) const
	{
		if( !this->_pos.contains(id) )
			return 0;
		return this->_pos(id).len + this->_buf.capacity() - this->_buf.length();
	}

	/// Inserts n bytes in the offset of the target id.
	/// On success, out contains n.
	/// On failure, out contains the max it can grow.
	virtual bool insert(size_t id, size_t off, size_t n, size_t& out)
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
	virtual bool remove(size_t id, size_t off, size_t n, size_t& out)
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
	virtual bool resize(size_t id, size_t sz, size_t& out)
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

	/// Returns the data of this buffer.
	const uint8* data() const
	{
		return this->_buf.data();
	}

	/// Returns the data of this buffer.
	uint8* data()
	{
		return this->_buf.data();
	}

	/// Returns the length of this buffer.
	size_t length() const
	{
		return this->_buf.length();
	}

protected:
	///////////////////////////////////////////////////////////////////////////
	/// Positions
	P _pos;

	/// Buffer
	B _buf;
};



/// Field buffer that uses another field buffer for data.
/// Ids are indexes in the position array.
///
/// @param P Positions
template<class P>
class CChildIdxFieldBuffer : public CIdxFieldHandler<P,CChildBuffer>
{
public:
	CChildIdxFieldBuffer()
	:	CIdxFieldHandler<P,CChildBuffer>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes this child buffer.
	void Init(IFieldHandler* h, size_t id, size_t off, size_t len)
	{
		this->_buf.Init(h, id, off, len);
	}

	/// Normalizes the size of this field buffer.
	/// Invoke after initializing fields and restricting size.
	void normalize()
	{
		size_t len = 0;
		size_t i;
		for( i = this->_pos.start_id(); i != this->_pos.end_id(); i = this->_pos.next_id(i) )
			len = max<size_t>(len, this->_pos(i));

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
	virtual void setup(size_t id, size_t off, size_t len)
	{
		if( !this->_pos.contains(id) && !this->_pos.add(id) )
			return;
		this->_pos(id).off = off;
		this->_pos(id).len = len;
	}
};



class CIdxAutoRegistry
{
public:
	CIdxAutoRegistry(IFieldHandler& h)
	:	_h(h)
	,	_idx(0)
	,	_off(0)
	{}

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
	/// Target field handler
	IFieldHandler& _h;
	/// Offset of the next field
	size_t _off;
	/// Index of the next field
	size_t _idx;
};




}// end namespace NFieldHandler
///////////////////////////////////////////////////////////////////////////////
// Packets
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



/// Fixed-size packet.
/// Must contain fixed-size fields only.
///
/// @param NUM Number of fields 
/// @param SZ Size of the buffer
template<size_t NUM,size_t SZ>
class CFixPacket : public APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >
{
protected:
	CFixPacket<NUM,SZ>()
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >(NULL, SZ)
	{}
	CFixPacket<NUM,SZ>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >(NULL, SZ)
	{
		if( buf )
		{
			if( sz > SZ )
				sz = SZ;
			memcpy(this->_h.data(), buf, sz);
		}
	}
};



/// Dynamic-size packet with a limited buffer.
///
/// @param NUM Number of fields 
/// @param SZ Size of the buffer
template<size_t NUM,size_t SZ>
class CLimPacket : public APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >
{
protected:
	CLimPacket<NUM,SZ>()
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >()
	{}
	CLimPacket<NUM,SZ>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CFixBuffer<SZ> > >(buf,sz)
	{}
};



/// Dynamic-size packet.
///
/// @param NUM Number of fields
template<size_t NUM>
class CDynPacket : public APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CDynBuffer> >
{
protected:
	CDynPacket<NUM>()
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CDynBuffer> >()
	{}
	CDynPacket<NUM>(const uint8* buf, size_t sz)
	:	APacket<NFieldHandler::CIdxFieldHandler<NFieldHandler::CFixPosSet<NUM>,NFieldHandler::CDynBuffer> >(buf,sz)
	{}
};





///////////////////////////////////////////////////////////////////////////////
// Fields
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



/// Abstract composite field
///
/// @param P Positions
template<class P>
class ACompositeField
{
protected:
	ACompositeField()
	:	_h()
	{}
	~ACompositeField()
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



/// Fixed-size typed field. Error for unsupported types.
template<typename T>
class CFixField
{};



typedef class CFixField<uint8> CFieldB;// Byte field
typedef class CFixField<uint16> CFieldW;// Word field
typedef class CFixField<uint32> CFieldL;// Long field



template<>
class CFixField<uint8> : public AFixField<1>
{
public:
	CFixField<uint8>()
	:	AFixField<1>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
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

	CFixField<uint8>& operator =(const CFixField<uint8>& f)
	{
		if( this->_h && f._h )
			this->_h->data(this->_id)[0] = f._h->data(f._id)[0];
		return *this;
	}
	uint8 operator =(uint8 val)
	{
		if( this->_h )
			this->_h->data(this->_id)[0] = val;
		return val;
	}
};



template<>
class CFixField<uint16> : public AFixField<2>
{
public:
	CFixField<uint16>()
	:	AFixField<2>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
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

	CFixField<uint16>& operator =(const CFixField<uint16>& f)
	{
		if( this->_h && f._h )
			memcpy(this->_h->data(this->_id), f._h->data(f._id), 2);
		return *this;
	}
	uint16 operator =(uint16 val)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			buf[0] = uint8( (val & 0x00FF)         );
			buf[1] = uint8( (val & 0xFF00) >> 0x08 );
		}
		return val;
	}
};



template<>
class CFixField<uint32> : public AFixField<4>
{
public:
	CFixField<uint32>()
	:	AFixField<4>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
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

	CFixField<uint32>& operator =(const CFixField<uint32>& f)
	{
		if( this->_h && f._h )
			memcpy(this->_h->data(this->_id), f._h->data(f._id), 4);
		return *this;
	}
	uint32 operator =(uint32 val)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			buf[0] = uint8( (val & 0x000000FF)         );
			buf[1] = uint8( (val & 0x0000FF00) >> 0x08 );
			buf[2] = uint8( (val & 0x00FF0000) >> 0x10 );
			buf[3] = uint8( (val & 0xFF000000) >> 0x18 );
		}
		return val;
	}
};



template<>
class CFixField<uint64> : public AFixField<8>
{
public:
	CFixField<uint64>()
	:	AFixField<8>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
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

	CFixField<uint64>& operator =(const CFixField<uint64>& f)
	{
		if( this->_h && f._h )
			memcpy(this->_h->data(this->_id), f._h->data(f._id), 8);
		return *this;
	}
	uint64 operator =(uint64 val)
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
		return val;
	}
};



/// Fixed-size string.
/// Warning: not nul-terminated if length equals capacity (SZ).
///
/// @param SZ Size of the string
template<size_t SZ>
class CFieldFixString : public AFixField<SZ>
{
public:
	CFieldFixString()
	:	AFixField<SZ>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	operator const char*() const
	{
		return this->operator ()();
	}
	const char* operator ()() const
	{
		if( this->_h )
			return (const char*)this->_h->data(this->_id);
		return "";
	}

	CFieldFixString<SZ>& operator =(const CFieldFixString<SZ>& f)
	{
		if( this->_h && f._h )
			memcpy(this->_h->data(this->_id), f._h->data(f._id), SZ);
		return *this;
	}
	const char* operator =(const char* str)
	{
		if( this->_h )
		{
			char* buf = (char*)this->_h->data(this->_id);
			strncpy(buf, str, SZ);
			return buf;
		}
		return "";
	}

	/// Copies the string with the specified size to this field.
	void copy(const char* str, size_t sz)
	{
		if( this->_h )
		{
			char* buf = (char*)this->_h->data(this->_id);
			if( sz < SZ )
			{
				strncpy(buf, str, sz);
				memset(buf + sz, 0, SZ - sz);
			}
			else
				strncpy(buf, str, SZ);
		}
	}

	/// Returns the length of this string.
	size_t length() const
	{
		return strnlen((const char*)this->_h->data(this->_id), SZ);
	}

	/// Returns the capacity of this field.
	size_t capacity() const
	{
		return SZ;
	}
};



/// Dynamic-size nul-terminated string.
class CFieldCString : public AField
{
public:
	CFieldCString()
	:	AField()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes this field.
	///
	/// @param len Maximum length of the field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0)
	{
		this->_id = id;
		this->_h = h;
		if( h )
		{
			// length based on contents
			if( len == 0 )
				h->setup(id, off, 1);// minimum length
			else if( len == 1 )
			{// nul-terminator
				h->setup(id, off, len);// get access to data
				char* data = (char*)this->_h->data(this->_id);
				data[0] = '\0';
			}
			else
			{// string length + nul-terminator
				h->setup(id, off, len);// get access to data
				char* data = (char*)this->_h->data(this->_id);
				size_t real_len = strnlen(data, len);
				if( real_len == len )
					data[real_len - 1] = '\0';
				else
					h->setup(id, off, real_len + 1);
			}
		}
	}
	operator const char*() const
	{
		return this->operator ()();
	}
	const char* operator ()() const
	{
		if( this->_h )
			return (const char*)this->_h->data(this->_id);
		return "";
	}

	CFieldCString& operator =(const CFieldCString& f)
	{
		if( this->_h && f._h )
		{
			size_t len;
			if( !this->_h->resize(this->_id, f._h->length(f._id), len) )
			{
#if defined(DEBUG)
				if( len == 0 )//## error, this should never happen
					printf("[Debug] resize failed (len=0) at CFieldCString::operator=(const CFieldCString& f)\n");
#endif//DEBUG
				this->_h->resize(this->_id, len, len);// resize to max instead
			}
			uint8* buf = this->_h->data(this->_id);
			memcpy(buf, f._h->data(f._id), len - 1);
			buf[len - 1] = 0;
		}
		return *this;
	}
	const char* operator =(const char* str)
	{
		if( this->_h )
		{
			if( !str )
				str = "";
			size_t len = strlen(str) + 1;
			if( !this->_h->resize(this->_id, len, len) )
			{
#if defined(DEBUG)
				if( len == 0 )//## error, this should never happen
					printf("[Debug] resize failed (len=0) at CFieldCString::operator=(const char* str)\n");
#endif//DEBUG
				this->_h->resize(this->_id, len, len);// resize to max instead
			}

			uint8* buf = this->_h->data(this->_id);
			memcpy(buf, str, len - 1);
			buf[len - 1] = '\0';
			return (const char*)buf;
		}
		return "";
	}

	/// Copies the string with at most the specified size to this field.
	void copy(const char* str, size_t sz)
	{
		if( this->_h )
		{
			if( str )
				str = "";
			size_t len = strlen(str);
			len = min(len, sz) + 1;
			if( !this->_h->resize(this->_id, len, len) )
			{
#if defined(DEBUG)
				if( len == 0 )//## error, this should never happen
					printf("[Debug] resize failed (len=0) at CFieldCString::copy(const char* str, size_t sz)\n");
#endif//DEBUG
				this->_h->resize(this->_id, len, len);// resize to max instead
			}
			char* buf = (char*)this->_h->data(this->_id);
			memcpy(buf, str, len - 1);
			buf[len - 1] = '\0';
		}
	}

	/// Returns the length of this string.
	size_t length() const
	{
		if( this->_h )
		{
			size_t len = this->_h->length(this->_id);
			if( len > 0 )
				return len - 1;
		}
		return 0;
	}

	/// Returns the capacity of this string.
	size_t capacity() const
	{
		if( this->_h )
		{
			size_t len = this->_h->capacity(this->_id);
			if( len > 0 )
				return len - 1;
		}
		return 0;
	}
};



/// Fixed-size blob of data.
///
/// @param SZ Size of the string
template<size_t SZ>
class CFieldFixBlob : public AFixField<SZ>
{
public:
	CFieldFixBlob()
	:	AFixField<SZ>()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	operator const uint8*() const
	{
		return this->data();
	}
	operator uint8*()
	{
		return this->data();
	}

	const uint8* operator ()() const
	{
		return this->data();
	}
	uint8* operator ()()
	{
		return this->data();
	}

	const uint8& operator [](size_t idx) const
	{
		return *(this->data() + idx);
	}
	uint8& operator [](size_t idx)
	{
		return *(this->data() + idx);
	}

	const uint8& operator ()(size_t idx) const
	{
		return *(this->data() + idx);
	}
	uint8& operator ()(size_t idx)
	{
		return *(this->data() + idx);
	}

	const uint8* data() const
	{
		return this->_h->data(this->_id);
	}
	uint8* data()
	{
		return this->_h->data(this->_id);
	}

	CFieldFixBlob<SZ>& operator =(const CFieldFixBlob<SZ>& f)
	{
		if( this->_h && f._h )
			memcpy(this->_h->data(this->_id), f._h->data(f._id), SZ);
		return *this;
	}

	const uint8* operator =(const uint8* data)
	{
		if( this->_h )
		{
			uint8* buf = this->_h->data(this->_id);
			strncpy(buf, data, SZ);
			return uint8;
		}
		return NULL;
	}

	/// Copies sz bytes from data to this field.
	void copy(uint8* data, size_t sz)
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
	}

	size_t size() const
	{
		return SZ;
	}
	size_t length() const
	{
		return SZ;
	}
	bool resize(size_t len)
	{
		return false;
	}
};



/// Fixed-size blob of data.
///
/// @param SZ Size of the string
class CFieldDynBlob : public AField
{
public:
	CFieldDynBlob()
	:	AField()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length the field can have
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t maxlen=0)
	{
		this->_id = id;
		this->_h = h;
		if( h )
			h->setup(id, off, maxlen);// take over all the data
	}

	operator const uint8*() const
	{
		return this->data();
	}
	operator uint8*()
	{
		return this->data();
	}

	const uint8* operator ()() const
	{
		return this->data();
	}
	uint8* operator ()()
	{
		return this->data();
	}

	const uint8& operator [](size_t idx) const
	{
		return *(this->data() + idx);
	}
	uint8& operator [](size_t idx)
	{
		return *(this->data() + idx);
	}

	const uint8& operator ()(size_t idx) const
	{
		return *(this->data() + idx);
	}
	uint8& operator ()(size_t idx)
	{
		return *(this->data() + idx);
	}

	const uint8* data() const
	{
		if( this->_h )
			return this->_h->data(this->_id);
		return NULL;
	}
	uint8* data()
	{
		if( this->_h )
			return this->_h->data(this->_id);
		return NULL;
	}

	CFieldDynBlob& operator =(const CFieldDynBlob& f)
	{
		if( this->_h && f._h )
		{
			size_t len;
			if( !this->_h->resize(this->_id, f.length(), len) )
				this->_h->resize(this->_id, len, len);// resize to the supported len
			if( len <= f.length() )
				memcpy(this->data(), f.data(), len);
			else//if( len > f.length() )
			{
				memcpy(this->data(), f.data(), f.length());
				memset(this->data() + f.length(), 0, len - f.length());
			}
		}
		return *this;
	}

	/// Copies sz bytes from data to this field.
	void assign(uint8* data, size_t sz)
	{
		if( this->_h )
		{
			size_t len;
			if( !this->_h->resize(this->_id, sz, len) )
				this->_h->resize(this->_id, len, len);// resize to the supported len
			if( len <= sz )
				memcpy(this->data(), data, len);
			else//if( len > sz )
			{
				memcpy(this->data(), data, sz);
				memset(this->data() + sz, 0, len - sz);
			}
		}
	}

	size_t size() const
	{
		return this->length();
	}
	size_t length() const
	{
		return this->_h->length(this->_id);
	}
	bool resize(size_t len)
	{
		if( this->_h )
			return this->_h->resize(this->_id, len, len);
		return false;
	}
};



/// Array with a fixed number of sub-fields.
///
/// @param T Type of sub-field
/// @param SZ Number of sub-fields
template<class T,size_t SZ>
class CFieldFixArray : public ACompositeField<NFieldHandler::CFixPosSet<SZ> >
{
public:
	CFieldFixArray()
	:	ACompositeField<NFieldHandler::CFixPosSet<SZ> >()
	{}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0)
	{
		if( h )
		{
			this->_h.Init(h, id, off, len);// gain access to data
			size_t i;
			size_t sum = 0;
			for( i = 0; i < SZ; ++i )
			{
				this->_arr[i].Init(&this->_h, i, sum, len - min(sum, len));
				sum += this->_h.length(i);
			}
			this->_h.normalize();// Adjust to the total occupied length
		}
		else
			this->_h.Init(NULL, id, 0, 0);
	}
	const T& operator [](size_t idx) const
	{
		return this->_arr[idx];
	}
	T& operator [](size_t idx)
	{
		return this->_arr[idx];
	}
	const T& operator ()(size_t idx) const
	{
		return this->_arr[idx];
	}
	T& operator ()(size_t idx)
	{
		return this->_arr[idx];
	}
	size_t size() const
	{
		return SZ;
	}
	size_t length() const
	{
		return SZ;
	}
	size_t capacity() const
	{
		return SZ;
	}

	bool insert(size_t idx, size_t count=1)
	{
		return false;// not supported
	}
	bool remove(size_t idx, size_t count=1)
	{
		return false;// not supported
	}
	bool resize(size_t len)
	{
		return false;// not supported
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	T _arr[SZ];
};



/// Array with a limited number of sub-fields.
///
/// @param T Type of sub-field
/// @param SZ Maximum number of sub-fields
template<class T,size_t SZ>
class CFieldLimArray : public ACompositeField<NFieldHandler::CFixPosSet<SZ> >
{
public:
	CFieldLimArray()
	:	ACompositeField<NFieldHandler::CFixPosSet<SZ> >()
	{
		// get minimum size of a field
		this->_arr[0].Init(&this->_h, 0, 0, 0);
		this->_min = this->_h.length(0);
		this->_arr[0].Init(NULL, 0, 0, 0);
		this->_h.setup(0, 0, 0);
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0)
	{
		if( h )
		{
			this->_h.Init(h, id, off, len);// gain access to data
			size_t i;
			size_t off = 0;
			for( i = 0; i < SZ && off + this->_min <= len; ++i )
			{
				this->_arr[i].Init(&this->_h, i, off, len - off);
				if( this->_h.length(i) == 0 )
				{// no length - remove the field and stop
					this->_arr[i].Init(NULL, i, 0, 0);
					this->_h.setup(i, 0, 0);
					break;
				}
				off += this->_h.length(i);
			}
			this->_len = i;
			this->_h.normalize();// Adjust to the total occupied length
		}
		else
			this->_h.Init(NULL, id, 0, 0);
	}

	const T& operator [](size_t idx) const
	{
		return this->_arr[idx];
	}
	T& operator [](size_t idx)
	{
		return this->_arr[idx];
	}
	const T& operator ()(size_t idx) const
	{
		return this->_arr[idx];
	}
	T& operator ()(size_t idx)
	{
		return this->_arr[idx];
	}

	size_t size() const
	{
		return this->_len;
	}
	size_t length() const
	{
		return this->_len;
	}
	size_t capacity() const
	{
		return SZ;
	}

	bool resize(size_t len)
	{
		if( len > this->capacity() )
			return false;// not enough space

		if( len > this->length() )
		{// increasing
			size_t id = this->length();
			size_t off = this->_h.length();

			// resize buffer
			this->_h.setup(id, off, 0);
			size_t out;
			if( !this->_h.resize(id, (len - id)*this->_min, out) )
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

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	T _arr[SZ];

	/// Length of this array
	size_t _len;

	/// Minimum size of a field
	size_t _min;
};



/// Array with a limited number of sub-fields.
///
/// @param T Type of sub-field
template<class T>
class CFieldDynArray : public ACompositeField<NFieldHandler::CDynPosSet>
{
public:
	CFieldDynArray()
	:	ACompositeField<NFieldHandler::CDynPosSet>()
	{
		// get minimum size of a field
		this->_arr.resize(1);
		this->_arr[0].Init(&this->_h, 0, 0, 0);
		this->_min = this->_h.length(0);
		this->_h.setup(0, 0, 0);
		this->_arr[0].Init(NULL, 0, 0, 0);
		this->_arr.resize(0);
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Initializes the field.
	///
	/// @param len Maximum length of this field
	void Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len=0)
	{
		if( h )
		{
			this->_h.Init(h, id, off, len);// gain access to data
			size_t i;
			size_t off = 0;

			for( i = 0; off + this->_min <= len; ++i )
			{
				this->_arr.resize(i + 1);
				this->_reconnect();
				this->_arr[i].Init(&this->_h, i, off, len - off);
				if( this->_h.length(i) == 0 )
				{// no length - remove the field and stop
					this->_arr[i].Init(NULL, i, 0, 0);
					this->_arr.resize(i);
					this->_h.setup(i, 0, 0);
					break;
				}
				off += this->_h.length(i);
			}
			this->_h.normalize();// Adjust to the total occupied length
			this->_reconnect();
		}
		else
			this->_h.Init(NULL, id, 0, 0);
	}

	const T& operator [](size_t idx) const
	{
		return this->_arr[idx];
	}
	T& operator [](size_t idx)
	{
		return this->_arr[idx];
	}
	const T& operator ()(size_t idx) const
	{
		return this->_arr[idx];
	}
	T& operator ()(size_t idx)
	{
		return this->_arr[idx];
	}

	size_t size() const
	{
		return this->length();
	}
	size_t length() const
	{
		return this->_arr.length();
	}
	size_t capacity() const
	{//## TODO capacity based on buffer capacity or just the length of _arr?
		return this->length();
	}

	bool resize(size_t len)
	{
		if( len > this->length() )
		{// increasing
			size_t id = this->length();
			size_t off = this->_h.length();

			// resize buffer
			this->_h.setup(id, off, 0);
			size_t out;
			if( !this->_h.resize(id, (len - id)*this->_min, out) )
			{// failed - not enough buffer space
				this->_h.setup(id, 0, 0);
				return false;
			}

			// update sub-fields
			this->_arr.resize(len);
			for( ; id < len; ++id )
			{
				this->_h.setup(id, off, this->_min);
				off += this->_min;
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

private:
	// Reconnects the fields in this array. Call this after setting up _h and resizing _arr.
	void _reconnect()
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

private:
	///////////////////////////////////////////////////////////////////////////
	/// Array of fields
	vector<T> _arr;

	/// Minimum size of a field
	size_t _min;
};





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


