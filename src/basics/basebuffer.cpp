#include "basebuffer.h"
#include "basesync.h"
#include "basearray.h"
#include "basestrformat.h"

NAMESPACE_BEGIN(basics)


#if defined(DEBUG)



class streamtest
{
	buffer ba;

public:
	streamtest()	{}

	template <class X> streamtest& operator<<(const formatstr<char,X>& a)
	{
		ba = (const X&)a;
		return *this;
	}
	template <class X> streamtest& operator>>(const formatstr<char,X>& a)
	{
		a = ba;
		return *this;
	}

	streamtest& operator<<(unsigned long a)
	{
		ba = a;
		return *this;
	}
	streamtest& operator>>(unsigned long &a)
	{
		a = ba;
		return *this;
	}

	streamtest& operator<<(unsigned short a)
	{
		ba = a;
		return *this;
	}
	streamtest& operator>>(unsigned short &a)
	{
		a = ba;
		return *this;
	}

	streamtest& operator<<(unsigned char a)
	{
		ba = a;
		return *this;
	}
	streamtest& operator>>(unsigned char &a)
	{
		a = ba;
		return *this;
	}

};

#endif//DEBUG


void test_buffer(void)
{

#if defined(DEBUG)

	{
		ulong tick, s, i,k;

		size_t sz=256;
		double arr[256*256];
		static double v;

		double**arr2 = new double*[sz];
		for(i=0;i<sz; ++i)
			arr2[i] = new double[sz];


		tick = clock();
		for(s=0; s<5000; ++s)
		for(i=0; i<128; ++i)
		for(k=0; k<128; ++k)
		{
			v = arr[i*sz+k];
		}
		printf("array ordered access mult %lu\n", clock()-tick);
	
		tick = clock();
		for(s=0; s<5000; ++s)
		for(i=0; i<128; ++i)
		for(k=0; k<128; ++k)
		{
			v = arr2[i][k];
		}
		printf("array ordered access deref %lu\n", clock()-tick);
	


		uchar *randix_x = new uchar[4194304];
		uchar *randix_y = new uchar[4194304];

		for(s=0;s<4194304;s++)
		{
			randix_x[s] = rand()%sz;
			randix_y[s] = rand()%sz;
		}

		tick = clock();
		for(i=0; i<4; ++i)
		for(s=0; s<4194304; ++s)
		{
			v = arr[randix_x[s]*sz+randix_y[s]];
		}
		printf("array rand access mult %lu\n", clock()-tick);
	
		tick = clock();
		for(i=0; i<4; ++i)
		for(s=0; s<4194304; ++s)
		{
			v = arr2[randix_x[s]][randix_y[s]];
		}
		printf("array rand access deref %lu\n", clock()-tick);

		delete[] randix_x;
		delete[] randix_y;


	}

	try
	{
		unsigned char ucbuf[64];
		_fixbuffer bbb(ucbuf, sizeof(ucbuf));
		_buffer ccc(20);




		bbb << 1ul;
		bbb << 2ul;

//		ccc << 1ul;
//		ccc << 2ul;
//		ccc << 3ul;


		_bufferaccess baa(ccc, 10);

		baa[5] = (uchar)12;
		baa[1] = 12ul;

		baa[6] = "hallo";

		bbb.length();
		ccc.length();

		streamtest tt;


		tt << format<unsigned long>("%i",1) << format<unsigned long>("%i",2) << 3ul;
		unsigned long a,b,c;

		tt >> format("%i",a) >> b >> format("%i",c);



		size_t i;

		buffer ba;
		unsigned char buf[16];
		buffer_iterator bb(buf, sizeof(buf));
		
		ba = (unsigned long)1;

		for(i=0; i<5; ++i)
			bb = (unsigned short)i;

		printf("buffer\n");
		for(i=0; i<sizeof(buf); ++i)
			printf("%02x ", buf[i]);
		printf("\n");

		for(i=5; i<10; ++i)
			printf("%02x ", (unsigned short)bb);
		printf("\n");

		printf("buffer\n");
		for(i=0; i<sizeof(buf); ++i)
			printf("%02x ", buf[i]);
		printf("\n");


		for(i=5; i<10; ++i)
			bb = (unsigned short)i;

		printf("buffer\n");
		for(i=0; i<sizeof(buf); ++i)
			printf("%02x ", buf[i]);
		printf("\n");

		for(i=0; i<10; ++i)
			printf("%i", (unsigned short)bb);
		printf("\n");
	
	}
	catch( exception e )
	{
		printf( e.what() );
	}
#endif//DEBUG
}

NAMESPACE_END(basics)
