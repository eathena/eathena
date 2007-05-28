
#include "basepacket.h"


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
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

class CTestFixPacket : public CFixPacket<5,20>
{
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

public:
	CFieldB b;
	CFieldW w;
	CFieldL l;
	CFieldFixString<10> str;
	CFieldFixArray<CFieldB,3> arr;

private:
	void Init()
	{
		// buf, index, offset
		b.Init(&this->_h, 0, 0);
		w.Init(&this->_h, 1, 1);
		l.Init(&this->_h, 2, 3);
		str.Init(&this->_h, 3, 7);
		arr.Init(&this->_h, 4, 17, 3);
	}
};

class CTestLimPacket : public CLimPacket<3,10>
{
public:
	CTestLimPacket()
	:	CLimPacket<3,10>()
	{
		size_t off = 0;
		// buf, index, offset
		pre.Init(&this->_h, 0, off); off += this->_h.length(0);
		str.Init(&this->_h, 1, off); off += this->_h.length(1);
		post.Init(&this->_h, 2, off);
	}

public:
	CFieldW pre;
	CFieldCString str;
	CFieldW post;
};

class CTestDynPacket : public CDynPacket<3>
{
public:
	CTestDynPacket()
	:	CDynPacket<3>()
	{
		size_t off = 0;
		// buf, index, offset
		pre.Init(&this->_h, 0, off); off += this->_h.length(0);
		str.Init(&this->_h, 1, off); off += this->_h.length(1);
		post.Init(&this->_h, 2, off);
	}

public:
	CFieldW pre;
	CFieldCString str;
	CFieldW post;
};

class CTestDynPacket2 : public CDynPacket<4>
{
public:
	CTestDynPacket2()
	:	CDynPacket<4>()
	{
		size_t off = 0;
		// buf, index, offset
		str1.Init(&this->_h, 0, off); off += this->_h.length(0);
		str2.Init(&this->_h, 1, off); off += this->_h.length(1);
		arr.Init(&this->_h, 2, off); off += this->_h.length(2);
		str3.Init(&this->_h, 3, off);
	}

public:
	CFieldCString str1;
	CFieldCString str2;
	CFieldFixArray<CFieldCString, 2> arr;
	CFieldCString str3;
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
		printf("test_packet: CTestFixPacket\n");
		CTestFixPacket p;
		dump(p);
		p.b = 100;
		dump(p);
		p.w = 10000;
		dump(p);
		p.l = 100000000 + p.b + p.w;
		dump(p);
		p.str = "under";
		dump(p);
		p.str = "minus one";
		dump(p);
		p.str = "exact-----";
		dump(p);
		p.str = "1234567890 OVER!!!";
		dump(p);
		p.str = "under";
		dump(p);
		p.arr[0] = 1;
		dump(p);
		p.arr[1] = 2;
		dump(p);
		p.arr[2] = 3;
		dump(p);
	}
	{
		printf("test_packet: CTestLimPacket\n");
		CTestLimPacket p;
		dump(p);
		p.pre = 1;
		dump(p);
		p.post = 2;
		dump(p);
		p.str = "test";
		dump(p);
		p.pre = 3;
		dump(p);
		p.post = 4;
		dump(p);
		p.str = "OVER test!!!";
		dump(p);
		p.str = "";
		dump(p);
	}
	{
		printf("test_packet: CTestDynPacket\n");
		CTestDynPacket p;
		dump(p);
		p.pre = 1;
		dump(p);
		p.post = 2;
		dump(p);
		p.str = "test";
		dump(p);
		p.pre = 3;
		dump(p);
		p.post = 4;
		dump(p);
		p.str = "can't do OVER so just try a big string";
		dump(p);
		p.str = "";
		dump(p);
	}
	{
		printf("test_packet: CTestDynPacket2\n");
		CTestDynPacket2 p;
		dump(p);
		p.str1 = "str1";
		dump(p);
		p.str2 = "str2";
		dump(p);
		p.arr[0] = "arr[0]";
		dump(p);
		p.arr[1] = "arr[1]";
		dump(p);
		p.str3 = "str3";
		dump(p);
		p.str2 = "2";
		dump(p);
		p.arr[0] = "3";
		dump(p);
		p.str1 = "1";
		dump(p);
		p.arr[1] = "4";
		dump(p);
		p.str3 = "";
		dump(p);
	}
#endif//DEBUG
}


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////
