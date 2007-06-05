
#include "basepacket.h"


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// Field handler
///////////////////////////////////////////////////////////////////////////////
namespace NFieldHandler {





///////////////////////////////////////////////////////////////////////////////
// Fixed set of positions.

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

/// Iteration operators.
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
// Dynamic set of positions.

CDynPosSet::CDynPosSet()
{}

CDynPosSet::~CDynPosSet()
{}

/// Returns if id is included in this set.
bool CDynPosSet::contains(size_t id) const
{
	return (id < this->_arr.size());
}

/// Adds id to this set.
bool CDynPosSet::add(size_t id)
{
	if( id >= this->_arr.size() && !this->_arr.resize(id + 1) )
		return false;
	this->_arr[id].off = 0;
	this->_arr[id].len = 0;
	return true;
}

/// Array access.
const SPos& CDynPosSet::operator [](size_t id) const
{
	return this->_arr[id];
}
SPos& CDynPosSet::operator [](size_t id)
{
	return this->_arr[id];
}

/// List access.
const SPos& CDynPosSet::operator ()(size_t id) const
{
	return this->_arr(id);
}
SPos& CDynPosSet::operator ()(size_t id)
{
	return this->_arr(id);
}

/// Iteration operators.
size_t CDynPosSet::start_id() const
{
	return 0;
}
size_t CDynPosSet::next_id(size_t id) const
{
	return id + 1;
}
size_t CDynPosSet::end_id() const
{
	return this->_arr.size();
}



///////////////////////////////////////////////////////////////////////////////
// Buffer with a fixed size.

template<size_t SZ>
CFixBuffer<SZ>::CFixBuffer()
:	_len(0)
{}

template<size_t SZ>
CFixBuffer<SZ>::CFixBuffer(const uint8* buf, size_t sz)
:	_len(min(sz, SZ))
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
// Buffer with a dynamic size.

CDynBuffer::CDynBuffer()
:	_buf()
{}

CDynBuffer::CDynBuffer(const uint8* buf, size_t sz)
:	_buf()
{
	this->_buf.resize(sz);
	if( buf )
		memcpy(this->_buf.begin(), buf, sz);
	else
		memset(this->_buf.begin(), 0, sz);
}

CDynBuffer::~CDynBuffer()
{}

/// Returns the data of this buffer.
const uint8* CDynBuffer::data(size_t off) const
{
	return this->_buf.begin() + off;
}
uint8* CDynBuffer::data(size_t off)
{
	return this->_buf.begin() + off;
}

/// Returns the length of this buffer.
size_t CDynBuffer::length() const
{
	return this->_buf.length();
}

/// Returns the capacity of this buffer.
size_t CDynBuffer::capacity() const
{
	return this->_buf.capacity();
}

/// Inserts n bytes in off.
/// If successfull out contains n.
/// Otherwise out contains the maximum n that can inserted in off.
bool CDynBuffer::insert(size_t off, size_t n, size_t& out)
{
	size_t old_len = this->length();
	if( !this->_buf.resize(old_len + n) )
	{// failed
		out = this->capacity() - old_len;
		return false;
	}
	memset(this->data(old_len), 0, n);
	this->_buf.move(off, old_len, n);
	out = n;
	return true;
}

/// Removes n bytes from off. [off,off+n[
/// If successfull out contains n.
/// Otherwise out contains the maximum n that can be removed from off.
bool CDynBuffer::remove(size_t off, size_t n, size_t& out)
{
	if( !this->_buf.removeindex(off, n) )
	{// failed
		out = 0;
		return false;
	}
	out = n;
	return true;
}

/// Resizes the buffer to sz.
/// If successfull out contains sz.
/// Otherwise out contains the maximum size if increasing 
/// or the minimum size if decreasing.
bool CDynBuffer::resize(size_t sz, size_t& out)
{
	size_t old_len = this->length();
	if( !this->_buf.resize(sz) )
	{// failed
		out = this->capacity();
		return false;
	}
	if( old_len < this->length() )// increasing
		memset(this->data(old_len), 0, this->length() - old_len);
	out = sz;
	return true;
}





}// end namespace NFieldHandler
///////////////////////////////////////////////////////////////////////////////
// Packets
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
template<size_t SZ>
class CPublicFixPacket : public CFixPacket<1,SZ>
{
public:
	CPublicFixPacket() : CFixPacket<1,SZ>() {}
	CPublicFixPacket(const uint8* buf, size_t sz) : CFixPacket<1,SZ>(buf, sz) {}
};

template<size_t SZ>
class CPublicLimPacket : public CLimPacket<1,SZ>
{
public:
	CPublicLimPacket() : CLimPacket<1,SZ>() {}
	CPublicLimPacket(const uint8* buf, size_t sz) : CLimPacket<1,SZ>(buf, sz) {}
};

class CPublicDynPacket : public CDynPacket<1>
{
public:
	CPublicDynPacket() : CDynPacket<1>() {}
	CPublicDynPacket(const uint8* buf, size_t sz) : CDynPacket<1>(buf, sz) {}
};

class CTestFixPacket : public CFixPacket<5,20>
{
public:
	CFieldB                   b;
	CFieldW                   w;
	CFieldL                   l;
	CFieldFixString<10>       str;
	CFieldFixArray<CFieldB,3> arr;
public:
	CTestFixPacket()
	:	CFixPacket<5,20>()
	{
		Init();
	}
	CTestFixPacket(const uint8* buf, size_t sz)
	:	CFixPacket<5,20>(buf, sz)
	{
		Init();
	}

private:
	void Init()
	{
		NFieldHandler::CIdxAutoRegistry reg(this->_h);
		reg << b
			<< w
			<< l
			<< str
			<< arr;
	}
};

class CTestLimPacket : public CLimPacket<3,10>
{
public:
	CFieldW       pre;
	CFieldCString str;
	CFieldW       post;
public:
	CTestLimPacket()
	:	CLimPacket<3,10>()
	{
		NFieldHandler::CIdxAutoRegistry reg(this->_h);
		reg << pre
			<< str
			<< post;
	}

};

class CTestDynPacket : public CDynPacket<3>
{
public:
	CFieldW       pre;
	CFieldCString str;
	CFieldW       post;
public:
	CTestDynPacket()
	:	CDynPacket<3>()
	{
		NFieldHandler::CIdxAutoRegistry reg(this->_h);
		reg << pre
			<< str
			<< post;
	}

};

class CTestDynPacket2 : public CDynPacket<4>
{
public:
	CFieldCString                    str1;
	CFieldCString                    str2;
	CFieldFixArray<CFieldCString, 2> arr;
	CFieldCString                    str3;
public:
	CTestDynPacket2()
	:	CDynPacket<4>()
	{
		NFieldHandler::CIdxAutoRegistry reg(this->_h);
		reg << str1
			<< str2
			<< arr
			<< str3;
	}
};

class CTestDynPacket3 : public CDynPacket<3>
{
public:
	CFieldB                       b1;
	CFieldDynArray<CFieldCString> arr;
	CFieldB                       b2;
public:
	CTestDynPacket3()
	:	CDynPacket<3>()
	{
		NFieldHandler::CIdxAutoRegistry reg(this->_h);
		reg << b1
			<< arr
			<< b2;
	}

};

class CTestMorphPacket : public CDynPacket<5>
{
public:
	CFieldB val0;
	CFieldB val1;
private:
	CFieldDynBlob pad0;
	CFieldDynBlob pad1;
	CFieldDynBlob pad2;
public:
	CTestMorphPacket()
	:	CDynPacket<5>()
	{
		NFieldHandler::CIdxAutoRegistry reg(this->_h);
		reg << pad0// 0
			<< val0// 1
			<< pad1// 2
			<< val1// 3
			<< pad2;// 4
	}
	CTestMorphPacket(uint8* buf, size_t len, size_t off0, size_t off1)
	:	CDynPacket<5>(buf, len)
	{
		this->Register(off0, off1);
	}

public:
	// off0 < off1 < len
	bool Morph(size_t off0, size_t off1, size_t len)
	{
		if( off0 >= off1 || off1 >= len )
		{
			printf("[Debug] invalid argument at CTestMorphPacket::Morph(%u,%u,%u)\n", (uint)off0, (uint)off1, (uint)len);
			return false;
		}
		this->pad0.resize(off0);
		this->pad1.resize(off1 - this->_h.offset(2));
		this->pad2.resize(len - this->_h.offset(4));
		return true;
	}
	// off0 < off1
	bool Register(size_t off0, size_t off1)
	{
		if( off0 >= off1 || off1 >= this->_h.length() )
		{
			printf("[Debug] invalid argument at CTestMorphPacket::Register(%u,%u) (len=%u)\n", (uint)off0, (uint)off1, (uint)this->_h.length());
			return false;
		}
		NFieldHandler::CIdxAutoRegistry reg(this->_h);
		reg.append(pad0, off0);// 0
		reg.append(val0);// 1
		reg.append(pad1, off1 - this->_h.offset(1) - this->_h.length(1));// 2
		reg.append(val1);// 3
		reg.append(pad2, this->_h.length() - this->_h.offset(3) - this->_h.length(3));// 4
		return true;
	}
};

void dump(IPacket& p)
{
	const uint8* data = p.data();
	size_t len = p.length();
	size_t i;
	for( i = 0; i < len; i++ )
		printf("%02X ", data[i]);
	printf("size=%u\n", (uint)len);
}
#endif//DEBUG

void test_packet(void)
{
#if defined(DEBUG)
	{
		printf("test_packet: fixed-size packet constructors\n");
		{// default constructor
			CPublicFixPacket<20> p;
			dump(p);
		}
		{// null data constructor
			CPublicFixPacket<20> p(NULL, 5);
			dump(p);
		}
		{// size under
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9};
			CPublicFixPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
		{// size minus one
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
			CPublicFixPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
		{// exact size
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
			CPublicFixPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
		{// size over
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29};
			CPublicFixPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
	}
	{
		printf("test_packet: limited-size packet constructors\n");
		{// default constructor
			CPublicLimPacket<20> p;
			dump(p);
		}
		{// null data constructor
			CPublicLimPacket<20> p(NULL, 5);
			dump(p);
		}
		{// size under
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9};
			CPublicLimPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
		{// size minus one
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
			CPublicLimPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
		{// exact size
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
			CPublicLimPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
		{// size over
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29};
			CPublicLimPacket<20> p(buf, sizeof(buf));
			dump(p);
		}
	}
	{
		printf("test_packet: dynamic-size packet constructors\n");
		{// default constructor
			CPublicDynPacket p;
			dump(p);
		}
		{// null data constructor
			CPublicDynPacket p(NULL, 5);
			dump(p);
		}
		{// data constructor
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9};
			CPublicDynPacket p(buf, sizeof(buf));
			dump(p);
		}
	}
	{
		printf("test_packet: CTestFixPacket\n");
		CTestFixPacket p;
		dump(p); p.b = 100;
		dump(p); p.w = 10000;
		dump(p); p.l = 100000000 + p.b + p.w;
		dump(p); p.str = "under";
		dump(p); p.str = "minus one";
		dump(p); p.str = "exact-----";
		dump(p); p.str = "1234567890 OVER!!!";
		dump(p); p.str = "under";
		dump(p); p.arr[0] = 1;
		dump(p); p.arr[1] = 2;
		dump(p); p.arr[2] = 3;
		dump(p);
	}
	{
		printf("test_packet: CTestLimPacket\n");
		CTestLimPacket p;
		dump(p); p.pre = 1;
		dump(p); p.post = 2;
		dump(p); p.str = "test";
		dump(p); p.pre = 3;
		dump(p); p.post = 4;
		dump(p); p.str = "OVER test!!!";
		dump(p); p.str = "";
		dump(p);
	}
	{
		printf("test_packet: CTestDynPacket\n");
		CTestDynPacket p;
		dump(p); p.pre = 1;
		dump(p); p.post = 2;
		dump(p); p.str = "test";
		dump(p); p.pre = 3;
		dump(p); p.post = 4;
		dump(p); p.str = "can't do OVER so just try a big string";
		dump(p); p.str = "";
		dump(p);
	}
	{
		printf("test_packet: CTestDynPacket2\n");
		CTestDynPacket2 p;
		dump(p); p.str1 = "str1";
		dump(p); p.str2 = "str2";
		dump(p); p.arr[0] = "arr[0]";
		dump(p); p.arr[1] = "arr[1]";
		dump(p); p.str3 = "str3";
		dump(p); p.str2 = "2";
		dump(p); p.arr[0] = "3";
		dump(p); p.str1 = "1";
		dump(p); p.arr[1] = "4";
		dump(p); p.str3 = "";
		dump(p);
	}
	{
		printf("test_packet: CTestDynPacket3\n");
		CTestDynPacket3 p;
		dump(p); p.b1 = 0x11;
		dump(p); p.b2 = 0x22;
		dump(p); p.arr.resize(1);
		dump(p); p.arr[0] = "1";
		dump(p); p.arr.resize(2);
		dump(p); p.arr[0] = "2";
		dump(p); p.arr[1] = "2";
		dump(p); p.arr.resize(5);
		dump(p); p.arr[0] = "0";
		dump(p); p.arr[1] = "1";
		dump(p); p.arr[2] = "2";
		dump(p); p.arr[3] = "3";
		dump(p); p.arr[4] = "4";
		dump(p); p.arr[0] = "str0";
		dump(p); p.arr[1] = "a";
		dump(p); p.arr[2] = "b";
		dump(p); p.arr[3] = "c";
		dump(p); p.arr[4] = "d";
		dump(p); p.b1 = 0x33;
		dump(p); p.b2 = 0x44;
		dump(p); p.arr.resize(4);
		dump(p); p.arr.resize(0);
		dump(p); p.b1 = 0x55;
		dump(p); p.b2 = 0x66;
		dump(p); p.arr.resize(3);
		dump(p); p.arr[0] = "str1";
		dump(p); p.arr[1] = "str2";
		dump(p); p.arr[2] = "str3";
		dump(p); p.b1 = 0x77;
		dump(p); p.b2 = 0x88;
		dump(p); p.arr[1] = "+++++++++++++";
		dump(p); p.arr[1] = "-";
		dump(p); p.b1 = 0x99;
		dump(p); p.b2 = 0xAA;
		dump(p); p.arr.resize(0);
		dump(p); p.b1 = 0xBB;
		dump(p); p.b2 = 0xCC;
		dump(p);
	}
	{
		printf("test_packet: morph CTestMorphPacket\n");
		CTestMorphPacket p;
		dump(p); p.val0 = 1;
		dump(p); p.val1 = 2;
		dump(p); p.Morph(5, 6, 10);
		dump(p); p.val0 = 3;
		dump(p); p.val1 = 4;
		dump(p); p.Morph(3, 7, 8);
		dump(p); p.val0 = 5;
		dump(p); p.val1 = 6;
		dump(p); p.Morph(2, 4, 7);
		dump(p); p.val0 = 7;
		dump(p); p.val1 = 8;
		dump(p); p.Morph(0, 1, 2);
		dump(p); p.val0 = 9;
		dump(p); p.val1 = 10;
		dump(p);
	}
	{
		printf("test_packet: re-register CTestMorphPacket\n");
		uint8 buf[] = {0,1,2,3,4,5,6,7,8,9};
		CTestMorphPacket p(buf, sizeof(buf), 3, 6);
		dump(p);
		printf("val0=%u, val1=%u\n", (uint)p.val0, (uint)p.val1);
		p.Register(2, 5);
		dump(p);
		printf("val0=%u, val1=%u\n", (uint)p.val0, (uint)p.val1);
		p.Register(0, 9);
		dump(p);
		printf("val0=%u, val1=%u\n", (uint)p.val0, (uint)p.val1);
		p.Register(5, 3);// invalid
		dump(p);
		printf("val0=%u, val1=%u\n", (uint)p.val0, (uint)p.val1);
		p.Register(0, 10);// invalid
		dump(p);
		printf("val0=%u, val1=%u\n", (uint)p.val0, (uint)p.val1);
	}
#endif//DEBUG
}


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////
