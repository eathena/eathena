
#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basealgo.h"
#include "basetime.h"
#include "basememory.h"
#include "basestring.h"
#include "basestrformat.h"
#include "basestrsearch.h"
#include "basesync.h"
#include "baseexceptions.h"
#include "basearray.h"





///////////////////////////////////////////////////////////////////////////
void vector_error(const char*errmsg)
{
#ifdef CHECK_EXCEPTIONS
	throw exception_bound(errmsg);
#else
	printf("%s\n", errmsg);
#endif
}


///////////////////////////////////////////////////////////////////////////
template<typename T, typename E, typename A> 
void vectorbase<T,E,A>::debug_print()
{
	string<> str;
	typename A::iterator iter(*this);
	while(iter)
		str << *iter++ << ' ';
	printf( (const char*)str );
	printf("\n");
}


///////////////////////////////////////////////////////////////////////////
// explicitely instanciate void* vectors
template class vector<void*>;








///////////////////////////////////////////////////////////////////////////
// predeclaration
class publisher;
class subscriber;
class singlesubscriber;
class multisubscriber;

///////////////////////////////////////////////////////////////////////////
// subscriber interface
class subscriber : public defaultcmp, public Mutex
{
public:
	subscriber()			{}
	virtual ~subscriber()	{}
	
	///////////////////////////////////////////////////////////////////////
	// connect/disconnect
	virtual void connect(publisher& pub) =0;
	virtual void disconnect(publisher& pub) =0;
	virtual void disconnect() =0;
	virtual void notify()	{}
};



///////////////////////////////////////////////////////////////////////////
// publisher
// can connect to a multiple subscriber objects
class publisher : public defaultcmp, public Mutex
{
	friend class singlesubscriber;
	friend class multisubscriber;
	ptrslist<subscriber> subs;
public:
	///////////////////////////////////////////////////////////////////////
	// constructor/destructor
	publisher()		{}
	~publisher()	{ this->disconnect(); }

	///////////////////////////////////////////////////////////////////////
	// connect a subscriber
	void connect(subscriber& sub)
	{
		sub.connect(*this);
	}
	///////////////////////////////////////////////////////////////////////
	// disconnect a subscriber
	void disconnect(subscriber& sub)
	{
		sub.disconnect(*this);
	}
	///////////////////////////////////////////////////////////////////////
	// disconnect all
	void disconnect()
	{
		ScopeLock sl(*this);
		size_t i;
		while( (i=this->subs.size())>0 )
		{
			if( this->subs[i-1] )
				this->subs[i-1]->disconnect(*this);
		}
	}
	///////////////////////////////////////////////////////////////////////
	// call function func in subscribers of type T (or derived)
	template<typename T> void call( void (T::*func)(void) )
	{
		size_t i;
		for(i=0; i<this->subs.size(); i++)
		{
			if( this->subs[i] )
			{
				T*ptr = dynamic_cast<T*>( this->subs[i] );
				if( ptr ) (ptr->*func)();
			}
		}
	}
	///////////////////////////////////////////////////////////////////////
	// call function func with parameter p1 in subscribers of type T (or derived)
	template<typename T, typename X> void call( void (T::*func)(X p1), X p1 )
	{
		size_t i;
		for(i=0; i<this->subs.size(); i++)
		{
			if( this->subs[i] )
			{
				T*ptr = dynamic_cast<T*>( this->subs[i] );
				if( ptr ) (ptr->*func)(p1);
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////
// single subscriber
// can connect to a (single) publisher object
class singlesubscriber : public subscriber
{
	publisher* pub;
public:
	singlesubscriber() : pub(NULL)	{}
	virtual ~singlesubscriber()		{ this->disconnect(); }
	
	///////////////////////////////////////////////////////////////////////
	// connect/disconnect
	virtual void connect(publisher& pub)
	{
		if( this->pub != &pub && (void*)this != (void*)&pub )
		{
			ScopeLock sl(pub);
			if( this->pub )
				this->disconnect(*this->pub);
			this->pub = &pub;
			pub.subs.append(this);
		}
	}
	virtual void disconnect(publisher& pub)
	{
		if( this->pub == &pub )
		{
			ScopeLock sl(pub);
			this->pub = NULL;
			size_t pos;
			if( pub.subs.find(this, 0, pos) )
				pub.subs.removeindex(pos);
		}
	}
	virtual void disconnect()
	{
		if( this->pub )
		{
			ScopeLock sl(*this);
			ScopeLock sp(*this->pub);
			size_t pos;
			if( this->pub->subs.find(this, 0, pos) )
				this->pub->subs.removeindex(pos);
			this->pub=NULL;
		}
	}
};


///////////////////////////////////////////////////////////////////////////
// multi subscriber
// can connect to multiple publisher object
class multisubscriber : public subscriber
{
	ptrslist<publisher> pubs;
public:
	multisubscriber()			{}
	virtual ~multisubscriber()	{ this->disconnect(); }
	
	///////////////////////////////////////////////////////////////////////
	// connect/disconnect
	virtual void connect(publisher& pub)
	{	
		if( (void*)&pub!=(void*)this )
		{
			ScopeLock sl(*this);
			ScopeLock sp(pub);
			size_t pos;
			if( !this->pubs.find(&pub, 0, pos) )
			{
				this->pubs.append(&pub);
				pub.subs.append(this);
			}
		}
	}
	virtual void disconnect(publisher& pub)
	{	
		ScopeLock sl(*this);
		ScopeLock sp(pub);
		size_t pos;
		if( this->pubs.find(&pub, 0, pos) )
			this->pubs.removeindex(pos);
		if( pub.subs.find(this, 0, pos) )
			pub.subs.removeindex(pos);
	}
	virtual void disconnect()
	{	// disconnect from all
		ScopeLock sl(*this);
		size_t i, pos;
		for(i=0; i<this->pubs.size(); i++)
		{
			if( this->pubs[i] )
			{
				ScopeLock sl(*this->pubs[i]);
				if( this->pubs[i]->subs.find(this, 0, pos) )
					this->pubs[i]->subs.removeindex(pos);
				this->pubs[i]=NULL;
			}
		}
		this->pubs.clear();
	}
};

///////////////////////////////////////////////////////////////////////////
// component
// publisher/subscriber combo
class component : public publisher, public singlesubscriber
{
public:
	component()				{}
	virtual ~component()	{}

	virtual void disconnect()
	{
		this->publisher::disconnect();
		this->singlesubscriber::disconnect();
	}
};

class subtest1 : public singlesubscriber
{
public:

	subtest1()	{}

	void functest0()
	{
		printf("1. ft %p\n", this);
	}
	void functest1(int param)
	{
		printf("1. ft %p -> %i\n", this, param);
	}
};


class subtest2 : public subtest1
{
public:

	subtest2()	{}

	void functest0()
	{
		printf("2. ft %p\n", this);
	}
	void functest1(int param)
	{
		printf("2. ft %p -> %i\n", this, param);
	}
};




class XXX
{
public:
	ssize_t i;
	string<> s;
	XXX() : i((ssize_t)this)	{}
	XXX(const char*str) : i(1), s(str)	{}
};



void test_array(void)
{
#if defined(DEBUG)

	{
		publisher p;

		singlesubscriber s1;
		p.connect(s1);

		subtest1 st1;
		p.connect(st1);

		{
			multisubscriber s2;
			p.connect(s2);


			subtest2 st2;
			p.connect(st2);

			p.call( &subtest1::functest0 );
			p.call( &subtest2::functest1, 1 );
		}

		p.disconnect();	
	}

	{
		map<uint32, TObjPtr<XXX> > ptmap;

		XXX tmp1("hallo");
		XXX tmp2;
		XXX tmp3("xxx");

		ptmap.insert(1, TObjPtr<XXX>(tmp1) );
		ptmap.insert(2, TObjPtr<XXX>(tmp2) );
		ptmap.insert(3, TObjPtr<XXX>(tmp3) );

		TObjPtr<XXX> aa;

		aa = ptmap[1];
		aa = ptmap[2];
	}

	{
		dualmap<string<>,int, int>	dm;

		dm.insert("one", 1, 1);
		dm.insert("two", 2, 2);

		dm.insert("one", 1, 1);
		dm.insert("one", 2, 1);
		dm.insert("one", 4, 1);
		dm.insert("xxx", 2, 1);

		try
		{
		int xx = dm["none"];
		xx = dm[2];
		}
		catch(exception& e)
		{
			printf("exception: %s\n", e.what());
		}

		dualmap<string<>,int, int>::iterator iter( dm() );
		while(iter)
		{
			printf("%s %i %i\n", (const char*)iter->key1, iter->key2, iter->data);
			++iter;
		}
	}

	{
		objvector<int> ivec;

		ivec.append(5);
		ivec.append(6);
		ivec.insert(0,2,1);
		ivec.append(1);

		objslist<int> isl;

		isl.append(1);
		isl.append(3);
		isl.insert(0,2,1);
		isl.append(1);

		isl = ivec;

		objslist<int>::iterator iter(isl);
		while( iter )
			printf("%i ", **iter++);

		iter = isl.begin();
	}



	{
		map<int, int> a;
		printf("sz of map: %i\n", sizeof(a) );
		//map<size_t, size_t> imap;

		//imap[1] = 5;
	
	}
	{

		string<> printtest = dprintf("%i %lf", 3, 3.3);

		printtest << 3.3;



		match_wildcard("*.[-d-z]a*", "hallo.xabmas");


		conststring<> ccc("hallo");


		ccc = "abcd";


		string<> test = "asd;jkl:asd;lkj";

//		ccc = test;


		vector< string<> > ret1 = split(test,';');
		vector< string<> > ret2 = split(test,";:");

		size_t i;
		for(i=0; i<ret1.size(); i++)
			printf("%s\n", (const char*)ret1[i]);
		for(i=0; i<ret2.size(); i++)
			printf("%s\n", (const char*)ret2[i]);


		test << format<ulong>("%ul", 3);

		test = bytestostring(12345678l);
		printf("%s\n", (const char*)test );

		char buf[100];
		conststring<> cs("hallo");
		staticstring<> ss(buf,100);
		basestring<> bs="hallo";


		bool rr;

		rr = (ss == ss);
		rr = (ss == bs);
		rr = (bs == ss);
		rr = (ss == cs);
		rr = (cs == ss);


		rr = (bs == bs);
		rr = (test == test);

		rr = (test == bs);
		rr = (bs == test);

		rr = (cs == test);
		rr = (test == cs);

		rr = (ss == test);
		rr = (test == ss);

		rr = (cs == "ab");
		rr = ("ab" == cs);
		rr = ("ab" == test);
		rr = (test == "ab");

	}














	ptrvector<char> ptrvec, ptrvec2;

	ptrvec.push("hallo");
	ptrvec.resize(6);
	ptrvec.move(2, 3, 2);
	ptrvec.removeindex(1);
	ptrvec.removeindex(2, 2);
	ptrvec.strip(1);
	ptrvec.clear();
	
	char *arr[] = {"hallo", "ballo", "test"};

	ptrvec.assign(arr, 3);
	ptrvec.assign("noo");
	ptrvec.assign("xxx", 2);
	ptrvec2.assign(ptrvec);
	ptrvec.append(arr, 3);
	ptrvec.append("qee");
	ptrvec.append("vis", 2);
	ptrvec.append(ptrvec2);

	ptrvec.insert(arr, 2, 1);
	ptrvec.insert("ggg", 1, 3);

	ptrvec.insert(ptrvec2, 3);

	ptrvec.copy(arr, 3, 1);
	ptrvec.copy(ptrvec2, 2);

	ptrvec.replace(arr, 3, 1, 5);

	ptrvec.replace(ptrvec2, 1, 2);

	char* 
	ret = ptrvec(1);
	ret = ptrvec[2];


	ptrvec.push( "zzz" );
	ptrvec.push( arr, 2);
	ptrvec.push(ptrvec2);
	
	ret = ptrvec.pop();
	ptrvec.pop(ret);
	ret = ptrvec.top();
	ptrvec.top(ret);









	//!! TODO copy testcases from caldon
	{
		printf("TArray vs. vector\n");
		size_t runs=1000, elems=1000;
		size_t i,k;
		ulong tick;
		TArrayDST<char> arr;
		vector<char> vec;

		tick = clock();

		for(k=0; k<runs;k++, arr.resize(1), arr.realloc(1000))
		for(i=0; i<elems; i++)
		{
			arr.append( (char)i );
		}
		printf("tarray %lu  (%lu,%lu)\n", clock()-tick, (ulong)runs, (ulong)elems);


		tick = clock();
		for(k=0; k<runs;k++, vec.resize(1), vec.realloc(1000))
		for(i=0; i<elems; i++)
		{			
			vec.append( (char)i );
		}
		printf("vector: %lu (%lu,%lu)\n", clock()-tick, (ulong)runs, (ulong)elems);

	}


	{
		printf("vector free tests\n");
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


		printf("vector push order\n");
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

		printf("fifo push order\n");
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


		printf("slist push \n");
		slist<char> ss;
		ss.push('a'); ss.debug_print();
		ss.push(ff); ss.debug_print();
		ss.insert("kksj", 4, 2); ss.debug_print();

		ss.append('g'); ss.debug_print();

		printf("slist element access (with out-of-bounds)\n");
		int a = ss[3];
		try
		{
			a = ss[455];
		}
		catch(exception e)
		{
			printf("catched: %s", e.what());
		}
		ss[3] = 'z'; 

		try
		{
			ss(300) = '.'; 
		}
		catch(exception e)
		{
			printf("catched: %s", e.what());
		}
		ss.debug_print(); 
		printf("slist order destroyed\n");
		ss.sort();
		ss.debug_print(); 
		printf("slist order restored\n");

		size_t pos;
		if( ss.find('m', 0, pos) )
			printf("'m' found at pos %lu\n", (ulong)pos);
		else
			printf("'m' not found (pos %lu)\n", (ulong)pos);
		if( ss.find('k', 0, pos) )
			printf("'k' found at pos %lu\n", (ulong)pos);
		else
			printf("'k' not found (pos %lu)\n", (ulong)pos);

	}




	/////////////////////////////////////////////////////
	{
		int err=0;
		int a;
		printf("vector test sequence\n");
		vector<char> f1;
		f1.clear();
		if( f1.size()!=0 ) printf("err 1\n");
		f1.realloc(10);
		if( f1.size()!=0 ) printf("err 2\n");
		f1.resize(12);
		if( f1.size()!=12 ) printf("err 3\n");
		f1.clear();
		f1.assign("abcdefgh", 8);
		if( f1.size()!=8 || 0!=memcmp(f1.begin(), "abcdefgh", 0) ) err++,printf("err 4\n");
		f1.move(2, 5, 2);
		if( 0!=memcmp(f1.begin(), "abfgcdeh", f1.size()) ) err++,printf("err 5\n");
		f1.removeindex(2);
		if( 0!=memcmp(f1.begin(), "abgcdeh", f1.size()) ) err++,printf("err 6\n");
		f1.removeindex(2, 2);
		if( 0!=memcmp(f1.begin(), "abdeh", f1.size()) ) err++,printf("err 7\n");
		f1.strip(3);
		if( 0!=memcmp(f1.begin(), "ab", f1.size()) ) err++,printf("err 8\n");
		f1.assign('a');
		if( 0!=memcmp(f1.begin(), "a", f1.size()) ) err++,printf("err 9\n");
		f1.assign('b', 3);
		if( 0!=memcmp(f1.begin(), "bbb", f1.size()) ) err++,printf("err 10\n");
		f1.append("aa", 2);
		if( 0!=memcmp(f1.begin(), "bbbaa", f1.size()) ) err++,printf("err 11\n");
		f1.append('b');
		if( 0!=memcmp(f1.begin(), "bbbaab", f1.size()) ) err++,printf("err 12\n");
		f1.append('c', 3);
		if( 0!=memcmp(f1.begin(), "bbbaabccc", f1.size()) ) err++,printf("err 13\n");
		f1.insert("abc", 3, 2);
		if( 0!=memcmp(f1.begin(), "bbabcbaabccc", f1.size()) ) err++,printf("err 14\n");
		f1.insert('v', 4, 1);
		if( 0!=memcmp(f1.begin(), "bvvvvbabcbaabccc", f1.size()) ) err++,printf("err 15\n");
		f1.copy("_copy_", 6, 5);
		if( 0!=memcmp(f1.begin(), "bvvvv_copy_abccc", f1.size()) ) err++,printf("err 16\n");
		f1.replace("_rep_", 5, 3, 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccc", f1.size()) ) err++,printf("err 17\n");
		a = f1.first();
		if( a != 'b' ) err++,printf("err 18\n");
		a = f1.last();
		if( a != 'c' ) err++,printf("err 19\n");
		f1.push('z');
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abcccz", f1.size()) ) err++,printf("err 20\n");
		f1.push("xy", 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccczxy", f1.size()) ) err++,printf("err 21\n");
		a = f1.pop();
		if( 0!=memcmp(f1.begin(), "vv_rep__copy_abccczxy", f1.size()) ) err++,printf("err 22\n");
		if( a != 'b' ) err++,printf("err 23\n");
		char el;
		f1.pop(el);
		if( 0!=memcmp(f1.begin(), "v_rep__copy_abccczxy", f1.size()) ) err++,printf("err 24\n");
		if( el != 'v' ) err++,printf("err 25\n");
		a = f1.top();
		if( a != 'v' ) err++,printf("err 26\n");
		f1.top(el);
		if( el != 'v' ) err++,printf("err 27\n");
		printf("vector test sequence finished, errors: %i\n", err);
	}
	/////////////////////////////////////////////////////
	{
		int err=0;
		int a;
		printf("stack test sequence\n");
		stack<char> f1;
		f1.clear();
		if( f1.size()!=0 ) printf("err 1\n");
		f1.realloc(10);
		if( f1.size()!=0 ) printf("err 2\n");
		f1.resize(12);
		if( f1.size()!=12 ) printf("err 3\n");
		f1.clear();
		f1.assign("abcdefgh", 8);
		if( f1.size()!=8 || 0!=memcmp(f1.begin(), "abcdefgh", 0) ) err++,printf("err 4\n");
		f1.move(2, 5, 2);
		if( 0!=memcmp(f1.begin(), "abfgcdeh", f1.size()) ) err++,printf("err 5\n");
		f1.removeindex(2);
		if( 0!=memcmp(f1.begin(), "abgcdeh", f1.size()) ) err++,printf("err 6\n");
		f1.removeindex(2, 2);
		if( 0!=memcmp(f1.begin(), "abdeh", f1.size()) ) err++,printf("err 7\n");
		f1.strip(3);
		if( 0!=memcmp(f1.begin(), "ab", f1.size()) ) err++,printf("err 8\n");
		f1.assign('a');
		if( 0!=memcmp(f1.begin(), "a", f1.size()) ) err++,printf("err 9\n");
		f1.assign('b', 3);
		if( 0!=memcmp(f1.begin(), "bbb", f1.size()) ) err++,printf("err 10\n");
		f1.append("aa", 2);
		if( 0!=memcmp(f1.begin(), "bbbaa", f1.size()) ) err++,printf("err 11\n");
		f1.append('b');
		if( 0!=memcmp(f1.begin(), "bbbaab", f1.size()) ) err++,printf("err 12\n");
		f1.append('c', 3);
		if( 0!=memcmp(f1.begin(), "bbbaabccc", f1.size()) ) err++,printf("err 13\n");
		f1.insert("abc", 3, 2);
		if( 0!=memcmp(f1.begin(), "bbabcbaabccc", f1.size()) ) err++,printf("err 14\n");
		f1.insert('v', 4, 1);
		if( 0!=memcmp(f1.begin(), "bvvvvbabcbaabccc", f1.size()) ) err++,printf("err 15\n");
		f1.copy("_copy_", 6, 5);
		if( 0!=memcmp(f1.begin(), "bvvvv_copy_abccc", f1.size()) ) err++,printf("err 16\n");
		f1.replace("_rep_", 5, 3, 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccc", f1.size()) ) err++,printf("err 17\n");
		a = f1.first();
		if( a != 'b' ) err++,printf("err 18\n");
		a = f1.last();
		if( a != 'c' ) err++,printf("err 19\n");
		f1.push('z');
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abcccz", f1.size()) ) err++,printf("err 20\n");
		f1.push("xy", 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccczxy", f1.size()) ) err++,printf("err 21\n");
		a = f1.pop();
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccczx", f1.size()) ) err++,printf("err 22\n");
		if( a != 'y' ) err++,printf("err 23\n");
		char el;
		f1.pop(el);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abcccz", f1.size()) ) err++,printf("err 24\n");
		if( el != 'x' ) err++,printf("err 25\n");
		a = f1.top();
		if( a != 'z' ) err++,printf("err 26\n");
		f1.top(el);
		if( el != 'z' ) err++,printf("err 27\n");
		printf("stack test sequence finished, errors: %i\n", err);
	}
	/////////////////////////////////////////////////////
	{
		int err=0;
		int a;
		printf("fifo test sequence\n");
		fifo<char> f1;
		f1.clear();
		if( f1.size()!=0 ) err++,printf("err 1\n");
		f1.realloc(10);
		if( f1.size()!=0 ) err++,printf("err 2\n");
		f1.resize(12);
		if( f1.size()!=12 ) err++,printf("err 3\n");
		f1.clear();
		f1.assign("abcdefgh", 8);
		if( f1.size()!=8 || 0!=memcmp(f1.begin(), "abcdefgh", 0) ) err++,printf("err 4\n");
		f1.move(2, 5, 2);
		if( 0!=memcmp(f1.begin(), "abfgcdeh", f1.size()) ) err++,printf("err 5\n");
		f1.removeindex(2);
		if( 0!=memcmp(f1.begin(), "abgcdeh", f1.size()) ) err++,printf("err 6\n");
		f1.removeindex(2, 2);
		if( 0!=memcmp(f1.begin(), "abdeh", f1.size()) ) err++,printf("err 7\n");
		f1.strip(3);
		if( 0!=memcmp(f1.begin(), "ab", f1.size()) ) err++,printf("err 8\n");
		f1.assign('a');
		if( 0!=memcmp(f1.begin(), "a", f1.size()) ) err++,printf("err 9\n");
		f1.assign('b', 3);
		if( 0!=memcmp(f1.begin(), "bbb", f1.size()) ) err++,printf("err 10\n");
		f1.append("aa", 2);
		if( 0!=memcmp(f1.begin(), "bbbaa", f1.size()) ) err++,printf("err 11\n");
		f1.append('b');
		if( 0!=memcmp(f1.begin(), "bbbaab", f1.size()) ) err++,printf("err 12\n");
		f1.append('c', 3);
		if( 0!=memcmp(f1.begin(), "bbbaabccc", f1.size()) ) err++,printf("err 13\n");
		f1.insert("abc", 3, 2);
		if( 0!=memcmp(f1.begin(), "bbabcbaabccc", f1.size()) ) err++,printf("err 14\n");
		f1.insert('v', 4, 1);
		if( 0!=memcmp(f1.begin(), "bvvvvbabcbaabccc", f1.size()) ) err++,printf("err 15\n");
		f1.copy("_copy_", 6, 5);
		if( 0!=memcmp(f1.begin(), "bvvvv_copy_abccc", f1.size()) ) err++,printf("err 16\n");
		f1.replace("_rep_", 5, 3, 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccc", f1.size()) ) err++,printf("err 17\n");
		a = f1.first();
		if( a != 'b' ) err++,printf("err 18\n");
		a = f1.last();
		if( a != 'c' ) err++,printf("err 19\n");
		f1.push('z');
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abcccz", f1.size()) ) err++,printf("err 20\n");
		f1.push("xy", 2);
		if( 0!=memcmp(f1.begin(), "bvv_rep__copy_abccczxy", f1.size()) ) err++,printf("err 21\n");
		a = f1.pop();
		if( 0!=memcmp(f1.begin(), "vv_rep__copy_abccczxy", f1.size()) ) err++,printf("err 22\n");
		if( a != 'b' ) err++,printf("err 23\n");
		char el;
		f1.pop(el);
		if( 0!=memcmp(f1.begin(), "v_rep__copy_abccczxy", f1.size()) ) err++,printf("err 24\n");
		if( el != 'v' ) err++,printf("err 25\n");
		a = f1.top();
		if( a != 'v' ) err++,printf("err 26\n");
		f1.top(el);
		if( el != 'v' ) err++,printf("err 27\n");
		printf("fifo test sequence finished, errors %i\n", err);
	}
	/////////////////////////////////////////////////////


#endif//DEBUG
}
