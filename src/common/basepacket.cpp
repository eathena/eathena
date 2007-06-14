
#include "basepacket.h"


///////////////////////////////////////////////////////////////////////////////
namespace NSocket {
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
namespace NFieldHandler {
///////////////////////////////////////////////////////////////////////////////





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

/// Forward id iteration.
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



///////////////////////////////////////////////////////////////////////////////
// Buffer backed by a field handler.

CChildBuffer::CChildBuffer()
:	_id(~size_t(0))
,	_h(NULL)
{}

CChildBuffer::~CChildBuffer()
{}

/// Initializes this child buffer.
void CChildBuffer::Init(IFieldHandler* h, size_t id, size_t off, size_t len)
{
	this->_id = id;
	this->_h = h;
	if( h )
		h->setup(id, off, len);
}

/// Returns the data of this buffer.
const uint8* CChildBuffer::data(size_t off) const
{
	if( this->_h )
		return this->_h->data(this->_id) + off;
	return NULL;
}
uint8* CChildBuffer::data(size_t off)
{
	if( this->_h )
		return this->_h->data(this->_id) + off;
	return NULL;
}

/// Returns the length of this buffer.
size_t CChildBuffer::length() const
{
	if( this->_h )
		return this->_h->length(this->_id);
	return 0;
}

/// Returns the capacity of this buffer.
size_t CChildBuffer::capacity() const
{
	if( this->_h )
		return this->_h->capacity(this->_id);
	return 0;
}

/// Inserts n bytes in off.
/// If successfull out contains n.
/// Otherwise out contains the maximum n that can inserted in off.
bool CChildBuffer::insert(size_t off, size_t n, size_t& out)
{
	if( this->_h )
		return this->_h->insert(this->_id, off, n, out);
	out = 0;
	return false;
}

/// Removes n bytes from off. [off,off+n[
/// If successfull out contains n.
/// Otherwise out contains the maximum n that can be removed from off.
bool CChildBuffer::remove(size_t off, size_t n, size_t& out)
{
	if( this->_h )
		return this->_h->remove(this->_id, off, n, out);
	out = 0;
	return false;
}

/// Resizes the buffer to sz.
/// If successfull out contains sz.
/// Otherwise out contains the maximum size if increasing 
/// or the minimum size if decreasing.
bool CChildBuffer::resize(size_t sz, size_t& out)
{
	if( this->_h )
		return this->_h->resize(this->_id, sz, out);
	return false;
}

/// Returns the id of this child buffer in the parent.
size_t CChildBuffer::id() const
{
	return this->_id;
}

/// Returns the parent of this child buffer.
const IFieldHandler* CChildBuffer::parent() const
{
	return this->_h;
}
IFieldHandler* CChildBuffer::parent()
{
	return this->_h;
}



///////////////////////////////////////////////////////////////////////////////
}// end namespace NFieldHandler
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
/// Dynamic-size nul-terminated string.

CFieldCString::CFieldCString()
:	AField()
{}

/// Initializes this field.
///
/// @param len Maximum length of the field
void CFieldCString::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t len)
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

/// Converts this c-string to data.
CFieldCString::operator const char*() const
{
	return this->data();
}

/// Returns the data of this c-string.
const char* CFieldCString::operator ()() const
{
	return this->data();
}
const char* CFieldCString::data() const
{
	if( this->_h )
		return (const char*)this->_h->data(this->_id);
	return "";
}

/// Assigns data to this c-string.
CFieldCString& CFieldCString::operator =(const CFieldCString& f)
{
	return this->assign(f.data(), f.length());
}
CFieldCString& CFieldCString::operator =(const char* str)
{
	return this->assign(str, ~size_t(0));
}
CFieldCString& CFieldCString::assign(const char* str, size_t sz)
{
	if( this->_h )
	{
		if( str == NULL )
			str = "";
		size_t len = strnlen(str, sz) + 1;
		if( !this->_h->resize(this->_id, len, len) )
		{
#if defined(DEBUG)
			if( len == 0 )//## error, this should never happen
				printf("[Debug] resize failed (len=0) at CFieldCString::assign(const char* str, size_t sz)\n");
#endif//DEBUG
			this->_h->resize(this->_id, len, len);// resize to max instead
		}
		char* buf = (char*)this->_h->data(this->_id);
		memcpy(buf, str, len - 1);
		buf[len - 1] = '\0';
	}
	return *this;
}

/// Returns the length of this c-string.
size_t CFieldCString::size() const
{
	return this->length();
}
size_t CFieldCString::length() const
{
	if( this->_h )
	{
		size_t len = this->_h->length(this->_id);
		if( len > 0 )
			return len - 1;
	}
	return 0;
}

/// Returns the capacity of this c-string.
size_t CFieldCString::capacity() const
{
	if( this->_h )
	{
		size_t len = this->_h->capacity(this->_id);
		if( len > 0 )
			return len - 1;
	}
	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// Dynamic-size blob of data.

CFieldDynBlob::CFieldDynBlob()
:	AField()
{}

/// Initializes the field.
///
/// @param len Maximum length the field can have
void CFieldDynBlob::Init(NFieldHandler::IFieldHandler* h, size_t id, size_t off, size_t maxlen)
{
	this->_id = id;
	this->_h = h;
	if( h )
		h->setup(id, off, maxlen);// take over all the data
}

/// Array access.
const uint8& CFieldDynBlob::operator [](size_t idx) const
{
	return *(this->data() + idx);
}
uint8& CFieldDynBlob::operator [](size_t idx)
{
	return *(this->data() + idx);
}

/// List access.
const uint8& CFieldDynBlob::operator ()(size_t idx) const
{
	return *(this->data() + idx);
}
uint8& CFieldDynBlob::operator ()(size_t idx)
{
	return *(this->data() + idx);
}

/// Converts this blob to data.
CFieldDynBlob::operator const uint8*() const
{
	return this->data();
}
CFieldDynBlob::operator uint8*()
{
	return this->data();
}

/// Returns the data of this blob.
const uint8* CFieldDynBlob::data() const
{
	if( this->_h )
		return this->_h->data(this->_id);
	return NULL;
}
uint8* CFieldDynBlob::data()
{
	if( this->_h )
		return this->_h->data(this->_id);
	return NULL;
}
const uint8* CFieldDynBlob::operator ()() const
{
	return this->data();
}
uint8* CFieldDynBlob::operator ()()
{
	return this->data();
}

/// Assigns data to this blob.
CFieldDynBlob& CFieldDynBlob::operator =(const CFieldDynBlob& f)
{
	return this->assign(f, f.length());
}

/// Copies sz bytes from data to this field.
CFieldDynBlob& CFieldDynBlob::assign(const uint8* data, size_t sz)
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
	return *this;
}

/// Returns the length of this blob.
size_t CFieldDynBlob::size() const
{
	return this->length();
}
size_t CFieldDynBlob::length() const
{
	return this->_h->length(this->_id);
}

/// Resizes this blob.
bool CFieldDynBlob::resize(size_t len)
{
	if( this->_h )
		return this->_h->resize(this->_id, len, len);
	return false;
}





///////////////////////////////////////////////////////////////////////////////
// Test/example packets
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
}// end namespace NSocket
///////////////////////////////////////////////////////////////////////////////
