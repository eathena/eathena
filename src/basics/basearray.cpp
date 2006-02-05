
#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basealgo.h"
#include "basetime.h"
#include "basememory.h"
#include "basestring.h"
#include "baseexceptions.h"
#include "basearray.h"



///////////////////////////////////////////////////////////////////////////
template<typename T, typename E, typename A> 
void vectorbase<T,E,A>::debug_print()
{
	string<> str;
	typename A::iterator iter(*this);
	while(iter <= this->end())
		str << *iter++ << ' ';
	printf( (const char*)str );
	printf("\n");
}
















int test_array(void)
{
	//!! TODO copy testcases from caldon


	{

		size_t i,k;
		ulong tick;
		TArrayDST<char> arr;
		vector<char> vec;

		tick = clock();

		for(k=0; k<1000;k++, arr.resize(1), arr.realloc(1000))
		for(i=0; i<1000; i++)
		{
			arr.append( (char)i );
		}
		printf("%lu\n", clock()-tick);


		tick = clock();
		for(k=0; k<1000;k++, vec.resize(1), vec.realloc(1000))
		for(i=0; i<1000; i++)
		{			
			vec.append( (char)i );
		}
		printf("%lu\n", clock()-tick);

	}







	{
		vector<char> ca, cb("hallo", 5);
//		vector< char, elaborator_st<char>, allocator_rw_st< char, elaborator_st<char> > > cc;
		vector< char > cc;
	
		if(cb.size() != 5) printf("vector construction failed (size=%lu should be 5)\n",(ulong)cb.size());

		ca = cb;
		if(ca.size() != 5) printf("vector assignment operator failed (size=%lu should be 5)\n",(ulong)ca.size());
		ca.clear();
		if(ca.size() != 0) printf("vector clear failed (size=%lu should be 0)\n",(ulong)ca.size());

		ca.assign(cb);
		if(ca.size() != 5) printf("vector assign vector failed (size=%lu should be 5)\n",(ulong)ca.size());

		ca.assign("abc", 3);
		if(ca.size() != 3) printf("vector assign carray failed (size=%lu should be 3)\n",(ulong)ca.size());

		cb.assign("", 0);
		if(cb.size() != 0) printf("vector assign 0 failed (size=%lu should be 0)\n",(ulong)ca.size());

		ca.assign('e', 5);
		if(ca.size() != 5) printf("vector assign element failed (size=%lu should be 5)\n",(ulong)ca.size());

		ca.assign("hallo", 5);
		if(ca.size() != 5) printf("vector assign element carray (size=%lu should be 5)\n",(ulong)ca.size());

		vector<int> ia(cc), ib(ca);

		if(ia.size() != 0) printf("array construction failed (size=%lu should be 0)\n",(ulong)ia.size());
		if(ib.size() != 5) printf("array construction failed (size=%lu should be 5)\n",(ulong)ib.size());

		cb = cc;
		if(cb.size() != 0) printf("vector assignment operator failed (size=%lu should be 0)\n",(ulong)cb.size());


		cb.append("hallo", 5);
		if(cb.size() != 5) printf("vector append carray failed (size=%lu should be 5)\n",(ulong)cb.size());

		ia = cb;
		if(ia.size() != 5) printf("vector append carray failed (size=%lu should be 5)\n",(ulong)ia.size());


		cb.insert("xxx", 3, 2);
		if(cb.size() != 8) printf("vector append failed (size=%lu should be 8)\n",(ulong)cb.size());
		if( memcmp(&(cb[0]),"haxxxllo", 8*sizeof(char)) ) printf("array append failed, wrong content\n");

		if( cb[0]!='h' ||
			cb[1]!='a' ||
			cb[2]!='x' ||
			cb[3]!='x' ||
			cb[4]!='x' ||
			cb[5]!='l' ||
			cb[6]!='l' ||
			cb[7]!='o' )
			printf("vector append failed, wrong content/ []operator failed");

		if( cb.first() != 'h' )
			printf("vector append failed, first() failed, returning '%c' should be 'h'\n", cb.first() );
		if( cb.last() != 'o' )
			printf("vector append failed, last() failed, returning '%c' should be 'o'\n", cb.last() );


		cb.resize(5);

		if( cb.size() != 5 )
			printf("vector resize failed, size %lu should be 5\n", (ulong)cb.size() );

		if( cb.last() != 'x' )
			printf("vector resize failed, last() returning '%c' should be 'x'\n", cb.last() );

		cb.resize(10);
		if( cb.size() != 10 )
			printf("vector resize failed, size %lu should be 10\n", (ulong)cb.size() );


		cb.append('i',2);
		if( cb.size() != 12 )
			printf("vector append element failed, size %lu should be 12\n", (ulong)cb.size() );

		cb.resize(0);
		cb.append(ca);
		cb.append(ca);
		if( cb.size() != 10 )
			printf("vector append vector failed, size %lu should be 10\n", (ulong)cb.size() );

		cb.append("dd",2);
		if( cb.size() != 12 )
			printf("vector append vector failed, size %lu should be 12\n", (ulong)cb.size() );

		cb.removeindex(1);
		if( cb.size() != 11 )
			printf("vector removeindex failed, size %lu should be 11\n", (ulong)cb.size() );
		if( cb[1] != 'l' )
			printf("vector removeindex failed, element[1] is %i should be %i\n", cb[1], 'l' );

		cb.removeindex(3,5);
		if( cb.size() != 6 )
			printf("vector removeindex failed, size %lu should be 6\n", (ulong)cb.size() );

		cb.clear();
		if( cb.size() != 0 )
			printf("vector clear failed, size %lu should be 0\n", (ulong)cb.size() );

		cb.append("aaaabbbbccccdddd", 16);
		cb.move(4,8,4);
		if( cb[ 0]!='a' ||
			cb[ 1]!='a' ||
			cb[ 2]!='a' ||
			cb[ 3]!='a' ||
			cb[ 4]!='c' ||
			cb[ 5]!='c' ||
			cb[ 6]!='c' ||
			cb[ 7]!='c' ||
			cb[ 8]!='b' ||
			cb[ 9]!='b' ||
			cb[10]!='b' ||
			cb[11]!='b' ||
			cb[12]!='d' ||
			cb[13]!='d' ||
			cb[14]!='d' ||
			cb[15]!='d' )
			printf("vector move failed, wrong content\n");
		

		ca.assign("hallo",5);

		cb.assign("aabb",4);

		cb.insert(ca,2);
		if( cb.size() != 4+ca.size() )
			printf("vector insert vector failed, size %lu should be %lu\n", (ulong)cb.size(), (ulong)(4+ca.size()) );

		cb.insert("xxxx", 4,2);
		if( cb.size() != 8+ca.size() )
			printf("vector insert carray failed, size %lu should be %lu\n", (ulong)cb.size(), (ulong)(8+ca.size()) );

		cb.insert('z',2,2);
		if( cb.size() != 10+ca.size() )
			printf("vector insert element failed, size %lu should be %lu\n", (ulong)cb.size(), (ulong)(10+ca.size()) );

		cb.assign("aabb",4);

		cb.copy(ca,2);
		if( cb.size() != 2+ca.size() )
			printf("vector copy vector failed, size %lu should be %lu\n", (ulong)cb.size(), (ulong)(2+ca.size()) );

		cb.clear();
		cb.append("aabbxxxxxx",10);

		cb.copy(ca,2);
		if( cb.size() != 10 )
			printf("vector copy vector failed, size %lu should be %lu\n", (ulong)cb.size(), 10lu );

		cb.assign("aabb",4);

		cb.copy("xxxx",4, 2);
		if( cb.size() != 2+4 )
			printf("vector copy carray failed, size %lu should be %lu\n", (ulong)cb.size(), (ulong)(2+4) );

		cb.assign("aabbxxxxxx",10);

		cb.copy("xxxx",4, 2);
		if( cb.size() != 10 )
			printf("vector copy carray failed, size %lu should be %lu\n", (ulong)cb.size(), 10lu );

		cb.assign("aaxxxxbb",8);

		cb.replace(ca, 2,4);
		if( cb.size() != 9 )
			printf("vector replace vector failed, size %lu should be %lu\n", (ulong)cb.size(), 9ul );

		cb.assign("aaxxxxxxxbb",11);

		cb.replace(ca, 2,7);
		if( cb.size() != 9 )
			printf("vector replace vector failed, size %lu should be %lu\n", (ulong)cb.size(), 9lu );

		cb.assign("aaxxxxbb",8);

		cb.replace("_____", 5, 2,4);
		if( cb.size() != 9 )
			printf("vector replace carray grow failed, size %lu should be %lu\n", (ulong)cb.size(), 9ul );

		cb.assign("aaxxxxxxxbb",11);

		cb.replace("_____", 5, 2,7);
		if( cb.size() != 9 )
			printf("vector replace carray shrink failed, size %lu should be %lu\n", (ulong)cb.size(), 9lu );


		printf("stack order\n");
		cb.assign('a');
		cb.push('b');
		cb.debug_print();
		cb.push('c');
		cb.debug_print();
		cb.push('d');
		cb.debug_print();
		cb.pop();
		cb.debug_print();
		cb.pop();
		cb.debug_print();

		printf("fifo order\n");
		fifo<char> ff('a');
		ff.debug_print();
		ff.push('b');
		ff.debug_print();
		ff.push('c');
		ff.debug_print();
		ff.push('d');
		ff.debug_print();
		ff.pop();
		ff.debug_print();
		ff.pop();
		ff.debug_print();


		printf("slist\n");
		slist<char> ss;
		ss.push('a'); ss.debug_print();
		ss.push(ff); ss.debug_print();
		ss.insert("kksj", 4, 2); ss.debug_print();

		ss.append('g'); ss.debug_print();

		int a = ss[3];

		ss[3] = 'z'; 
		ss.debug_print();



/////////////////////////////////////////
		fifo<char> f1;
		f1.clear();
		if( f1.size()!=0 ) printf("err 1\n");
		f1.realloc(10);
		if( f1.size()!=0 ) printf("err 2\n");
		f1.resize(12);
		if( f1.size()!=12 ) printf("err 3\n");
		f1.clear();
		f1.assign("abcdefgh", 8);
		if( f1.size()!=8 || 0!=memcmp(f1.begin(), "abcdefgh", 0) ) printf("err 4\n");
		f1.move(2, 5, 2);
		if( 0!=memcmp(f1.begin(), "abfgcdeh", f1.size()) ) printf("err 5\n");
		f1.removeindex(2);
		if( 0!=memcmp(f1.begin(), "abgcdeh", f1.size()) ) printf("err 6\n");
		f1.removeindex(2, 2);
		if( 0!=memcmp(f1.begin(), "abdeh", f1.size()) ) printf("err 7\n");
		f1.strip(3);
		if( 0!=memcmp(f1.begin(), "ab", f1.size()) ) printf("err 8\n");
		f1.assign('a');
		if( 0!=memcmp(f1.begin(), "a", f1.size()) ) printf("err 9\n");
		f1.assign('b', 3);
		if( 0!=memcmp(f1.begin(), "bbb", f1.size()) ) printf("err 10\n");
		f1.append("aa", 2);
		if( 0!=memcmp(f1.begin(), "bbbaa", f1.size()) ) printf("err 11\n");
		f1.append('b');
		if( 0!=memcmp(f1.begin(), "bbbaab", f1.size()) ) printf("err 12\n");
		f1.append('c', 3);
		if( 0!=memcmp(f1.begin(), "bbbaabccc", f1.size()) ) printf("err 13\n");
		f1.insert("abc", 3, 2);
		if( 0!=memcmp(f1.begin(), "bbabcbaabccc", f1.size()) ) printf("err 14\n");
		f1.insert('v', 4, 1);
		if( 0!=memcmp(f1.begin(), "bvvvvbabcbaabccc", f1.size()) ) printf("err 15\n");
		f1.copy("_copy_", 6, 5);
		if( 0!=memcmp(f1.begin(), "bvvvv_copy_abccc", f1.size()) ) printf("err 16\n");
		f1.replace("_rep_", 5, 3, 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccc", f1.size()) ) printf("err 17\n");
		a = f1.first();
		if( a != 'b' ) printf("err 18\n");
		a = f1.last();
		if( a != 'c' ) printf("err 19\n");
		f1.push('z');
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abcccz", f1.size()) ) printf("err 20\n");
		f1.push("xy", 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccczxy", f1.size()) ) printf("err 21\n");
		a = f1.pop();
		if( 0!=memcmp(f1.begin(), "vv_rep__copy_abccczxy", f1.size()) ) printf("err 22\n");
		if( a != 'b' ) printf("err 23\n");
		char el;
		f1.pop(el);
		if( 0!=memcmp(f1.begin(), "v_rep__copy_abccczxy", f1.size()) ) printf("err 24\n");
		if( el != 'v' ) printf("err 25\n");
		a = f1.top();
		if( a != 'v' ) printf("err 26\n");
		f1.top(el);
		if( el != 'v' ) printf("err 27\n");
/////////////////////////////////////////////////////

	}
	return 0;
}
