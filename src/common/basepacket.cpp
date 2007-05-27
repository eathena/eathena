
#include "basepacket.h"


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
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

	CFieldB b;
	CFieldW w;
	CFieldL l;
	CFieldFixString<10> str;
	CFieldFixArray<CFieldB,3> arr;

private:
	void Init()
	{
		b.Init(&_buf, 0, 0);
		w.Init(&_buf, 1, 1);
		l.Init(&_buf, 2, 3);
		str.Init(&_buf, 3, 7);
		arr.Init(&_buf, 4, 17);
	}
};

void dump(IPacket& p)
{
	size_t i;
	for( i = 0; i < p.length(); i++ )
		printf("%02X ", p.data()[i]);
	printf("size=%u\n", (uint)p.length());
}
#endif//DEBUG

void test_packet(void)
{
#if defined(DEBUG)
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
		printf("test_packet: fixed size packet constructors\n");
		{// default constructor
			CFixPacket<1,20> p;
			dump(p);
		}
		{// size under
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9};
			CFixPacket<1,20> p(buf, sizeof(buf));
			dump(p);
		}
		{// size minus one
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
			CFixPacket<1,20> p(buf, sizeof(buf));
			dump(p);
		}
		{// exact size
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
			CFixPacket<1,20> p(buf, sizeof(buf));
			dump(p);
		}
		{// size over
			uint8 buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29};
			CFixPacket<1,20> p(buf, sizeof(buf));
			dump(p);
		}
	}
#endif//DEBUG
}


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////
