#include "basepacket.h"



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// Fixed-size packets and fields
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// Field buffer with a fixed internal buffer.
// Does not support resizing or being managed by a field buffer controller.

template<size_t SZ>
CBufFieldBuffer<SZ>::CBufFieldBuffer()
{
	memset(_buf, 0, SZ);
}

/// Returns the length of this buffer.
template<size_t SZ>
size_t CBufFieldBuffer<SZ>::length() const
{
	return SZ;
}

/// Returns the data of this buffer.
template<size_t SZ>
uint8* CBufFieldBuffer<SZ>::buffer() const
{
	return (uint8*)_buf;
}

/// Returns false.
/// Resizing is not supported.
template<size_t SZ>
bool CBufFieldBuffer<SZ>::do_resize(size_t sz)
{
	return false;
}

/// Does nothing.
/// Resizing is not supported.
template<size_t SZ>
void CBufFieldBuffer<SZ>::length(size_t sz)
{}

/// Does nothing.
/// An internal buffer is always used.
template<size_t SZ>
void CBufFieldBuffer<SZ>::buffer(uint8* buf)
{}


///////////////////////////////////////////////////////////////////////////////
/// Fixed-size packet.

template<size_t SZ>
CFixPacket<SZ>::CFixPacket()
{}

template<size_t SZ>
CFixPacket<SZ>::CFixPacket(uint8* buf, size_t sz)
{
	if( buf )
		memcpy(_buf.buffer(), buf, (sz<SZ)?sz:SZ);
}

template<size_t SZ>
CFixPacket<SZ>::~CFixPacket()
{}

/// Returns the length of this packet.
template<size_t SZ>
size_t CFixPacket<SZ>::length() const
{ 
	return SZ;
}

/// Returns the data of this packet.
template<size_t SZ>
const uint8* CFixPacket<SZ>::buffer() const
{ 
	return _buf.buffer();
}


///////////////////////////////////////////////////////////////////////////////
// Byte field

CFieldB::CFieldB(IFieldBuffer& buf, size_t off)
:	_buf(buf)
,	_off(off)
{}

CFieldB::operator uint8() const
{
	return this->operator()();
}

uint8 CFieldB::operator()() const
{
	return _buf.buffer()[_off];
}

CFieldB& CFieldB::operator=(const CFieldB& f)
{
	_buf.buffer()[_off] = f._buf.buffer()[f._off];
	return *this;
}

uint8 CFieldB::operator=(uint8 val)
{
	_buf.buffer()[_off] = val;
	return val;
}


///////////////////////////////////////////////////////////////////////////////
// Word field

CFieldW::CFieldW(IFieldBuffer& buf, size_t off)
:	_buf(buf)
,	_off(off)
{}

CFieldW::operator uint16() const
{
	return this->operator()();
}

uint16 CFieldW::operator()() const
{
	uint8* buf = _buf.buffer() + _off;
	return
		( uint16(buf[0])         )|
		( uint16(buf[1]) << 0x08 );
}

CFieldW& CFieldW::operator=(const CFieldW& f)
{
	uint8* buf = _buf.buffer() + _off;
	uint8* fbuf = f._buf.buffer() + f._off;
	memcpy(buf, fbuf, 2);
	return *this;
}

uint16 CFieldW::operator=(uint16 val)
{
	uint8* buf = _buf.buffer() + _off;
	buf[0] = uint8( (val & 0x00FF)         );
	buf[1] = uint8( (val & 0xFF00) >> 0x08 );
	return val;
}


///////////////////////////////////////////////////////////////////////////////
// Long field

CFieldL::CFieldL(IFieldBuffer& buf, size_t off)
:	_buf(buf)
,	_off(off)
{}

CFieldL::operator uint32() const
{
	return this->operator()();
}

uint32 CFieldL::operator()() const
{
	uint8* buf = _buf.buffer() + _off;
	return
		( uint32(buf[0])         )|
		( uint32(buf[1]) << 0x08 )|
		( uint32(buf[2]) << 0x10 )|
		( uint32(buf[3]) << 0x18 );
}

CFieldL& CFieldL::operator=(const CFieldL& f)
{
	uint8* buf = _buf.buffer() + _off;
	uint8* fbuf = f._buf.buffer() + f._off;
	memcpy(buf, fbuf, 4);
	return *this;
}

uint32 CFieldL::operator=(uint32 val)
{
	uint8* buf = _buf.buffer() + _off;
	buf[0] = uint8( (val & 0x000000FF)         );
	buf[1] = uint8( (val & 0x0000FF00) >> 0x08 );
	buf[2] = uint8( (val & 0x00FF0000) >> 0x10 );
	buf[3] = uint8( (val & 0xFF000000) >> 0x18 );
	return val;
}


///////////////////////////////////////////////////////////////////////////////
// String field - fixed size

template<size_t SZ>
CFieldStringFix<SZ>::CFieldStringFix(IFieldBuffer& buf, size_t off)
:	_buf(buf)
,	_off(off)
{}

template<size_t SZ>
CFieldStringFix<SZ>::operator const char*()
{
	return this->operator()();
}

template<size_t SZ>
const char* CFieldStringFix<SZ>::operator()() const
{
	return (const char*)(_buf.buffer() + _off);
}

template<size_t SZ>
size_t CFieldStringFix<SZ>::length() const
{
	const char* str = (const char*)(_buf.buffer() + _off);
	return strnlen(str, SZ);
}

template<size_t SZ>
size_t CFieldStringFix<SZ>::capacity() const
{
	return SZ;
}

template<size_t SZ>
CFieldStringFix<SZ>& CFieldStringFix<SZ>::operator=(const CFieldStringFix<SZ>& f)
{
	char* buf = (char*)(_buf.buffer() + _off);
	char* str = (char*)(f._buf.buffer() + f._off);
	strncpy(buf, str, SZ);
	return *this;
}

template<size_t SZ>
const char* CFieldStringFix<SZ>::operator=(const char* str)
{
	char* buf = (char*)(_buf.buffer() + _off);
	strncpy(buf, str, SZ);
	return (const char*)buf;
}


///////////////////////////////////////////////////////////////////////////////
// Array of fields - fixed size (the fields can be dynamic)

template<class T, size_t TSZ, size_t LEN>
CFieldStaticArrayFix<T,TSZ,LEN>::CFieldStaticArrayFix(IFieldBuffer& buf, size_t off)
:	_buf(buf)
,	_off(off)
{}

/// Returns the element at the target index.
template<class T, size_t TSZ, size_t LEN>
T CFieldStaticArrayFix<T,TSZ,LEN>::operator[](size_t idx)
{
	//## TODO when idx is out of range? throw an expection?
	return T(_buf, _off + TSZ*idx);
}

/// Returns the number of elements in this array.
template<class T, size_t TSZ, size_t LEN>
size_t CFieldStaticArrayFix<T,TSZ,LEN>::length() const
{
	return LEN;
}





///////////////////////////////////////////////////////////////////////////////
// Dynamic packets and fields
///////////////////////////////////////////////////////////////////////////////





//## TODO





///////////////////////////////////////////////////////////////////////////////

class CTestFixPacket : public CFixPacket<20>
{
public:
	CTestFixPacket()
	:	CFixPacket<20>()
	,	b(_buf)
	,	w(_buf, 1)
	,	l(_buf, 3)
	,	str(_buf, 7)
	,	arr(_buf, 17)
	{ }
	CTestFixPacket(uint8* buf, size_t sz=20)
	:	CFixPacket<20>(buf,sz)
	,	b(_buf)
	,	w(_buf, 1)
	,	l(_buf, 3)
	,	str(_buf, 7)
	,	arr(_buf, 17)
	{}

	CFieldB b;
	CFieldW w;
	CFieldL l;
	CFieldStringFix<10> str;
	CFieldStaticArrayFix<CFieldB,1,3> arr;
};

void dump(IPacket& p)
{
	size_t i;
	for( i = 0; i < p.length(); i++ )
		printf("%02X ", p.buffer()[i]);
	printf("size=%u\n", (uint)p.length());
}

void test_packet(void)
{
#if defined(DEBUG)
	{// CFixPacket
		CTestFixPacket p;
		dump(p);
		p.b = 100;
		dump(p);
		p.w = 10000;
		dump(p);
		p.l = 100000000 + p.b + p.w;
		dump(p);
		p.str = "ABCD";
		dump(p);
		p.str = "1234567890 OVER!!!";
		dump(p);
		p.arr[0] = 1;
		dump(p);
		p.arr[1] = 2;
		dump(p);
		p.arr[2] = 3;
		dump(p);
	}
#endif//DEBUG
}


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////
