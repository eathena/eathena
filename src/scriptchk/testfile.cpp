
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "basesafeptr.h"
#include "basetime.h"


void vartest();


class memorymodule
{
public:
	memorymodule()	{}
	~memorymodule()	{}

	void *operator new (size_t sz)
	{
		void *a = malloc(sz);
		if(NULL==a) throw exception_memory("memory allocation failed");
		printf("memorymodule operator new sz=%u, returning %p\n", sz, a);
		return a;
	}
	void *operator new[] (size_t sz)
	{
		void *a = malloc(sz);
		if(NULL==a) exception_memory("memory allocation failed");
		printf("memorymodule operator new[] sz=%u, returning %p\n", sz, a);
		return a;
	}

	void operator delete (void * a)
	{
		printf("memorymodule operator delete deleting %p\n", a);
		if(a)
			free(a);
	}

	void operator delete[] (void *a)
	{
		printf("memorymodule operator delete[] deleting %p\n", a);
		if(a)
			free(a);
	}
};

class testclass : public memorymodule
{
	int a;
public:
	testclass()	{}
	~testclass()	{}

};

class AA
{
public:
	AA()	{ printf("ac\n"); }
	~AA()	{ printf("ad\n"); }

	int aa;

	int func()	{ return aa; }
};
class BB : public AA
{
public:
	BB()	{ printf("bc\n"); }
	~BB()	{ printf("bd\n"); }
	int bb;

	int func()	{ return bb; }
};



extern inline int newt_sqrt(int input)
{
	int new_value, value = input/2, count = 0;
	if (!value) //Division by zero fix, pointed out by Shinomori. [Skotlex]
		return input;
	do
	{
		new_value = (value + input/value)>>1;
		if (abs(value - new_value) <= 1)
			return new_value;
		value = new_value;
	}
	while (count++ < 25);
	return new_value;
}
// newton approximation
template<class T> extern inline T sqrt1(T n)
{
	if(n>0)
	{
		T q=0, qx=n/2;

		while( q!=qx && q+1!=qx )
		{
			q = qx;
			qx = (q + n/q)/2;
		}
		return q;
	}
	// should set matherr or throw something
	return 0;
}
// newton approximation
template<class T> extern inline T sqrt2(T n)
{
	if(n>0)
	{
		
		T q=0, qx = n>>(log2(n)/2);

		while( q!=qx && q+1!=qx )
		{
			q = qx;
			qx = (q + n/q)/2;
		}
		return q;
	}
	// should set matherr or throw something
	return 0;
}
// newton approximation
template<class T> extern inline T sqrt3(T n)
{
	if(n>0)
	{
		T q=0, qx = 1<<(log2(n)/2);

		while( q!=qx && q+1!=qx )
		{
			q = qx;
			qx = (q + n/q)/2;
		}
		return q;
	}
	// should set matherr or throw something
	return 0;
}
// newton approximation
template<class T> extern inline T sqrt4(T n)
{
	if(n>0)
	{
		T q=0, xx = (log2(n)/2), qx = ((n>>xx) + (1<<xx))/2;
		do
		{
			q  = qx;
			qx = (q + n/q)/2;
		}
		while( q!=qx && q+1!=qx );
		return q;
	}
	// should set matherr or throw something
	return 0;
}


// newton approximation
template<class T> extern inline T sqrt4test(T n)
{
	if(n>0)
	{
		T q=0, xx = (log2(n)/2), qx = ((n>>xx) + (1<<xx))/2;

		printf("srt %8u (", n);
		do
		{
			printf("%u->", qx);
			q  = qx;
			qx = (q + n/q)/2;
			printf("%u, ", qx);
		}
		while( q!=qx && q+1!=qx );
		
		printf(")(%u) = %u\n",qx,q);
		return q;
	}
	// should set matherr or throw something
	return 0;
}


void base1(int a, va_list ap)
{
	
	int b = va_arg(ap,int);
	int c = va_arg(ap,int);
	int d = va_arg(ap,int);
	int e = va_arg(ap,int);
	printf("call1: %i %i %i %i %i\n", a, b, c, d, e);

}
void base2(int a, va_list ap)
{
	int b = va_arg(ap,int);
	void* c = va_arg(ap,void*);
	int d = va_arg(ap,int);
	int e = va_arg(ap,int);

	printf("call1: %i %i %p %i %i\n", a, b, c, d, e);
}





void call(void (*func)(int,va_list), ...)
{
	va_list ap;
	va_start(ap, func);

	func(1, ap);
	func(2, ap);
	
	va_end(ap);
}

int argtest(void)
{
	void *x = (void *)&argtest;

	call(base1,2,3,4,5);
	call(base2,2,x,4,5);

	return 0;
}



template <class T> class TScopeChange
{
	T& val;
	T  tar;
public:
	TScopeChange(T& v, const T&t) : val(v), tar(t)	{}
	~TScopeChange()			{ val=tar; }
	void disable()			{ tar = val; }
	void set(const T& t)	{ tar = t; }
};













class xlist : public noncopyable
{
	class xelem
	{
		friend class iterator;
		xelem* next;
	public:

		xelem() : next(NULL)	{}
	};
	xelem* list;
public:
	friend class iterator;
	xlist():list(NULL)	{}
};


class iterator : public noncopyable
{
	const xlist& x;
	xlist::xelem* elem;
public:
	void next()	{ if(elem) elem = elem->next; }

	iterator(const xlist& xx) : x(xx), elem(xx.list)	{}
};


template <class T> class list : public xlist
{
public:

};


template <class T> class xyz
{
public:
	int& getstart() { static int a=0; return a; }
};



class CDLinkNode
{
	CDLinkNode	*mpPrev;
	CDLinkNode	*mpNext;
public:
	CDLinkNode() : mpPrev(NULL), mpNext(NULL)
	{}
	CDLinkNode(CDLinkNode& root) : mpPrev(&root), mpNext(root.mpNext)
	{	// double link with one anchor,
		// add in front of all others
		root.mpNext = this;
		if(this->mpNext) this->mpNext->mpPrev = this;
	}
	CDLinkNode(CDLinkNode& head, CDLinkNode& tail) : mpPrev(tail.mpPrev), mpNext(&tail)
	{	// double link with two anchors
		// add at the end of the existing list
		if(!head.mpNext)
		{
			this->mpPrev = &head;
			head.mpNext = tail.mpPrev=this;
		}
		else if(this->mpPrev)
		{
			tail.mpPrev = this->mpPrev->mpNext = this;
		}
	}
	~CDLinkNode()	{ unlink(); }
	void link(CDLinkNode& root)
	{
		unlink();
		this->mpPrev = &root;
		this->mpNext = root.mpNext;
		root.mpNext = this;
		if(this->mpNext) this->mpNext->mpPrev = this;
	}
	void link(CDLinkNode& head, CDLinkNode& tail)
	{
		unlink();
		this->mpNext = &tail;
		this->mpPrev = tail.mpPrev;
		if(!head.mpNext)
		{	// first node insertion with neither head<->tail connected
			this->mpPrev = &head;
			head.mpNext = tail.mpPrev=this;
		}
		else if(this->mpPrev)
		{
			tail.mpPrev = this->mpPrev->mpNext = this;
		}
	}
	void unlink()
	{
		if(this->mpPrev) this->mpPrev->mpNext=this->mpNext;
		if(this->mpNext) this->mpNext->mpPrev=this->mpPrev;
		this->mpNext=NULL;
		this->mpPrev=NULL;
	}

	CDLinkNode* next()	{ return this->mpNext; }
	CDLinkNode* prev()	{ return this->mpPrev; }
};

















#define HUFFBITS unsigned long
#define HTN	34
#define MXOFF	250
 
struct huffcodetab
{
	char tablename[3];			// string, containing table_description
	unsigned int xlen;			// max. x-index+
	unsigned int ylen;			// max. y-index+
	unsigned int linbits;		// number of linbits
	unsigned int linmax;		// max number to be stored in linbits
	int ref;					// a positive value indicates a reference
	HUFFBITS *table;			// pointer to array[xlen][ylen]
	unsigned char *hlen;		// pointer to array[xlen][ylen]
	unsigned char(*val)[2];		// decoder tree
	unsigned int treelen;		// length of decoder tree
};

HUFFBITS dmask = 1 << (8*sizeof(HUFFBITS)-1);
unsigned int hs = sizeof(HUFFBITS)*8;

struct huffcodetab ht[HTN];	// array of all huffcodtable headers
				// 0..31 Huffman code table 0..31
				// 32,33 count1-tables


int read_huffcodetab(FILE *fi)
{
	char line[100],command[40],huffdata[40];
	unsigned int t,i,j,k,nn,x,y,n=0;
	unsigned int xl, yl, len;
	HUFFBITS h;
	do
	{
		fgets(line,99,fi);
	}while ((line[0] == '#') || (line[0] < ' ') );
	do
	{
		while ((line[0]=='#') || (line[0] < ' '))
		{
			fgets(line,99,fi);
		}
		
		sscanf(line,"%s %s %u %u %u",command,ht[n].tablename, &xl,&yl,&ht[n].linbits);
		if (strcmp(command,".end")==0)
			return n;
		else if (strcmp(command,".table")!=0)
		{
			fprintf(stderr,"huffman table %u data corrupted\n",n);
			return -1;
		}
		ht[n].linmax = (1<<ht[n].linbits)-1;
		
		sscanf(ht[n].tablename,"%u",&nn);
		if (nn != n)
		{
			fprintf(stderr,"wrong table number %u\n",n);
			return(-2);
		} 
		ht[n].xlen = xl;
		ht[n].ylen = yl;

		do
		{
			fgets(line,99,fi);
		} while ((line[0] == '#') || (line[0] < ' '));
		
		sscanf(line,"%s %u",command,&t);
		if (strcmp(command,".reference")==0)
		{
			ht[n].ref   = t;
			ht[n].table = ht[t].table;
			ht[n].hlen  = ht[t].hlen;
			if ( (xl != ht[t].xlen) || (yl != ht[t].ylen)  )
			{
				fprintf(stderr,"wrong table %u reference\n",n);
				return (-3);
			}
			do
			{
				fgets(line,99,fi);
			} while ((line[0] == '#') || (line[0] < ' ') );
		}
		else
		{
			ht[n].ref  = -1;
			ht[n].table=(HUFFBITS *) calloc(xl*yl,sizeof(HUFFBITS));
			if (ht[n].table == NULL)
			{
				fprintf(stderr,"unsufficient heap error\n");
				return (-4);
			}
			ht[n].hlen=(unsigned char *) calloc(xl*yl,sizeof(unsigned char));
			if (ht[n].hlen == NULL)
			{
				fprintf(stderr,"unsufficient heap error\n");
				return (-4);
			}
			for (i=0; i<xl; i++)
			for (j=0;j<yl; j++)
			{
				if (xl>1) 
					sscanf(line,"%u %u %u %s",&x, &y, &len,huffdata);
				else 
					sscanf(line,"%u %u %s",&x,&len,huffdata);
				h=0;k=0;
				while (huffdata[k])
				{
					h <<= 1;
					if (huffdata[k] == '1')
						h++;
					else if (huffdata[k] != '0')
					{
						fprintf(stderr,"huffman-table %u bit error\n",n);
						return (-5);
					}
					k++;
				}
				if (k != len)
				{
					fprintf(stderr, "warning: wrong codelen in table %u, pos [%2u][%2u]\n", n,i,j);
				}
				ht[n].table[i*xl+j] = h;
				ht[n].hlen[i*xl+j] = (unsigned char) len;
				do
				{
					fgets(line,99,fi);
				} while ((line[0] == '#') || (line[0] < ' '));
			}
		}
		n++;
	}while (1);
	return 0;
}

// read the huffman decoder table
int read_decoder_table(FILE *fi)
{
	int n,i,nn,t;
	unsigned int v0,v1;
	char command[100],line[100];
	for (n=0;n<HTN;n++)
	{
		// .table number treelen xlen ylen linbits
		do
		{
			fgets(line,99,fi);
		} while ((line[0] == '#') || (line[0] < ' '));
		
		sscanf(line,"%s %s %u %u %u %u",command,ht[n].tablename, &ht[n].treelen, &ht[n].xlen, &ht[n].ylen, &ht[n].linbits);
		if (strcmp(command,".end")==0)
			return n;
		else if (strcmp(command,".table")!=0)
		{
			fprintf(stderr,"huffman table %u data corrupted\n",n);
			return -1;
		}
		ht[n].linmax = (1<<ht[n].linbits)-1;
		sscanf(ht[n].tablename,"%u",&nn);
		if (nn != n)
		{
			fprintf(stderr,"wrong table number %u\n",n);
			return(-2);
		}
		do
		{
			fgets(line,99,fi);
		} while ((line[0] == '#') || (line[0] < ' '));
		
		sscanf(line,"%s %u",command,&t);
		if (strcmp(command,".reference")==0)
		{
			ht[n].ref   = t;
			ht[n].val   = ht[t].val;
			ht[n].treelen  = ht[t].treelen;
			if ( (ht[n].xlen != ht[t].xlen) || (ht[n].ylen != ht[t].ylen)  )
			{
				fprintf(stderr,"wrong table %u reference\n",n);
				return (-3);
			}
			while ((line[0] == '#') || (line[0] < ' ') )
			{
				fgets(line,99,fi);
			}
		}
		else if (strcmp(command,".treedata")==0)
		{
			ht[n].ref  = -1;
			ht[n].val = (unsigned char (*)[2])calloc(2*(ht[n].treelen),sizeof(unsigned char));
			if (ht[n].val == NULL)
			{
				fprintf(stderr, "heaperror at table %d\n",n);
				exit (-10);
			}
			for (i=0;(unsigned int)i<ht[n].treelen; i++)
			{
				fscanf(fi,"%x %x",&v0, &v1);
				ht[n].val[i][0]=(unsigned char)v0;
				ht[n].val[i][1]=(unsigned char)v1;
			}
			fgets(line,99,fi); // read the rest of the line
		}
		else
		{
			fprintf(stderr,"huffman decodertable error at table %d\n",n);
		}
	}
	return n;
}







struct node
{
	struct node* child0;
	struct node* child1;
	int value;
};

class Tree
{
	struct node field[7];
public:
	struct node* Root;

	Tree()
	{
		Root = &field[0];

		field[0].child0 = &field[2];
		field[0].child1 = &field[1];

		field[1].child0 = NULL;
		field[1].child1 = NULL;
		field[1].value  = 0;

		field[2].child0 = &field[4];
		field[2].child1 = &field[3];

		field[3].child0 = NULL;
		field[3].child1 = NULL;
		field[3].value  = 1;

		field[4].child0 = &field[6];
		field[4].child1 = &field[5];

		field[5].child0 = NULL;
		field[5].child1 = NULL;
		field[5].value  = 2;

		field[6].child0 = NULL;
		field[6].child1 = NULL;
		field[6].value  = 3;
	}
};


int decode(struct node *root, int* inbits, size_t incount, int*outbits, size_t maxoutcount)
{

	struct node *workingnode=root;
	size_t i, cnt=0;
	for(i=0; i<incount; i++)
	{
		if(inbits[i]==0)
			workingnode = workingnode->child0;
		else
			workingnode = workingnode->child1;

		if(workingnode->child0==NULL)
		{
			if(cnt<maxoutcount)
			{
				outbits[cnt++] = workingnode->value;
			}
			workingnode = root;
		}
	}
	return cnt;
}


bool decode(struct node *root, struct node *&workingnode, int inbit, int&outbit)
{
	if(inbit==0)
		workingnode = workingnode->child0;
	else
		workingnode = workingnode->child1;

	if(workingnode->child0==NULL)
	{
		outbit = workingnode->value;
		workingnode = root;
		return true;
	}
	return false;
}



int testmain(int argc, char *argv[])
{

/*
//huffman test
	{
		
		int arrayin [20] = { 0,0,0,0,0,0,1,0,1,1,0,1,0,1,1,1,1,1,1,1 };
		int arrayout[20] = { 0,0,0,0,0,0,1,0,1,1,0,1,0,1,1,1,1,1,1,1 };

		Tree tree;
		struct node *workingnode=tree.Root;

		int outnum=decode(tree.Root, arrayin, 20, arrayout, 20);

		int i;
		for(i=0; i<outnum; i++)
			printf("%i ", arrayout[i]);
		printf("\n");

		for(i=0; i<20; i++)
		{
			if( decode(tree.Root, workingnode, arrayin[i], outnum) )
				printf("%i ", outnum);
		}
		printf("\n");

	}
*/




/*
//double linked list
	CDLinkNode head, tail, root;

	{
	CDLinkNode a(head,tail);
	CDLinkNode b(head,tail);
	CDLinkNode c(head,tail);
	}

	{
	CDLinkNode d(root);
	CDLinkNode e(root);
	{
	CDLinkNode f(root);
	}
	}

	list<int> intlist;

	iterator iter(intlist);
	iter.next();

	xyz<int> ia;
	xyz<long> ib;

	int& a = ia.getstart();
	int& b = ib.getstart();
*/

//	argtest();

/*	{
		unsigned long i, r1;
		double k, r2;
		unsigned long tick1, tick2;

		tick1=GetTickCount();
		for(i=0; i<10000000; i+=1)
			r1=sqrt1(i);
		tick2=GetTickCount();
		printf("%lu ", tick2-tick1);

		tick1=GetTickCount();
		for(i=0; i<10000000; i+=1)
			r1=sqrt2(i);
		tick2=GetTickCount();
		printf("%lu ", tick2-tick1);

		tick1=GetTickCount();
		for(i=0; i<10000000; i+=1)
			r1=sqrt3(i);
		tick2=GetTickCount();
		printf("%lu ", tick2-tick1);


		tick1=GetTickCount();
		for(k=0; k<10000000; k+=1)
			r2=sqrt(k);
		tick2=GetTickCount();
		printf("%lu \n", tick2-tick1);
	}
*/

/*	{
		size_t k;
		for(k=0; k<40; k++)
			sqrt4test(k);

		return 0;
	}
*/
/*	{
		size_t k,v,res;
		int i;

		for(k=0; k<0x00010000; k++)
		for(i=-10; i<10; i++)
		{
			v = k*k+i;
			printf("\r%u                                   ", v);
			if( (int)v >=0 )
			{
				res = sqrt4(v);
				if( res != (size_t)sqrt(v) )
					printf("\rerr: %i -> %i != %lf (%i,%i,%i)\n", v, res, sqrt(v), (log2(v)/2), (v>>(log2(v)/2)), (1<<(log2(v)/2)), ((v>>(log2(v)/2))+(1<<(log2(v)/2)))/2);
			}
		}
		printf("\n");
		return 0;
	}
*/
/*
	{
		unsigned long tick;
		size_t j,r=0;
		size_t i,k, start, run=1000, width=10000, res=0;

		for(start=0x7FFFFFFF; start; start>>=4)
		{
			printf("starting sqrt with %i, %i ascending numbers, %i runs\n", start, width, run);

			printf("results: shino1 %i, shino2 %i, shino3 %i, shino4 %i, skot %i, std sqrt %lf\n", sqrt1(start), sqrt2(start), sqrt3(start), sqrt4(start), newt_sqrt(start),sqrt(start));

			tick = GetTickCount();
			for(k=0;k<run; k++)
			for(i=start;i<start+width;i++)
				res += sqrt1(i);
			printf("%li ", GetTickCount()-tick);
			printf("sqrt shinomori, init with n/2\n");
			
			tick = GetTickCount();
			for(k=0;k<run; k++)
			for(i=start;i<start+width;i++)
				res += sqrt2(i);
			printf("%li ", GetTickCount()-tick);
			printf("sqrt shinomori, init with n>>log2(n)/2\n");

			tick = GetTickCount();
			for(k=0;k<run; k++)
			for(i=start;i<start+width;i++)
				res += sqrt3(i);
			printf("%li ", GetTickCount()-tick);
			printf("sqrt shinomori, init with 1<<log2(n)/2\n");
			
			tick = GetTickCount();
			for(k=0;k<run; k++)
			for(i=start;i<start+width;i++)
				res += sqrt4(i);
			printf("%li ", GetTickCount()-tick);
			printf("sqrt shinomori, init with (n>>log2(n)/2+1<<log2(n)/2)/2\n");

			tick = GetTickCount();
			for(k=0;k<run; k++)
			for(i=start;i<start+width;i++)
				res += newt_sqrt(i);
			printf("%li ", GetTickCount()-tick);
			printf("sqrt skotlex, init with n/2 + division by zero fix\n");

			tick = GetTickCount();
			for(k=0;k<run; k++)
			for(j=start;j<start+width;j++)
				r += (size_t)sqrt((double)j);
			printf("%li ", GetTickCount()-tick);
			printf("standard double sqrt\n");
		}
		
	}

*/

/*
	{
		BB bb;

		int x = bb.func();
		int y = bb.AA::func();
	}
return 0;
*/
/*
	printf("%i\n", sizeof(testclass));
	testclass* aaa1= new testclass;
	testclass* aaa2= new testclass[2];
	delete aaa1;
	delete[] aaa2;

	vartest();
*/

	return 0;
}