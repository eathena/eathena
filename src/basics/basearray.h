#ifndef __BASEARRAY_H__
#define __BASEARRAY_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basesort.h"


//## TODO: rewrite with the new memory allocation
//## TODO: change names to "vector" and use "array" for two dimentionals
//## TODO: clean virtual functions and check if they could be better templated
//## TODO: combine vector&stack using allocator_w for both and have a seperated fifo using allocator_rw
//## TODO: complete testcases
//## TODO: check if merging base classes back to the allocators is feasible

NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// test function
void test_array(void);


///////////////////////////////////////////////////////////////////////////////
/// either throws or just prints the error message
void vector_error(const char*errmsg);





///////////////////////////////////////////////////////////////////////////////
/// list initialisation as "vector = element,element,...;".
/// only for single assigns, does not work on construction or multiples
template<typename A, typename T>
class ListInit
{
public:
    A& array;

	ListInit(A& a) : array(a)
    { }

    ListInit<A, T> operator,(const T& x)
    {
        array.append(x);
        return *this;
    }
private:
    ListInit();
};






///////////////////////////////////////////////////////////////////////////////
/// vector interface.
template<typename T>
class vectorinterface : public defaultcmp
{
protected:
	vectorinterface<T>()			{}
public:
	virtual ~vectorinterface<T>()	{}

	///////////////////////////////////////////////////////////////////////////
	// 
	virtual size_t size() const=0;
	virtual size_t length() const=0;
	virtual bool is_empty() const=0;

	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates to newsize but leaves cnt as it is
	// usefull prior to large insertions
	virtual bool realloc(size_t newsize=0)=0;

	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1], 
	/// leave new elements uninitialized/default constructed/cuts trailing
	virtual bool resize(size_t cnt)=0;

	///////////////////////////////////////////////////////////////////////////
	/// move elements inside the buffer
	// using element assignments
	virtual bool move(size_t tarpos, size_t srcpos, size_t cnt=1)=0;

	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	virtual bool removeindex(size_t inx)=0;

	///////////////////////////////////////////////////////////////////////////
	/// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)=0;

	///////////////////////////////////////////////////////////////////////////
	//// removes elements from the end
	virtual bool strip(size_t cnt)=0;
	///////////////////////////////////////////////////////////////////////////
	/// removes elements from the start
	virtual bool skip(size_t cnt)=0;

	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	virtual bool clear()=0;

	///////////////////////////////////////////////////////////////////////////
	/// assignment 
	virtual bool assign(const T* e, size_t cnt)=0;
	virtual bool assign(const T& e)=0;
	virtual bool assign(const T& e, size_t cnt)=0;

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)=0;
	virtual bool append(const T& elem)=0;
	virtual bool append(const T& elem, size_t cnt)=0;
	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)=0;
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)=0;

	///////////////////////////////////////////////////////////////////////////
	/// copy the given array to pos, 
	/// overwrites existing elements, 
	/// expands automatically but does not shrink when array is already larger
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)=0;

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)=0;

	///////////////////////////////////////////////////////////////////////////
	/// access to element[inx] 0 and length()-1
	virtual T& first()=0;
	virtual T& last()=0;
	virtual const T& first() const=0;
	virtual const T& last() const=0;

	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx]
	virtual const T& operator()(size_t inx) const =0;
	virtual const T& operator[](size_t inx) const =0;
	virtual const T& operator[](int inx) const =0;
	virtual       T& operator()(size_t inx) =0;
	virtual       T& operator[](size_t inx) =0;
	virtual       T& operator[](int inx) =0;


	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	virtual bool push(const T& elem)=0;
	virtual bool push(const T* elem, size_t cnt)=0;
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T pop()=0;
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)=0;
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T& top() const=0;
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const=0;


	///////////////////////////////////////////////////////////////////////////
	/// some of the stl compatible methods
	void push_back(const T& elem)	{ push(elem); }
	void erase(size_t pos)			{ removeindex(pos); }
};


///////////////////////////////////////////////////////////////////////////////
/// internal storage class
template<typename T, typename E, typename A> 
class vectorbase : public A, public E, public vectorinterface<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	/// only derived can create
	vectorbase<T,E,A>()				{}
public:
	virtual ~vectorbase<T,E,A>()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// 
	virtual size_t size() const		{ return this->A::size(); }
	virtual size_t length() const	{ return this->A::length(); }
	virtual bool is_empty() const	{ return this->size()==0; }

	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates to newsize but leaves cnt as it is
	// usefull prior to large insertions
	virtual bool realloc(size_t newsize=0)
	{
		return ( newsize <= this->size() || this->checkwrite(newsize-this->size()) );
	}

	///////////////////////////////////////////////////////////////////////////
	/// access to element[inx] 0 and length()-1
	virtual T& first()				{ return *const_cast<T*>(this->begin()); }
	virtual T& last()				{ return *const_cast<T*>(this->end()); }
	virtual const T& first() const	{ return *this->begin(); }
	virtual const T& last() const	{ return *this->end(); }

	///////////////////////////////////////////////////////////////////////////
	//
	void debug_print();
};



///////////////////////////////////////////////////////////////////////////////
/// vector.
/// numerates elements 0 ... length()-1
/// implements fifo hehaviour on push/pop (add last/remove first)
/// but not as efficient as the fifo class
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename E=elaborator_ct<T>, typename A=allocator_w_dy<T> >
class vector : public vectorbase<T,E,A>
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// standard constructor / destructor
	vector<T,E,A>()				{}
	virtual ~vector<T,E,A>()	{}

	///////////////////////////////////////////////////////////////////////////
	/// copy/assignment
	vector<T,E,A>(const vector<T,E,A>& v)
	{
		this->assign(v);
	}
	const vector<T,E,A>& operator=(const vector<T,E,A>& v)
	{	
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated copy/assignment
	// microsoft does not understand templated copy/assign together with default copy/assign
	// but gnu needs them both seperated
	// otherwise generates an own default copy/assignment which causes trouble then
	// so could add all constructors and seperate the standard one with
	// #if defined(__GNU__) or #if !defined(_MSC_VER) / #endif
	// another workaround is to have a baseclass to derive the hierarchy from 
	// and have templated copy/assignment refering the baseclass beside standard copy/assignment
	template<class TT, class EE, class AA> vector<T,E,A>(const vectorbase<TT,EE,AA>& v)
	{
		this->assign(v);
	}
	template<class TT, class EE, class AA> const vector<T,E,A>& operator=(const vectorbase<TT,EE,AA>& v)
	{
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// carray constructor
	template<class TT> vector(const TT* elem, size_t sz)
	{	// we are clean and empty here
		this->convert_assign(elem, sz);
	}
	explicit vector(const T& elem)
	{	// we are clean and empty here
		this->convert_assign(elem);
	}

	///////////////////////////////////////////////////////////////////////////
	/// just size constructor (leaving the elements default constructed)
	explicit vector(size_t sz)
	{	// we are clean and empty here
		this->resize( sz );
	}
	///////////////////////////////////////////////////////////////////////////
	/// with variable argument array (use with care)
	vector(size_t sz, const T& t0, ...)
	{	// we are clean and empty here
		if( sz && this->checkwrite( sz ) )
		{
			va_list va;
			va_start(va, t0);
			T* ptr = this->cWpp;
			T* eptr= this->cWpp = ptr+sz;
			*ptr++ = t0;
			while(ptr<eptr)
				*ptr++ = va_arg(va, T);
			va_end(va);
		}
	}

	///////////////////////////////////////////////////////////////////////////
    /// Scalar assignment
    ListInit<vector<T,E,A>, T> operator=(const T& x)
    {
		this->assign(x);
        return ListInit<vector<T,E,A>, T>(*this);
    }


protected:
	///////////////////////////////////////////////////////////////////////////
	/// get start pointer to the array
	virtual const T* raw() const
	{
		const_cast< vector<T,E,A>* >(this)->checkwrite(1);
		return this->ptrRpp();
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1], 
	/// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)
	{
		if( this->ptrRpp()+cnt <= this->cEnd || this->checkwrite( (this->size()<cnt)?cnt:(this->size()-cnt) ) )
		{
			this->cWpp = this->ptrRpp()+cnt;
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// move elements inside the buffer
	// using element assignments
	virtual bool move(size_t tarpos, size_t srcpos, size_t cnt=1)
	{
		return elementmove(this->ptrRpp(), this->size(), tarpos, srcpos, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		if( inx < this->size() )
		{	
			if( this->ptrRpp()+inx+1<this->cWpp )
				this->intern_move( this->ptrRpp()+inx, this->ptrRpp()+inx+1, this->cWpp-this->ptrRpp()-inx-1);
			this->cWpp--;
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		if( cnt && inx < this->size() )
		{	
			if( this->ptrRpp()+inx+cnt<this->cWpp )
				this->intern_move(this->ptrRpp()+inx, this->ptrRpp()+inx+cnt, this->cWpp-this->ptrRpp()-inx-cnt);
			if( this->ptrRpp()+cnt >= this->cWpp )
				this->clear();
			else
				this->cWpp-=cnt;
			return true;
		}
		else
			return false;

	}
	///////////////////////////////////////////////////////////////////////////
	/// removes elements from the end
	virtual bool strip(size_t cnt)
	{
		if(cnt)
		{
			if( this->ptrRpp()+cnt >= this->cWpp )
				this->clear();
			else
				this->cWpp-=cnt;
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// removes elements from the start
	virtual bool skip(size_t cnt)
	{
		return this->removeindex(0, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	virtual bool clear()
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		this->checkwrite(0);
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// assignment 
	virtual bool assign(const T* e, size_t cnt)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->convert_append(e, cnt);
	}
	virtual bool assign(const T& e)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->convert_append(e);
	}
	virtual bool assign(const T& e, size_t cnt)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->convert_append_multiple(e, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated assignment
	template<class TT, class EE, class AA> bool assign(const vectorbase<TT,EE,AA>& array)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->append(array);
	}
	template<typename TT> bool convert_assign(const TT* e, size_t cnt)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return convert_append(e, cnt);
	}
	template<typename TT> bool convert_assign(const TT& e)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->convert_append(e);
	}
	// some compilers cannot decide on overloaded template members
	// so have seperated names instead
	template<typename TT> bool convert_assign_multiple(const T& e, size_t cnt)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return convert_append_multiple(e,cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		return convert_append(elem,cnt);
	}
	virtual bool append(const T& elem)
	{
		return convert_append(elem);
	}
	virtual bool append(const T& elem, size_t cnt)
	{
		return convert_append_multiple(elem,cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated append
	template<typename TT, typename EE, typename AA> bool append(const vectorbase<TT,EE,AA>& array)
	{
		if( !array.size() )
			return true;
		else if( this->cWpp+array.size() <= this->cEnd || this->checkwrite( array.size() ) )
		{	
			typename AA::iterator iter(array);
			for( ; iter; ++iter)
				*this->cWpp++ = *iter;
			return true;
		}
		else
			return false;
	}
	template<typename TT> bool convert_append(const TT* elem, size_t cnt)
	{
		if( !cnt || !elem )
			return true;
		else if( this->cWpp+cnt <= this->cEnd || this->checkwrite( cnt ) )
		{	
			const TT* eptr = elem+cnt;
			while(elem < eptr)
				*this->cWpp++ = *elem++;
			return true;
		}
		else
			return false;
	}
	template<typename TT> bool convert_append(const TT& elem)
	{
		if( this->cWpp < this->cEnd || this->checkwrite(1) )
		{
			*this->cWpp++ = elem;
			return true;
		}
		else
			return false;
	}
	// some compilers cannot decide on overloaded template members
	// so have seperated names instead
	template<typename TT> bool convert_append_multiple(const TT& elem, size_t cnt)
	{
		if( !cnt )
			return true;
		else if( this->cWpp+cnt <= this->cEnd || this->checkwrite(cnt) )
		{
			while(cnt--)
				*this->cWpp++ = elem;
			return true;
		}
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		return convert_insert(elem,cnt,pos);
	}
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		return convert_insert_multiple(elem,cnt,pos);
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated insert
	template<typename TT, typename EE, typename AA> bool insert(const vectorbase<TT,EE,AA>& array, size_t pos)
	{
		if( !array.size() )
			return true;
		else if( pos>this->size() )
			return this->append(array);
		else if( this->cWpp+array.size() <= this->cEnd || checkwrite( array.size() ) )
		{	// make the hole to insert
			this->intern_move(this->ptrRpp()+pos+array.size(), this->ptrRpp()+pos, this->cWpp-this->ptrRpp()-pos);
			// move the end pointer
			this->cWpp+=array.size();
			// fill the hole
			T* ptr = this->ptrRpp()+pos;

			typename AA::iterator iter(array);
			for( ; iter; ++iter)
				*ptr++ = *iter;
			return true;
		}
		else
			return false;
	}
	template <class TT> bool convert_insert(const TT* elem, size_t cnt, size_t pos)
	{
		if( !cnt )
			return true;
		else if( pos>this->size() )
			return this->append(elem,cnt);
		else if( this->cWpp+cnt <= this->cEnd || this->checkwrite(cnt) )
		{	// make the hole to insert
			this->intern_move(this->ptrRpp()+pos+cnt, this->ptrRpp()+pos, this->cWpp-this->ptrRpp()-pos);
			// move the end pointer
			this->cWpp+=cnt;
			// fill the hole
			T* ptr = this->ptrRpp()+pos, *eptr=ptr+cnt;
			while(ptr < eptr)
				*ptr++ = *elem++;
			return true;
		}
		else
			return false;
	}
	// stupid compilers cannot decide between overloaded template versions
	// so need to have seperated names 
	template <class TT> bool convert_insert_multiple(const TT& elem, size_t cnt, size_t pos)
	{
		if( !cnt )
			return true;
		else if( pos>this->size() )
			return this->append(elem,cnt);
		else if( this->cWpp+cnt <= this->cEnd || this->checkwrite(cnt) )
		{	// make the hole to insert
			this->intern_move(this->ptrRpp()+pos+cnt, this->ptrRpp()+pos, this->cWpp-this->ptrRpp()-pos);
			// move the end pointer
			this->cWpp+=cnt;
			// fill the hole
			T* ptr = this->ptrRpp()+pos;
			while(cnt--)
				*ptr++ = elem;
			return true;
		}
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// copy the given array to pos, 
	/// overwrites existing elements, 
	/// expands automatically but does not shrink when array is already larger
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		return convert_copy(elem, cnt, pos);
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given array to pos, 
	// overwrites existing elements, 
	// expands automatically but does not shrink when array is already larger
	template<typename TT, typename EE, typename AA> bool copy(const vectorbase<TT,EE,AA>& array, size_t pos=0)
	{
		if( pos >= this->size() )
		{
			return this->append(array);
		}
		else if( this->size()-pos > array.size() || this->checkwrite( array.size()-this->size()+pos) )
		{	
			T*ptr = this->ptrRpp()+pos;

			typename AA::iterator iter(array);
			for( ; iter; ++iter)
				*ptr++ = *iter;

			// set new write pointer if expanded
			if(ptr>this->cWpp)
				this->cWpp=ptr;
			return true;
		}
		else
			return false;
	}
	template<class TT> bool convert_copy(const TT* elem, size_t cnt, size_t pos=0)
	{
		if( pos >= this->size() )
		{
			return this->append(elem, cnt);
		}
		else if( this->size()-pos > cnt || this->checkwrite( cnt-this->size()+pos) )
		{	
			T* ptr = this->ptrRpp()+pos, *eptr=ptr+cnt;
			
			while( ptr < eptr )
				*ptr++ = *elem++;
			// set new write pointer if expanded
			if(ptr>this->cWpp)
				this->cWpp=ptr;
			return true;
		}
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		return convert_replace(elem, cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with array
	template<class TT, class EE, class AA> bool replace(const vectorbase<TT,EE,AA>& array, size_t pos, size_t poscnt)
	{
		if( pos >= this->size() )
		{	// replace position outside
			// assume append
			return this->append(array);
		}
		else if( pos+poscnt >= this->size() ) 
		{	// replaceing the last elements of the array
			// first resize
			if( this->ptrRpp()+pos <= this->cEnd )
				this->cWpp = this->ptrRpp()+pos;
			// then append
			return this->append(array);
		}
		else if( array.size()<poscnt || this->checkwrite( array.size()-poscnt ) )
		{	// preplacing something in the middle
			// move the trailing elements in place
			this->intern_move(this->ptrRpp()+pos+array.size(), this->ptrRpp()+pos+poscnt, this->cWpp-this->ptrRpp()-pos-poscnt);
			// set new write pointer
			this->cWpp = this->cWpp+array.size()-poscnt;
			// fill the hole with the array elements
			T* ptr = this->ptrRpp()+pos;

			typename AA::iterator iter(array);
			for( ; iter; ++iter)
				*ptr++ = *iter;
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	template<class TT> bool convert_replace(const TT* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		if( pos >= this->size() )
		{	// replace position outside
			// assume append
			return this->append(elem, cnt);
		}
		else if( pos+poscnt >= this->size() ) 
		{	// replaceing the last elements of the array
			// first resize
			if( this->ptrRpp()+pos <= this->cEnd  )
				this->cWpp = this->ptrRpp()+pos;
			// then append
			return this->append(elem, cnt);
		}
		else if( cnt<poscnt || this->checkwrite( cnt-poscnt ) )
		{	// preplacing something in the middle
			// move the trailing elements in place
			this->intern_move(this->ptrRpp()+pos+cnt, this->ptrRpp()+pos+poscnt, this->cWpp-this->ptrRpp()-pos-poscnt);
			// set new write pointer
			this->cWpp = this->cWpp+cnt-poscnt;
			// fill the hole with the array elements
			T* ptr = this->ptrRpp()+pos, *eptr=ptr+cnt;
			while( ptr<eptr )
				*ptr++ = *elem++;
			return true;
		}
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx]
	virtual const T& operator () (size_t inx) const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->ptrRpp()+inx >= this->cWpp )
		{
			vector_error("vector: access out of bound");
#ifndef CHECK_EXCEPTIONS
			// provide some fallback in case of out of bound
			static T dummy;
			return dummy;
#endif
		}
#endif
		return this->ptrRpp()[inx];
	}
	virtual const T& operator[](size_t inx) const
	{	
		return this->operator()((size_t)inx);
	}
	virtual const T& operator[](int inx) const
	{	
		return this->operator()((size_t)inx);
	}
	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx] / writable
	virtual T& operator () (size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->ptrRpp()+inx >= this->cWpp )
		{
			vector_error("vector: access out of bound");
#ifndef CHECK_EXCEPTIONS
			// provide some fallback in case of out of bound
			static T dummy;
			return dummy;
#endif
		}
#endif
		return this->ptrRpp()[inx];
	}
	virtual T& operator[](size_t inx)
	{	
		return this->operator()((size_t)inx);
	}
	virtual T& operator[](int inx)
	{	
		return this->operator()((size_t)inx);
	}


	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	/// implement fifo behaviour (push to the end and pop from front)
	virtual bool push(const T& elem)			{ return convert_push(elem); }
	virtual bool push(const T* elem, size_t cnt){ return convert_push(elem,cnt); }

	template<class TT, class EE, class AA> bool push(const vectorbase<TT,EE,AA>& array){ return append(array); }
	template<class TT> bool convert_push(const TT& elem)			{ return append(elem); }
	template<class TT> bool convert_push(const TT* elem, size_t cnt){ return append(elem,cnt); }
	
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T pop()
	{
		if( this->ptrRpp()>=this->cWpp )
		{
			vector_error("vector underflow");
			return (this->ptrRpp())?(*this->ptrRpp()):T();
		}
		else
		{
			T tmp = (*(this->ptrRpp()));
			this->removeindex(0);
			return tmp;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		// check for access to outside memory
		if( this->ptrRpp()>=this->cWpp )
		{
			return false;
		}
		else
		{
			elem = (*(this->ptrRpp()));
			this->removeindex(0);
			return true;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		if( this->ptrRpp()>=this->cWpp )
		{
			vector_error("vector underflow");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
#endif
		return (*(this->ptrRpp()));
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		// check for access to outside memory
		if( this->ptrRpp()>=this->cWpp )
		{
			return false;
		}
		else
		{
			elem = (*(this->ptrRpp()));
			return true;
		}
	}

};



///////////////////////////////////////////////////////////////////////////////
/// stack.
/// covers stack push/pop behaviour
///////////////////////////////////////////////////////////////////////////////
template <class T> class stack : public vector<T>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor / destructor
	stack<T>()				{}
	virtual ~stack<T>()		{}

	///////////////////////////////////////////////////////////////////////////
	// copy/assignment
	stack<T>(const stack<T>& v)
	{
		this->assign(v);
	}
	const stack<T>& operator=(const stack<T>& v)
	{	
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated copy/assignment
	//#if defined(__GNU__)
	//#if !defined(_MSC_VER)
	// microsoft does not understand templated copy/assign together with default copy/assign
	// but gnu needs them both seperated
	// otherwise generates an own default copy/assignment which causes trouble then
	//#endif
	// another workaround is to have a baseclass to derive the hierarchy from 
	// and have templated copy/assignment refering the baseclass beside standard copy/assignment
	template<class TT, class EE, class AA> stack<T>(const vectorbase<TT,EE,AA>& v)
	{
		this->convert_assign(v);
	}
	template<class TT, class EE, class AA> const stack<T>& operator=(const vectorbase<TT,EE,AA>& v)
	{
		this->convert_assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// carray constructor
	template<class TT> stack(const TT* elem, size_t sz)
	{	// we are clean and empty here
		this->convert_assign(elem, sz);
	}
	explicit stack(const T& elem)
	{	// we are clean and empty here
		this->convert_assign(elem);
	}
	///////////////////////////////////////////////////////////////////////////
	/// just size constructor (leaving the elements default constructed)
	explicit stack(size_t sz)
	{	// we are clean and empty here
		this->resize( sz );
	}
	///////////////////////////////////////////////////////////////////////////
	/// with variable argument array (use with care)
	stack(size_t sz, const T& t0, ...)
	{	// we are clean and empty here
		if( this->checkwrite( sz ) )
		{
			va_list va;
			va_start(va, t0);
			T* ptr = this->cWpp;
			T* eptr= this->cWpp = ptr+sz;
			*ptr++ = t0;
			while(ptr<eptr)
				*ptr++ = va_arg(va, T);
			va_end(va);
		}
	}

	///////////////////////////////////////////////////////////////////////////
    /// Scalar assignment
    ListInit<stack<T>, T> operator=(const T& x)
    {
		this->assign(x);
        return ListInit<stack<T>, T>(*this);
    }


	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	/// implement stack behaviour (push to the end and pop from the end)
	/// inherit push from vectorbase
 

	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T pop()
	{
		if( this->ptrRpp()>=this->cWpp )
		{
			vector_error("vector underflow");
			return (this->ptrRpp())?(*this->ptrRpp()):T();
		}
		return (*(--this->cWpp));
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		// check for access to outside memory
		if( this->ptrRpp()<this->cWpp )
		{
			elem = (*(--this->cWpp));
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T& top() const
	{
		if( this->ptrRpp()>=this->cWpp )
		{
			vector_error("vector underflow");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
		return (*(this->cWpp-1));
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		// check for access to outside memory
		if( this->ptrRpp()<this->cWpp )
		{
			elem = (*(this->cWpp-1));
			return true;
		}
		else
			return false;
	}
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// fifo.
/// does fifo push/pop behaviour
/// and implements moving readpointer to minimize data movements
///////////////////////////////////////////////////////////////////////////////
template <class T> class fifo : public vector<T, elaborator_ct<T>, allocator_rw_dy<T> >
{
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor / destructor
	fifo()			{}
	virtual ~fifo()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy/assignment
	///////////////////////////////////////////////////////////////////////////
	fifo(const fifo<T>& v)
	{
		this->assign(v);
	}
	const fifo<T>& operator=(const fifo<T>& v)
	{	
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated baseclasse copy/assignment
	template<class TT, class EE, class AA> fifo<T>(const vectorbase<TT,EE,AA>& v)
	{
		this->assign(v);
	}
	template<class TT, class EE, class AA> const fifo<T>& operator=(const vectorbase<TT, EE, AA>& v)
	{
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// carray constructor
	template<class TT> fifo(const TT* elem, size_t sz)
	{	
		this->convert_assign(elem, sz);
	}
	explicit fifo(const T& elem)
	{	
		this->convert_assign(elem);
	}

	///////////////////////////////////////////////////////////////////////////
	/// just size constructor (leaving the elements default constructed)
	explicit fifo(size_t sz)
	{	// we are clean and empty here
		this->resize( sz );
	}
	///////////////////////////////////////////////////////////////////////////
	/// with variable argument array (use with care)
	fifo(size_t sz, const T& t0, ...)
	{	// we are clean and empty here
		if( this->checkwrite( sz ) )
		{
			va_list va;
			va_start(va, t0);
			T* ptr = this->cWpp;
			T* eptr= this->cWpp = ptr+sz;
			*ptr++ = t0;
			while(ptr<eptr)
				*ptr++ = va_arg(va, T);
			va_end(va);
		}
	}

	///////////////////////////////////////////////////////////////////////////
    /// Scalar assignment
    ListInit<fifo<T>, T> operator=(const T& x)
    {
		this->assign(x);
        return ListInit<fifo<T>, T>(*this);
    }

public:
	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx]
	// with 0 beeing the first element in the fifo
	virtual const T& operator () (size_t inx) const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cRpp+inx >= this->cWpp )
		{
			vector_error("vector out of bound");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
#endif
		return this->cRpp[inx];
	}
	virtual const T& operator[](size_t inx) const
	{	
		return this->operator()((size_t)inx);
	}
	virtual const T& operator[](int inx) const
	{	
		return this->operator()((size_t)inx);
	}
	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx] / writable
	virtual T& operator () (size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cRpp+inx >= this->cWpp )
		{
			vector_error("vector out of bound");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
#endif
		return this->cRpp[inx];
	}
	virtual T& operator[](size_t inx)
	{	
		return this->operator()((size_t)inx);
	}
	virtual T& operator[](int inx)
	{	
		return this->operator()((size_t)inx);
	}

	///////////////////////////////////////////////////////////////////////////
	/// implement fifo behaviour (push at the end and pop from the start)
	/// inherit push from vector

	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T pop()
	{
		if( this->cRpp>=this->cWpp )
		{
			vector_error("vector underflow");
			return (this->cRpp)?(*this->cRpp):T();
		}
		return *(this->cRpp++);
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		// check for access to outside memory
		if( this->cRpp<this->cWpp )
		{
			elem = *(this->cRpp++);
			return true;
		}
		else
			return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T& top() const
	{
		if( this->cRpp>=this->cWpp )
		{
			vector_error("vector underflow");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
		return *(this->cRpp);
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		// check for access to outside memory
		if( this->cRpp<this->cWpp )
		{
			elem = *(this->cRpp);
			return true;
		}
		else
			return false;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// sorted array.
template <typename T, typename E=elaborator_ct<T>, typename A=allocator_w_dy<T> >
class slist : public vector<T,E,A>
{
/*
public:
	class assignee
	{
		friend class slist<T,E,A>;
		slist<T,E,A>& sl;
		size_t inx;
		assignee(slist<T,E,A>& s, size_t i) : sl(s), inx(i)	{}

		void assign(const T& val)
		{
			sl.removeindex(inx);
			sl.append(val);
		}
	public:
		assignee(const assignee& e);	// ignore copy
		const assignee& operator=(const assignee& e)
		{
			this->assign( e.sl.ptrRpp()[inx] );
			return *this;
		}
		const T& operator=(const T& t)
		{
			this->assign(t);
			return *this;
		}
		operator const T&()	{ return sl.ptrRpp()[inx]; }
	};
	friend class assignee;
*/

public:
    struct _config
    {
        unsigned ownobjects : 1;	///< array is responsible for destroying				// necessary?
        unsigned sorted     : 1;	///< sorted array										// necessary?
        unsigned ascending  : 1;	///< sort order										// necessary? could be always ascending
        unsigned duplicates : 1;	///< sorted: allows duplicate keys					// ok
        unsigned casesens   : 1;	///< sorted: string comparison is case sensitive		// ??
        unsigned _reserved  :27;

		_config(bool a=true, bool d=false) : 
			ownobjects(0),
			sorted(1),
			ascending(a),
			duplicates(d),
			casesens(0)
		{}
    } config;

	int (*compare)(T const &, T const &);

	static int defaultcompare(T const &a, T const &b)
	{
		if(a==b)
			return 0;
		else if(a < b)
			return -1;
		else
			return +1;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor / destructor
	slist<T,E,A>(bool a=true, bool d=false) : config(a,d), compare(defaultcompare)
	{ }
	virtual ~slist<T,E,A>() {}

	///////////////////////////////////////////////////////////////////////////
	// copy/assignment
	slist<T,E,A>(const slist<T,E,A>& v) : config(v.config), compare(v.compare)
	{
		this->assign(v);
	}
	const slist<T,E,A>& operator=(const slist<T,E,A>& v)
	{	
		this->config = v.config;
		this->compare= v.compare;
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated copy/assignment
	//#if defined(__GNU__)
	//#if !defined(_MSC_VER)
	// microsoft does not understand templated copy/assign together with default copy/assign
	// but gnu needs them both seperated
	// otherwise generates an own default copy/assignment which causes trouble then
	//#endif
	// another workaround is to have a baseclass to derive the hierarchy from 
	// and have templated copy/assignment refering the baseclass beside standard copy/assignment
	template<class TT, class EE, class AA> slist<T,E,A>(const vectorbase<TT,EE,AA>& v)
	{
		this->assign(v);
	}
	template<class TT, class EE, class AA> const slist<T,E,A>& operator=(const vectorbase<TT,EE,AA>& v)
	{
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// carray constructor
	template<class TT> slist(const TT* elem, size_t sz) : config(true,false), compare(defaultcompare)
	{	// we are clean and empty here
		this->convert_append(elem, sz);
	}
	explicit slist(const T& elem) : config(true,false), compare(defaultcompare)
	{	// we are clean and empty here
		this->convert_append(elem);
	}

	///////////////////////////////////////////////////////////////////////////
	/// just size constructor (leaving the elements default constructed)
	explicit slist(size_t sz, const T& elem=T()) : config(true,false), compare(defaultcompare)
	{	// we are clean and empty here
		this->convert_append(elem, sz);
	}
	///////////////////////////////////////////////////////////////////////////
	/// with variable argument array (use with care)
	slist(size_t sz, const T& t0, ...) : config(true,false), compare(defaultcompare)
	{	// we are clean and empty here
		if( this->checkwrite( sz ) )
		{
			va_list va;
			va_start(va, t0);
			this->assign(t0);
			while(sz--)
				this->vector<T,E,A>::assign( va_arg(va, T) );
			va_end(va);
		}
	}

	///////////////////////////////////////////////////////////////////////////
    /// Scalar assignment
    ListInit<slist<T,E,A>, T> operator=(const T& x)
    {
		this->vector<T,E,A>::assign(x);
        return ListInit<slist<T,E,A>, T>(*this);
    }

public:
	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1], 
	/// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)
	{
		if( cnt < this->size() )
		{
			return vector<T>::resize(cnt);
		}
		else if( this->config.duplicates )
		{	
			return this->convert_append_multiple( T(), cnt-this->size());
		}
		else
			return false;
	}
private:
	///////////////////////////////////////////////////////////////////////////
	/// move elements inside the buffer
	virtual bool move(size_t tarpos, size_t srcpos, size_t cnt=1)
	{	// not allowed
		return false;
	}
public:	

	///////////////////////////////////////////////////////////////////////////
	/// assignment 
	virtual bool assign(const T* e, size_t cnt)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->convert_append(e, cnt);
	}
	virtual bool assign(const T& e)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->convert_append(e);
	}
	virtual bool assign(const T& e, size_t cnt)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->convert_append_multiple(e, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated assignment
	template<class TT, class EE, class AA> bool assign(const vectorbase<TT,EE,AA>& array)
	{
		this->cWpp=this->ptrRpp()=this->cBuf;
		return this->append(array);
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		return convert_append(elem,cnt);
	}
	virtual bool append(const T& elem)
	{
		return convert_append(elem);
	}
	virtual bool append(const T& elem, size_t cnt)
	{
		return convert_append_multiple(elem,cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated append
	template<typename TT, typename EE, typename AA> bool append(const vectorbase<TT,EE,AA>& array)
	{
		if( this->cWpp+array.size() <= this->cEnd || this->checkwrite( array.size() ) )
		{	
			typename AA::iterator iter(array);
			size_t pos;
			for( ; iter; ++iter )
			{
				if( !BinarySearchC<T, const T*, T>( *iter, this->begin(), this->size(), 0, pos, this->compare, this->config.ascending) || this->config.duplicates )
				{
					T* xptr = this->ptrRpp()+pos, *xeptr=xptr+1;
					this->intern_move(xeptr, xptr, this->cWpp-this->ptrRpp()-pos);
					*xptr = *iter;
					this->cWpp++;
				}
			}
			return true;
		}
		else
			return false;
	}
	template<typename TT> bool convert_append(const TT* elem, size_t cnt)
	{
		if( this->cWpp+cnt <= this->cEnd || this->checkwrite( cnt ) )
		{	
			size_t pos;
			const TT* ptr = elem, *eptr = ptr+cnt;
			while( ptr<eptr)
			{
				if( !BinarySearchC<T, const T*, T>( *ptr, this->begin(), this->size(), 0, pos, this->compare, this->config.ascending) || this->config.duplicates )
				{
					T* xptr = this->ptrRpp()+pos, *xeptr=xptr+1;
					this->intern_move(xeptr, xptr, this->cWpp-this->ptrRpp()-pos);
					*xptr = *ptr;
					this->cWpp++;
				}
				ptr++;
			}
			return true;
		}
		else
			return false;
	}
	template<typename TT> bool convert_append(const TT& elem)
	{
		if( this->cWpp < this->cEnd || this->checkwrite(1) )
		{	
			size_t pos;
			if( !BinarySearchC<T, const T*, T>( elem, this->begin(), this->size(), 0, pos, this->compare, this->config.ascending) || this->config.duplicates )
			{	
				T* xptr = this->ptrRpp()+pos, *xeptr=xptr+1;
				this->intern_move(xeptr, xptr, this->cWpp-this->ptrRpp()-pos);
				*xptr = elem;
				this->cWpp++;
			}
			return true;
		}
		else
			return false;
	}
	// some compilers cannot decide on overloaded template members
	// so have seperated names instead
	template<typename TT> bool convert_append_multiple(const TT& elem, size_t cnt)
	{
		if( this->cWpp+cnt <= this->cEnd || this->checkwrite(cnt) )
		{	
			size_t pos;
			if( !BinarySearchC<T, const T*, T>( elem, this->begin(), this->size(), 0, pos, this->compare, this->config.ascending) || this->config.duplicates )
			{
				if( !this->config.duplicates ) cnt = 1;
				T* xptr = this->ptrRpp()+pos, *xeptr=xptr+cnt;
				this->intern_move(xeptr, xptr, this->cWpp-this->ptrRpp()-pos);
				while(xptr<xeptr)
					*xptr++ = elem;
				this->cWpp+=cnt;
			}
			return true;
		}
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		return this->convert_append(elem,cnt);
	}
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		return this->convert_append_multiple(elem,cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// templated insert
	template<typename TT, typename EE, typename AA> bool insert(const vectorbase<TT,EE,AA>& array, size_t pos)
	{
		return this->convert_append(array);
	}
	template <class TT> bool convert_insert(const TT* elem, size_t cnt, size_t pos)
	{
		return this->convert_append(elem, cnt);
	}
	// stupid compilers cannot decide between overloaded template versions
	// so need to have seperated names 
	template <class TT> bool convert_insert_multiple(const TT& elem, size_t cnt, size_t pos)
	{
		return this->convert_append_multiple(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// copy the given array to pos, 
	/// overwrites existing elements, 
	/// expands automatically but does not shrink when array is already larger
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		return this->convert_append(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given array to pos, 
	// overwrites existing elements, 
	// expands automatically but does not shrink when array is already larger
	template<typename TT, typename EE, typename AA> bool copy(const vectorbase<TT,EE,AA>& array, size_t pos=0)
	{
		return this->convert_append(array);
	}
	template<class TT> bool convert_copy(const TT* elem, size_t cnt, size_t pos=0)
	{
		return this->convert_append(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->convert_append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with array
	template<class TT, class EE, class AA> bool replace(const vectorbase<TT,EE,AA>& array, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->convert_append(array);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	template<class TT> bool convert_replace(const TT* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->convert_append(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx]
	// only read access to element(inx)
	virtual const T& operator () (size_t inx) const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->ptrRpp()+inx >= this->cWpp )
		{
			vector_error("vector out of bound");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
#endif
		return this->ptrRpp()[inx];
	}
	virtual const T& operator[](size_t inx) const
	{	
		return this->operator()((size_t)inx);
	}
	virtual const T& operator[](int inx) const
	{	
		return this->operator()((size_t)inx);
	}
/*	///////////////////////////////////////////////////////////////////////////
	// access to elements(inx) [inx] / writable
	// return assignee object on non-const access
	assignee operator()(size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->ptrRpp()+inx >= this->cWpp )
		{
			vector_error("vector out of bound");
#ifndef CHECK_EXCEPTIONS
			// return some out of bound element
			return assignee(*this, this->size());
#endif
		}
#endif
		//return this->ptrRpp()[inx];
		return assignee(*this, inx);
	}
	assignee<T,E,A> operator[](size_t inx)
	{	
		return this->operator()((size_t)inx);
	}
	assignee<T,E,A> operator[](int inx)
	{	
		return this->operator()((size_t)inx);
	}
*/
	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx] / writable
	/// dangerous since able to destroy sort order by altering the objects
	virtual T& operator () (size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->ptrRpp()+inx >= this->cWpp )
		{
			vector_error("vector out of bound");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
#endif
		return this->ptrRpp()[inx];
	}
	virtual T& operator[](size_t inx)
	{	
		return this->operator()((size_t)inx);
	}
	virtual T& operator[](int inx)
	{	
		return this->operator()((size_t)inx);
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	/// ignore orders, but insert sorted
	virtual bool push(const T& elem)			{ return convert_append(elem); }
	virtual bool push(const T* elem, size_t cnt){ return convert_append(elem,cnt); }

	template<class TT, class EE, class AA> bool push(const vectorbase<TT,EE,AA>& array){ return append(array); }
	template<class TT> bool convert_push(const TT& elem)			{ return convert_append(elem); }
	template<class TT> bool convert_push(const TT* elem, size_t cnt){ return convert_append(elem,cnt); }


	template<class TT> bool find(const TT& elem, size_t start, size_t& pos) const
	{
		return BinarySearchC<T,const T*,T>(elem, this->begin(), this->size(), start, pos, this->compare, this->config.ascending);
	}

	void sort()
	{
		if(this->size()>1)
			QuickSortClassicC<T>( const_cast<T*>(this->begin()), 0, this->size()-1, this->compare); 
	}

};





///////////////////////////////////////////////////////////////////////////////
/// vector of pointers.
/// uses partial specialisation of vector template storing all pointer types as void*
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename E=elaborator_ct<void*>, typename A=allocator_w_dy<void*> >
class ptrvector
{
public:
	typedef T*					value_type;
	typedef const value_type*	const_iterator;
	typedef value_type*			simple_iterator;

	///////////////////////////////////////////////////////////////////////////
	/// iterator.
	/// just wrapps internal iterator with casts 
	/// so it is completely removed by the compiler
	class iterator
	{
		typename A::iterator iter;
	public:
		iterator(const ptrvector& a)
			: iter(a.cVect)
		{}
		iterator(const ptrvector& a, const T** p)
			: iter(a.cVect, (void**)p)
		{}
		~iterator()	{}
		/// can use default copy//assignment
		const iterator& operator=(const T** p)
		{
			this->iter=(void**)p;
			return *this;
		}
		const iterator& operator=(const ptrvector& a)
		{
			this->iter = a.cVect;
			return *this;
		}
		bool isvalid() const	{ return this->iter.isvalid(); }
		operator bool() const	{ return this->iter.isvalid(); }
		T* operator()()			{ return *((T**)this->iter.operator()()); }
		T& operator*()			{ return *((T*&)this->iter.operator*() ); }
		T* operator->()			{ return *((T**)this->iter.operator->()); }

		T** operator++()			
		{	// preincrement
			return ((T**)(++this->iter));
		}
		T** operator++(int)
		{	// postincrement
			return ((T**)(this->iter++));
		}
		T** operator--()
		{	// predecrement
			return ((T**)(--this->iter));
		}
		T** operator--(int)
		{	// postdecrement
			return ((T**)(this->iter--));
		}
		bool operator==(const T**p) const	{ return this->iter==(void**)p; }
		bool operator!=(const T**p) const	{ return this->iter!=(void**)p; }
		bool operator>=(const T**p) const	{ return this->iter>=(void**)p; }
		bool operator> (const T**p) const	{ return this->iter> (void**)p; }
		bool operator<=(const T**p) const	{ return this->iter<=(void**)p; }
		bool operator< (const T**p) const	{ return this->iter< (void**)p; }

		bool operator==(const iterator&i) const	{ return this->iter==i.iter; }
		bool operator!=(const iterator&i) const	{ return this->iter!=i.iter; }
		bool operator>=(const iterator&i) const	{ return this->iter>=i.iter; }
		bool operator> (const iterator&i) const	{ return this->iter> i.iter; }
		bool operator<=(const iterator&i) const	{ return this->iter<=i.iter; }
		bool operator< (const iterator&i) const	{ return this->iter< i.iter; }
	};
	friend class iterator;

	virtual const T** begin() const		{ return (const T**)this->cVect.begin(); }
	virtual const T** end() const		{ return (const T**)this->cVect.end(); }
	virtual const T** final() const		{ return (const T**)this->cVect.final(); }

	virtual       T** begin()			{ return (      T**)this->cVect.begin(); }
	virtual       T** end()				{ return (      T**)this->cVect.end(); }
	virtual       T** final()			{ return (      T**)this->cVect.final(); }
protected:
	vector<void*,E,A>	cVect;
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor / destructor
	ptrvector<T,E,A>()				{}
	virtual ~ptrvector<T,E,A>()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy/assignment
#if !defined(NO_TEMPLATED_COPY_ASSIGN)
	ptrvector<T,E,A>(const ptrvector<T,E,A>& v)
	{
		this->assign(v);
	}
	const ptrvector<T,E,A>& operator=(const ptrvector<T,E,A>& v)
	{	
		this->assign(v);
		return *this;
	}
#endif
	template<class EE, class AA> ptrvector<T,E,A>(const ptrvector<T,EE,AA>& v)
	{
		this->assign(v);
	}
	template<class EE, class AA> const ptrvector<T,E,A>& operator=(const ptrvector<T,EE,AA>& v)
	{
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// carray constructor
	ptrvector(T*const* elem, size_t sz)
	{	// we are clean and empty here
		this->assign(elem, sz);
	}
	explicit ptrvector(T*const& elem)
	{	// we are clean and empty here
		this->assign(elem);
	}

	///////////////////////////////////////////////////////////////////////////
	/// just size constructor (leaving the elements default constructed)
	explicit ptrvector(size_t sz)
	{	// we are clean and empty here
		this->resize( sz );
	}

	///////////////////////////////////////////////////////////////////////////
    /// Scalar assignment
    ListInit<ptrvector<T,E,A>, T> operator=(const T& x)
    {
		this->assign(x);
        return ListInit<ptrvector<T,E,A>, T>(*this);
    }

protected:
	virtual T*const* raw() const
	{
		return (T*const*)this->cVect.begin();
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// 
	virtual size_t size() const		{ return this->cVect.size(); }
	virtual size_t length() const	{ return this->cVect.length(); }
	virtual bool is_empty() const	{ return this->cVect.is_empty(); }

	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1], 
	/// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)
	{
		size_t sz =this->cVect.size(); 
		bool ret = this->cVect.resize(cnt);
		if(ret)
			memset(const_cast<void**>(this->cVect.final()-(cnt-sz)), 0, (cnt-sz)*sizeof(void*) );
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// move elements inside the buffer
	// using element assignments
	virtual bool move(size_t tarpos, size_t srcpos, size_t cnt=1)
	{
		return this->cVect.move(tarpos, srcpos, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		return this->cVect.removeindex(inx);
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		return this->cVect.removeindex(inx, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// removes elements from the end
	virtual bool strip(size_t cnt)
	{
		return this->cVect.strip(cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	virtual bool clear()
	{
		return this->cVect.clear();
	}

	///////////////////////////////////////////////////////////////////////////
	/// assignment 
	virtual bool assign(T*const* e, size_t cnt)
	{
		return this->cVect.assign( (void*const*)e,cnt);
	}
	virtual bool assign(T*const& e)
	{
		return this->cVect.assign( (void*const&)e );
	}
	virtual bool assign(T*const& e, size_t cnt)
	{
		return this->cVect.assign( (void*const&)e,cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated assignment
	template<class EE, class AA> bool assign(const ptrvector<T,EE,AA>& array)
	{
		return this->cVect.assign(array.cVect);
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(T*const* elem, size_t cnt)
	{
		return this->cVect.append( (void*const*)elem,cnt);
	}
	virtual bool append(T*const& e)
	{
		return this->cVect.append( (void*const&)e );
	}
	virtual bool append(T*const& e, size_t cnt)
	{
		return this->cVect.append( (void*const&)e,cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated append
	template<typename EE, typename AA> bool append(const ptrvector<T,EE,AA>& array)
	{
		return this->cVect.append(array.cVect);
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos
	virtual bool insert(T*const* elem, size_t cnt, size_t pos=~0)
	{
		return this->cVect.insert( (void*const*)elem, cnt, pos);
	}
	virtual bool insert(T*const& elem, size_t cnt=1, size_t pos=~0)
	{
		return this->cVect.insert( (void*const&)elem, cnt, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated insert
	template<typename EE, typename AA> bool insert(const ptrvector<T,EE,AA>& array, size_t pos)
	{
		return this->cVect.insert(array.cVect, pos);
	}

	///////////////////////////////////////////////////////////////////////////
	/// copy the given array to pos, 
	/// overwrites existing elements, 
	/// expands automatically but does not shrink when array is already larger
	virtual bool copy(T*const* elem, size_t cnt, size_t pos=0)
	{
		return this->cVect.copy( (void*const*)elem, cnt, pos);
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given array to pos, 
	// overwrites existing elements, 
	// expands automatically but does not shrink when array is already larger
	template<typename EE, typename AA> bool copy(const ptrvector<T,EE,AA>& array, size_t pos=0)
	{
		return this->cVect.copy(array.cVect, pos);
	}

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(T*const* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		return this->cVect.replace((void*const*)elem, cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with array
	template<class EE, class AA> bool replace(const ptrvector<T,EE,AA>& array, size_t pos, size_t poscnt)
	{
		return this->cVect.replace(array.cVect, pos, poscnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx]
	virtual T*const& operator () (size_t inx) const
	{
		return (T*const&)this->cVect.operator()(inx);
	}
	virtual T*const& operator[](size_t inx) const
	{	
		return (T*const&)this->cVect.operator[](inx);
	}
	virtual T*const& operator[](int inx) const
	{	
		return (T*const&)this->cVect.operator[](inx);
	}
	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx] / writable
	virtual T*& operator () (size_t inx)
	{
		return (T*&)this->cVect.operator()(inx);
	}
	virtual T*& operator[](size_t inx)
	{	
		return (T*&)this->cVect.operator[](inx);
	}
	virtual T*& operator[](int inx)
	{	
		return (T*&)this->cVect.operator[](inx);
	}
	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	/// implement fifo behaviour (push to the end and pop from front)
	virtual bool push( T*const& elem)				{ return this->cVect.push((void*const&)elem); }
	virtual bool push( T*const* elem, size_t cnt)	{ return this->cVect.push((void*const*)elem, cnt); }

	template<class EE, class AA> bool push(const ptrvector<T,EE,AA>& array){ return this->cVect.push(array.cVect); }
	
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T* pop()
	{
		return (T*)this->cVect.pop();
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T*& elem)
	{
		return this->cVect.pop( (void*&)elem );
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T*& top() const
	{
		return (T*&)this->cVect.top();
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T*& elem) const
	{
		return this->cVect.top( (void*&)elem );
	}
};

///////////////////////////////////////////////////////////////////////////////
/// slist of pointers.
/// uses partial specialisation of vector template storing all pointer types as void*
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename E=elaborator_ct<void*>, typename A=allocator_w_dy<void*> >
class ptrslist : public ptrvector<T,E,A>
{
protected:
    struct _config
    {
        unsigned ownobjects : 1;	///< array is responsible for destroying				// necessary?
        unsigned sorted     : 1;	///< sorted array										// necessary?
        unsigned ascending  : 1;	///< sort order										// necessary? could be always ascending
        unsigned duplicates : 1;	///< sorted: allows duplicate keys					// ok
        unsigned casesens   : 1;	///< sorted: string comparison is case sensitive		// ??
        unsigned _reserved  :27;

		_config(bool a=true, bool d=false) : 
			ownobjects(0),
			sorted(1),
			ascending(a),
			duplicates(d),
			casesens(0)
		{}
    } config;
public:
	///////////////////////////////////////////////////////////////////////////
	// standard constructor / destructor
	ptrslist<T,E,A>()				{}
	virtual ~ptrslist<T,E,A>()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy/assignment
	ptrslist<T,E,A>(const ptrslist<T,E,A>& v)
	{
		this->assign(v);
	}
	const ptrslist<T,E,A>& operator=(const ptrslist<T,E,A>& v)
	{	
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated copy/assignment
	// microsoft does not understand templated copy/assign together with default copy/assign
	// but gnu needs them both seperated
	// otherwise generates an own default copy/assignment which causes trouble then
	// so could add all constructors and seperate the standard one with
	// #if defined(__GNU__) or #if !defined(_MSC_VER) / #endif
	// another workaround is to have a baseclass to derive the hierarchy from 
	// and have templated copy/assignment refering the baseclass beside standard copy/assignment
	template<class EE, class AA> ptrslist<T,E,A>(const ptrvector<T,EE,AA>& v)
	{
		this->assign(v);
	}
	template<class EE, class AA> const ptrslist<T,E,A>& operator=(const ptrvector<T,EE,AA>& v)
	{
		this->assign(v);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// carray constructor
	ptrslist(T*const* elem, size_t sz)
	{	// we are clean and empty here
		this->assign(elem, sz);
	}
	explicit ptrslist(T*const& elem)
	{	// we are clean and empty here
		this->assign(elem);
	}

	///////////////////////////////////////////////////////////////////////////
	/// just size constructor (leaving the elements default constructed)
	explicit ptrslist(size_t sz)
	{	// we are clean and empty here
		this->resize( sz );
	}

	///////////////////////////////////////////////////////////////////////////
    /// Scalar assignment
    ListInit<ptrslist<T,E,A>, T> operator=(const T& x)
    {
		this->assign(x);
        return ListInit<ptrslist<T,E,A>, T>(*this);
    }

public:
	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1], 
	/// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)
	{
		size_t sz =this->cVect.size(); 
		bool ret = this->cVect.resize(cnt);
		if(ret && cnt>sz)
			memset( const_cast<void**>(this->cVect.final()-(cnt-sz)), 0, (cnt-sz)*sizeof(void*) );
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// move elements inside the buffer
	// using element assignments
private:
	virtual bool move(size_t tarpos, size_t srcpos, size_t cnt=1)
	{
		return false;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// assignment 
	virtual bool assign(T*const* e, size_t cnt)
	{
		this->clear();
		return this->append( e,cnt);
	}
	virtual bool assign(T*const& e)
	{
		this->clear();
		return this->append( e );
	}
	virtual bool assign(T*const& e, size_t cnt)
	{
		this->clear();
		return this->append( e,cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated assignment
	template<class EE, class AA> bool assign(const ptrvector<T,EE,AA>& array)
	{
		this->clear();
		return this->append(array.cVect);
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(T*const* elem, size_t cnt)
	{
		bool ret =true;
		size_t pos;		
		T*const* end = elem+cnt;
		while( elem<end)
		{
			if( !this->find( *elem, 0, pos) )
			{
				ret &= this->cVect.insert( ((void*)(*elem)), 1, pos );
			}
			elem++;
		}
		return ret;
	}
	virtual bool append(T*const& e)
	{
		size_t pos;
		if( !this->find( e, 0, pos) )
		{
			return this->cVect.insert( (void*const&)e, 1, pos );
		}
		return true;
	}
	virtual bool append(T*const& e, size_t cnt)
	{
		size_t pos;
		if( !this->find( e, 0, pos) )
		{
			return this->cVect.insert( (void*const&)e, 1, pos );
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated append
	template<typename EE, typename AA> bool append(const ptrvector<T,EE,AA>& array)
	{
		bool ret = true;
		typename ptrvector<T,EE,AA>::cVect::iterator iter(array.cVect);
		size_t pos;
		for( ; iter; ++iter )
		{
			if( !this->find(*iter, 0, pos) )
			{
				ret &= this->cVect.insert( *iter, 1, pos );
			}
		}
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos
	virtual bool insert(T*const* elem, size_t cnt, size_t pos=~0)
	{
		return this->append( elem, cnt);
	}
	virtual bool insert(T*const& elem, size_t cnt=1, size_t pos=~0)
	{
		return this->append( elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// templated insert
	template<typename EE, typename AA> bool insert(const ptrvector<T,EE,AA>& array, size_t pos)
	{
		return this->append(array);
	}

	///////////////////////////////////////////////////////////////////////////
	/// copy the given array to pos, 
	/// overwrites existing elements, 
	/// expands automatically but does not shrink when array is already larger
	virtual bool copy(T*const* elem, size_t cnt, size_t pos=0)
	{
		this->removeindex(pos,cnt);
		return this->append( elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given array to pos, 
	// overwrites existing elements, 
	// expands automatically but does not shrink when array is already larger
	template<typename EE, typename AA> bool copy(const ptrvector<T,EE,AA>& array, size_t pos=0)
	{
		this->removeindex(pos, array.size());
		return this->append(array);
	}

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(T*const* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos, poscnt);
		return this->append( elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	template<class EE, class AA> bool replace(const ptrvector<T,EE,AA>& array, size_t pos, size_t poscnt)
	{
		this->removeindex(pos, poscnt);
		return this->append(array);
	}

	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	/// implement fifo behaviour (push to the end and pop from front)
	virtual bool push( T*const& elem)				{ return this->append(elem); }
	virtual bool push( T*const* elem, size_t cnt)	{ return this->append(elem, cnt); }

	template<class EE, class AA> bool push(const ptrvector<T,EE,AA>& array){ return this->append(array); }

	///////////////////////////////////////////////////////////////////////////
	/// search within the field
	bool find(T*const& elem, size_t start, size_t& pos) const
	{
		return BinarySearch((void*)elem, (const void**)(this->cVect.begin()), this->cVect.size(), start, pos, &this->cmp, this->config.ascending);
	}
	///////////////////////////////////////////////////////////////////////////
	/// sort the array
	void sort()
	{
		if(this->size()>1)
			QuickSortClassic( this->cVect.begin(), 0, this->cVect.size()-1, &this->cmp, this->config.ascending); 
	}
private:
	///////////////////////////////////////////////////////////////////////////
	/// callback for searching and sorting
	static int cmp(const void* elem, const void* listelem, bool asc)
	{
		if( elem == listelem || *((const T*)elem)==*((const T*)listelem) )
			return 0;
		else if( *((const T*)elem) < *((const T*)listelem) )
			return (asc)?-1:+1;
		else
			return (asc)?+1:-1;
	}

};




///////////////////////////////////////////////////////////////////////////////
/// object vector.
/// stores a vector of pointers to objects
/// but access is still on element base
/// uses partial specialisation of vector template storing all pointer types as void*
template <typename T, typename E=elaborator_ct<void*>, typename A=allocator_w_dy<void*> >
class objvector : public vectorinterface<T>
{
public:
	typedef T*					value_type;
	typedef const value_type*	const_iterator;
	typedef value_type*			simple_iterator;

	///////////////////////////////////////////////////////////////////////////
	/// iterator.
	/// just wrapps internal iterator with casts 
	/// so it is completely removed by the compiler
	class iterator
	{
		typename A::iterator iter;
	public:
		iterator(const objvector& a)
			: iter(a.cVect)
		{}
		iterator(const objvector& a, const T*const* p)
			: iter(a.cVect, (void**)p)
		{}
		~iterator()	{}
		// can use default copy//assignment
		const iterator& operator=(const T*const* p)
		{
			this->iter=(void**)p;
			return *this;
		}
		const iterator& operator=(const objvector& a)
		{
			this->iter = a.cVect;
			return *this;
		}
		bool isvalid() const	{ return this->iter.isvalid(); }
		operator bool() const	{ return this->iter.isvalid(); }
		T* operator()()			{ return *((T**)this->iter.operator()()); }
		T& operator*()			{ return *((T* )this->iter.operator*() ); }
		T* operator->()			{ return *((T**)this->iter.operator->()); }

		T** operator++()			
		{	// preincrement
			return ((T**)(++this->iter));
		}
		T** operator++(int)
		{	// postincrement
			return ((T**)(this->iter++));
		}
		T** operator--()
		{	// predecrement
			return ((T**)(--this->iter));
		}
		T** operator--(int)
		{	// postdecrement
			return ((T**)(this->iter--));
		}
		bool operator==(const T*const*p) const	{ return this->iter==(void**)p; }
		bool operator!=(const T*const*p) const	{ return this->iter!=(void**)p; }
		bool operator>=(const T*const*p) const	{ return this->iter>=(void**)p; }
		bool operator> (const T*const*p) const	{ return this->iter> (void**)p; }
		bool operator<=(const T*const*p) const	{ return this->iter<=(void**)p; }
		bool operator< (const T*const*p) const	{ return this->iter< (void**)p; }

		bool operator==(const iterator&i) const	{ return this->iter==i.iter; }
		bool operator!=(const iterator&i) const	{ return this->iter!=i.iter; }
		bool operator>=(const iterator&i) const	{ return this->iter>=i.iter; }
		bool operator> (const iterator&i) const	{ return this->iter> i.iter; }
		bool operator<=(const iterator&i) const	{ return this->iter<=i.iter; }
		bool operator< (const iterator&i) const	{ return this->iter< i.iter; }
	};
	friend class iterator;

	virtual const T** begin() const		{ return (const T**)this->cVect.begin(); }
	virtual const T** end() const		{ return (const T**)this->cVect.end(); }
	virtual const T** final() const		{ return (const T**)this->cVect.final(); }
	virtual       T** begin()			{ return (      T**)this->cVect.begin(); }
	virtual       T** end()				{ return (      T**)this->cVect.end(); }
	virtual       T** final()			{ return (      T**)this->cVect.final(); }


protected:
	vector<void*,E,A>	cVect;
public:
	///////////////////////////////////////////////////////////////////////////
	// 
	objvector<T,E,A>()				{}
	virtual ~objvector<T,E,A>()		{ this->clear(); }

public:
	///////////////////////////////////////////////////////////////////////////
	// constructors/assignments
	objvector<T,E,A>(const objvector<T,E,A>& v) : cVect(v.cVect){}
	const objvector<T,E,A> operator=(const objvector<T,E,A>& v)	{ cVect = v.cVect; return *this;}


	///////////////////////////////////////////////////////////////////////////
	// 
	virtual size_t size() const		{ return this->cVect.size(); }
	virtual size_t length() const	{ return this->cVect.length(); }

	///////////////////////////////////////////////////////////////////////////
	// 
	virtual bool is_empty() const	{ return this->cVect.is_empty(); }

	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates to newsize but leaves cnt as it is
	// usefull prior to large insertions
	virtual bool realloc(size_t newsize=0)
	{
		return this->cVect.realloc(newsize);
	}
	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1], 
	/// leave new elements uninitialized/default constructed/cuts trailing
	virtual bool resize(size_t cnt)
	{
		if( cnt < this->cVect.size() )
		{	// clear cutted elements
			T** ptr = (T**)this->cVect.begin()+this->cVect.size();
			T** end = (T**)this->cVect.begin()+cnt;
			while(ptr>end)
			{
				ptr--;
				delete ptr;
				*ptr=NULL;
			}
			return this->cVect.resize(cnt);
		}
		else
		{	// add default elements
			return this->append( T(), cnt-this->cVect.size());
		}
	}
	///////////////////////////////////////////////////////////////////////////
	/// move elements inside the buffer
	// using element assignments
	virtual bool move(size_t tarpos, size_t srcpos, size_t cnt=1)
	{
		return this->cVect.move(tarpos, srcpos, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		if( inx < this->cVect.size() )
		{
			T** ptr = (T**)this->cVect.begin()+inx;
			delete *ptr;
			return this->cVect.removeindex(inx);
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		if( inx < this->cVect.size() )
		{
			T** ptr = (T**)this->cVect.begin()+inx;
			T** end = ((inx+cnt)<this->cVect.size())?((T**)this->cVect.begin()+cnt):((T**)this->cVect.begin()+this->cVect.size());
			while( ptr<end )
			{
				delete *ptr;
				*ptr++ = NULL;
			}
			return this->cVect.removeindex(inx, cnt);
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// removes elements from the end
	virtual bool strip(size_t cnt)
	{
		T** ptr = (T**)this->cVect.begin()+this->cVect.size();
		T** end = ( cnt >= this->cVect.size() )? ((T**)this->cVect.begin()) : (ptr-cnt);
		while( ptr>end )
		{
			--ptr;
			delete *ptr;
			*ptr = NULL;
		}
		return this->cVect.strip(cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// removes elements from the start
	virtual bool skip(size_t cnt)
	{
		return removeindex(0, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	virtual bool clear()
	{
		T** ptr = (T**)this->cVect.begin();
		T** end = (T**)this->cVect.begin()+this->cVect.size();
		while( ptr<end )
		{
			delete *ptr;
			*ptr++ = NULL;
		}
		return this->cVect.clear();
	}
	///////////////////////////////////////////////////////////////////////////
	/// assignment 
	virtual bool assign(const objvector<T,E,A>& v)
	{
		this->clear();
		this->append(v);
		return true;
	}
	virtual bool assign(const T* e, size_t cnt)
	{
		this->clear();
		return this->append(e,cnt);
	}
	virtual bool assign(const T& e)
	{
		this->clear();
		return this->append(e);
	}
	virtual bool assign(const T& e, size_t cnt)
	{
		this->clear();
		return this->append(e,cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(const objvector<T,E,A>& v)
	{
		size_t i;
		for(i=0; i<v.size(); ++i)
			this->append( v[i] );
		return true;
	}
	virtual bool append(const T* elem, size_t cnt)
	{
		const T* end=elem+cnt;
		while(elem<end)
			this->cVect.append( (void*)new T(*elem++) );
		return true;
	}
	virtual bool append(const T& elem)
	{
		return this->cVect.append( (void*)new T(elem) );
	}
	virtual bool append(const T& elem, size_t cnt)
	{
		while(cnt--)
			this->cVect.append( (void*)new T(elem) );
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		this->cVect.resize( this->cVect.size()+cnt);
		this->cVect.move(pos+cnt, pos, cnt);
		T** ptr = (T**)this->cVect.begin()+pos;
		T** end = ptr+cnt;
		while(ptr<end)
			*ptr++ = new T(*elem++);
		return true;
	}
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		size_t sz = this->cVect.size();
		this->cVect.resize(sz+cnt);
		this->cVect.move(pos, sz, cnt);
		T** ptr = (T**)this->cVect.begin()+pos;
		T** end = ptr+cnt;
		while(ptr<end)
			*ptr++ = new T(elem);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// copy the given array to pos, 
	/// overwrites existing elements, 
	/// expands automatically but does not shrink when array is already larger
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		T** ptr = (T**)this->cVect.begin()+pos;
		T** end = (T**)this->cVect.end();
		const T*  xxx = elem+cnt;
		while( ptr<=end && elem<xxx)
		{	// overwriting exisitng elements
			**ptr++ = *elem++;
		}
		while(elem<xxx)
		{	// appending the rest
			this->cVect.append( (void*)new T(*elem++) );
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		if( cnt>poscnt )
		{	// enlarging
			this->copy(elem, poscnt, pos);						// overwrite the the existing
			this->insert(elem+poscnt, cnt-poscnt, pos+poscnt);	// insert the rest
		}
		else
		{	// reduce
			this->copy(elem, cnt, pos);							// overwrite the the existing
			this->removeindex(pos+cnt,poscnt-cnt);				// remove the rest
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// access to element[inx] 0 and length()-1
	T& first()				{ return **((T**)this->cVect.begin()); }
	T& last()				{ return **((T**)this->cVect.end()); }
	const T& first() const	{ return **((T**)this->cVect.begin()); }
	const T& last() const	{ return **((T**)this->cVect.end()); }

	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx]
	virtual const T& operator () (size_t inx) const	{ return *((T*&)cVect[inx]); }
	virtual const T& operator[](size_t inx) const	{ return *((T*&)cVect[inx]); }
	virtual const T& operator[](int inx) const		{ return *((T*&)cVect[inx]); }
	virtual       T& operator () (size_t inx)		
	{
		// automatic resize on out-of-bound
		if( inx>=this->cVect.size() )
		{	
			printf("objvector autoresize\n");
			this->resize(inx+1);
		}

		if( !this->cVect[inx] )
		{	// create a new element if not exist
			this->cVect = new T();
		}
		return *((T*&)cVect[inx]);
	}
	virtual       T& operator[](size_t inx)			{ return this->operator()(inx); }
	virtual       T& operator[](int inx)			{ return this->operator()((size_t)inx); }

	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	virtual bool push(const T& elem)
	{
		return this->append(elem, 1);
	}
	virtual bool push(const T* elem, size_t cnt)
	{
		return this->append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T pop()
	{
		T tmp = *((T*&)this->cVect[0]);
		this->removeindex(0);
		return tmp;
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cVect.size() > 0 )
		{
			elem = *((T*&)this->cVect[0]);
			this->removeindex(0);
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T& top() const
	{
		return *((T*&)this->cVect[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cVect.size() > 0 )
		{
			elem = *((T*&)this->cVect[0]);
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// basic default compare operators
	virtual bool operator==(const objvector&e) const { return this==&e; }
	virtual bool operator< (const objvector&e) const { return this< &e; }
};

///////////////////////////////////////////////////////////////////////////////
/// object vector.
/// stores a vector of pointers to objects
/// uses partial specialisation of vector template storing all pointer types as void*
template <typename T, typename E=elaborator_ct<void*>, typename A=allocator_w_dy<void*> >
class objslist : public objvector<T,E,A>
{
protected:
    struct _config
    {
        unsigned ownobjects : 1;	// array is responsible for destroying				// necessary?
        unsigned sorted     : 1;	// sorted array										// necessary?
        unsigned ascending  : 1;	// sort order										// necessary? could be always ascending
        unsigned duplicates : 1;	// sorted: allows duplicate keys					// ok
        unsigned casesens   : 1;	// sorted: string comparison is case sensitive		// ??
        unsigned _reserved  :27;

		_config(bool a=true, bool d=false) : 
			ownobjects(1),
			sorted(1),
			ascending(a),
			duplicates(d),
			casesens(0)
		{}
    } config;
public:
	///////////////////////////////////////////////////////////////////////////
	// 
	objslist<T,E,A>()				{}
	virtual ~objslist<T,E,A>()		{}

public:
	///////////////////////////////////////////////////////////////////////////
	// constructors/assignments
	objslist<T,E,A>(const objvector<T,E,A>& v)					{ this->append(v); }
	const objslist<T,E,A> operator=(const objvector<T,E,A>& v)	{ this->assign(v); return *this; }
	objslist<T,E,A>(const objslist<T,E,A>& v)					{ this->append(v); }
	const objslist<T,E,A> operator=(const objslist<T,E,A>& v)	{ this->assign(v); return *this; }


	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1], 
	/// leave new elements uninitialized/default constructed/cuts trailing
	virtual bool resize(size_t cnt)
	{
		if( cnt < this->cVect.size() )
		{	// clear cutted elements
			T** ptr = (T**)this->cVect.begin()+this->cVect.size();
			T** end = (T**)this->cVect.begin()+cnt;
			while(ptr>end)
			{
				ptr--;
				delete ptr;
				*ptr=NULL;
			}
			return this->cVect.resize(cnt);
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// move elements inside the buffer
	// using element assignments
	virtual bool move(size_t tarpos, size_t srcpos, size_t cnt=1)
	{
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(const objvector<T,E,A>& v)
	{
		size_t i;
		for(i=0; i<v.size(); ++i)
			this->append( v[i] );
		return true;
	}
	virtual bool append(const T* elem, size_t cnt)
	{
		const T* end=elem+cnt;
		while(elem<end)
			this->append( *elem++ );
		return true;
	}
	virtual bool append(const T& elem)
	{
		size_t pos;
		if( !this->find(elem, 0, pos) | this->config.duplicates )
		{
			this->cVect.insert( (void*)new T(elem), 1, pos);
			return true;
		}
		return false;
	}
	virtual bool append(const T& elem, size_t cnt)
	{
		while(cnt--)
			this->append( elem );
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		return this->append(elem, cnt);
	}
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		return this->append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// copy the given array to pos, 
	/// overwrites existing elements, 
	/// expands automatically but does not shrink when array is already larger
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		this->removeindex(pos, cnt);
		return this->append(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos, poscnt);
		return this->append(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	/// access to element[inx] 0 and length()-1
	T& first()				{ return **((T**)this->cVect.begin()); }
	T& last()				{ return **((T**)this->cVect.end()); }
	const T& first() const	{ return **((T**)this->cVect.begin()); }
	const T& last() const	{ return **((T**)this->cVect.end()); }

	///////////////////////////////////////////////////////////////////////////
	/// access to elements(inx) [inx]
	virtual const T& operator () (size_t inx) const	{ return *((T*&)this->cVect[inx]); }
	virtual const T& operator[](size_t inx) const	{ return *((T*&)this->cVect[inx]); }
	virtual const T& operator[](int inx) const		{ return *((T*&)this->cVect[inx]); }
	virtual       T& operator () (size_t inx)		{ return *((T*&)this->cVect[inx]); }
	virtual       T& operator[](size_t inx)			{ return *((T*&)this->cVect[inx]); }
	virtual       T& operator[](int inx)			{ return *((T*&)this->cVect[inx]); }

	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	virtual bool push(const T& elem)
	{
		return this->append(elem, 1);
	}
	virtual bool push(const T* elem, size_t cnt)
	{
		return this->append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T pop()
	{
		T tmp = *((T*&)this->cVect[0]);
		this->removeindex(0);
		return tmp;
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cVect.size() > 0 )
		{
			elem = *((T*&)this->cVect[0]);
			this->removeindex(0);
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T& top() const
	{
		return *((T*&)this->cVect[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cVect.size() > 0 )
		{
			elem = *((T*&)this->cVect[0]);
			return true;
		}
		return false;
	}

	bool find(const T& elem, size_t start, size_t& pos) const
	{
		return BinarySearch((void*)&elem, (const void**)this->cVect.begin(), this->cVect.size(), start, pos, &this->cmp, this->config.ascending);
	}

	void sort()
	{
		if(this->size()>1)
			QuickSortClassic( (const void**)this->cVect.begin(), 0, this->cVect.size()-1, &this->cmp, this->config.ascending); 
	}

private:
	static int cmp(const void* elem, const void* listelem, bool asc)
	{
		if( elem == listelem || *((const T*)elem)==*((const T*)listelem) )
			return 0;
		else if( *((const T*)elem) < *((const T*)listelem) )
			return (asc)?-1:+1;
		else
			return (asc)?+1:-1;
	}
};





///////////////////////////////////////////////////////////////////////////////
/// map. associates a data object with a key value
/// using a pointer array for storing, may switch to a tree later
/// does not allow multiple data entries per key
/// to get similar object as stl::multimap
/// just use ie. map< key, vector<data> >
template<class K, class D> class map
{
private:
	typedef struct _node
	{
		K key;
		D data;
		_node(const K& k) : key(k), data(D())			{}
		_node(const K& k, const D& d) : key(k), data(d)	{}
	} node ;
	ptrvector<node>	cVect;

public:
	typedef typename ptrvector<node>::const_iterator const_iterator;
	typedef typename ptrvector<node>::simple_iterator simple_iterator;
	typedef typename ptrvector<node>::iterator iterator;
	operator ptrvector<node>&()				{ return this->cVect; }
	operator const ptrvector<node>&() const { return this->cVect; }
	ptrvector<node>& operator()(size_t i=0)	{ return this->cVect; }
	const ptrvector<node>& operator()(size_t i=0) const	{ return this->cVect; }


	virtual const node** begin() const	{ return this->cVect.begin(); }
	virtual const node** end() const	{ return this->cVect.end(); }
	virtual const node** final() const	{ return this->cVect.final(); }

	virtual       node** begin()		{ return this->cVect.begin(); }
	virtual       node** end()			{ return this->cVect.end(); }
	virtual       node** final()		{ return this->cVect.final(); }

	virtual size_t size() const			{ return this->cVect.size(); }
	virtual size_t length() const		{ return this->cVect.size(); }

private:
	static int cmp(const K& k,  node* const & n)
	{
		if(k==n->key)
		{
			return 0;
		}
		else if(k<n->key)
			return -1;
		else
			return +1;
	}

	bool find(const K& key, size_t& pos) const
	{
		return BinarySearchC<K, ptrvector<node>, node*> (key, cVect, cVect.size(), 0, pos, &this->cmp);
	}


public:
	map()
	{ }
	virtual ~map()
	{
		clear();
	}
	map(const map& m)
	{
		typename ptrvector<node>::iterator iter(m.cVect);
		for(; iter; ++iter)
			this->insert(iter->key, iter->data);
	}
	const map& operator=(const map& m)
	{
		typename ptrvector<node>::iterator iter(m.cVect);
		this->clear();
		for(; iter; ++iter)
			this->insert(iter->key, iter->data);
		return *this;
	}

	void clear()
	{
		size_t i=cVect.size();
		while(i)
		{
			i--;
			delete cVect[i];
			cVect[i]=NULL;
		}
		cVect.clear();
	}

	bool exists(const K& key) const
	{
		size_t pos;
		return this->find(key, pos);
	}
	bool insert(const K& key, const D& data)
	{
		size_t pos;
		if( this->find(key, pos) )
			cVect[pos]->data = data;
		else
		{	// create a new entry with default data
			node* n = new node(key,data);
			if(n)
				cVect.insert(n, 1, pos);
			else
				return false;
			//throw exception("out of memory");
		}
		return true;
	}
	bool erase(const K& key)
	{
		size_t pos;
		if( this->find(key, pos) )
		{
			delete cVect[pos];
			cVect.removeindex(pos);
			return true;
		}
		return false;
	}
	D& operator[](const K& key)
	{
		size_t pos;
		if( !this->find(key, pos) )
		{	// create a new entry with default data
			node* n = new node(key);
			if(n)
				cVect.insert(n, 1, pos);
			//else
			//throw exception("out of memory");
		}
		return cVect[pos]->data;
	}
	const D& operator[](const K& key) const
	{
		return const_cast<map<K,D>*>(this)->operator[](key);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// simple map. associates a data object with a key value
/// using a linear array for storing, more efficient for small objects
/// does not allow multiple data entries per key
template<class K, class D> class smap
{
private:
	typedef struct _node
	{
		K key;
		D data;
		_node() : key(K()), data(D())	{}
		_node(const K& k, const D& d) : key(k), data(d)	{}
		// array compares
		bool operator==(const _node& n) const	{ return key==n.key; }
		bool operator< (const _node& n) const	{ return key< n.key; }


		friend bool operator==(const K& key, const _node& n)	{ return key==n.key; }
		friend bool operator!=(const K& key, const _node& n)	{ return key!=n.key; }
		friend bool operator< (const K& key, const _node& n)	{ return key< n.key; }
		friend bool operator<=(const K& key, const _node& n)	{ return key<=n.key; }
		friend bool operator> (const K& key, const _node& n)	{ return key> n.key; }
		friend bool operator>=(const K& key, const _node& n)	{ return key>=n.key; }
	} node ;


	vector<node>	cVect;
public:
	typedef typename vector<node>::const_iterator const_iterator;
	typedef typename vector<node>::simple_iterator simple_iterator;
	typedef typename vector<node>::iterator iterator;
	operator vector<node>&()				{ return this->cVect; }
	operator const vector<node>&() const	{ return this->cVect; }
	vector<node>& operator()(size_t i=0)	{ return this->cVect; }
	const vector<node>& operator()(size_t i=0) const	{ return this->cVect; }

	virtual const node* begin() const	{ return this->cVect.begin(); }
	virtual const node* end() const		{ return this->cVect.end(); }
	virtual const node* final() const	{ return this->cVect.final(); }
	virtual       node* begin()			{ return this->cVect.begin(); }
	virtual       node* end()			{ return this->cVect.end(); }
	virtual       node* final()			{ return this->cVect.final(); }

	virtual size_t size() const			{ return this->cVect.size(); }
	virtual size_t length() const		{ return this->cVect.size(); }

private:
	bool find(const K& key, size_t& pos) const
	{
		return BinarySearch<K, const node*>(key, cVect.begin(), cVect.size(), 0, pos);
	}

public:
	smap()
	{ }
	virtual ~smap()
	{
		clear();
	}
	void clear()
	{
		cVect.clear();
	}

	bool exists(const K& key) const
	{
		size_t pos;
		return this->find(key, pos);
	}
	bool insert(const K& key, const D& data)
	{
		size_t pos;
		if( this->find(key, pos) )
			cVect[pos].data = data;
		else
			cVect.insert(node(key,data), 1, pos);
		return true;
	}
	bool erase(const K& key)
	{
		size_t pos;
		if( this->find(key, pos) )
		{
			cVect.removeindex(pos);
			return true;
		}
		return false;
	}
	D& operator[](const K& key)
	{
		size_t pos;
		if( !this->find(key, pos) )
		{	// create a new entry with default data
			cVect.insert(node(key,D()), 1, pos);
		}
		return cVect[pos].data;
	}
	const D& operator[](const K& key) const
	{
		return const_cast<smap<K,D>*>(this)->operator[](key);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// map with two keys. 
/// keys need to be of different type
template<class K1, class K2, class D> class dualmap
{
private:
	typedef struct _node
	{
		K1 key1;
		K2 key2;
		D data;
		_node(const K1& k1, const K2& k2)
			: key1(k1), key2(k2), data(D())		{}
		_node(const K1& k1, const K2& k2, const D& d)
			: key1(k1), key2(k2), data(d)		{}
	} node ;
	ptrvector<node>	cVect1;
	ptrvector<node>	cVect2;
public:
	typedef typename ptrvector<node>::const_iterator const_iterator;
	typedef typename ptrvector<node>::simple_iterator simple_iterator;
	typedef typename ptrvector<node>::iterator iterator;
	operator ptrvector<node>&()				{ return this->cVect1; }
	operator const ptrvector<node>&() const	{ return this->cVect1; }
	ptrvector<node>& operator()(size_t i=0)	{ return (i==0)?(this->cVect1):(this->cVect2); }
	const ptrvector<node>& operator()(size_t i=0) const	{ return (i==0)?(this->cVect1):(this->cVect2); }

	virtual const node** begin() const	{ return this->cVect1.begin(); }
	virtual const node** end() const	{ return this->cVect1.end(); }
	virtual const node** final() const	{ return this->cVect1.final(); }

	virtual       node** begin()		{ return this->cVect1.begin(); }
	virtual       node** end()			{ return this->cVect1.end(); }
	virtual       node** final()		{ return this->cVect1.final(); }

	virtual size_t size() const			{ return this->cVect1.size(); }
	virtual size_t length() const		{ return this->cVect1.size(); }

private:
	static int cmp1(const K1& k,  node* const & n)
	{
		if(k==n->key1)
		{
			return 0;
		}
		else if(k<n->key1)
			return -1;
		else
			return +1;
	}
	static int cmp2(const K2& k,  node* const & n)
	{
		if(k==n->key2)
		{
			return 0;
		}
		else if(k<n->key2)
			return -1;
		else
			return +1;
	}

	bool find(const K1& key, size_t& pos)
	{
		return BinarySearchC<K1, ptrvector<node>, node*> (key, cVect1, cVect1.size(), 0, pos, &this->cmp1);
	}
	bool find(const K2& key, size_t& pos)
	{
		return BinarySearchC<K2, ptrvector<node>, node*> (key, cVect2, cVect2.size(), 0, pos, &this->cmp2);
	}
public:
	dualmap()
	{ }
	virtual ~dualmap()
	{
		clear();
	}
	dualmap(const dualmap& m)
	{
		typename ptrvector<node>::iterator iter(m.cVect1);
		for(; iter; ++iter)
			this->insert(iter->key1, iter->key2, iter->data);
	}
	const dualmap& operator=(const dualmap& m)
	{
		typename ptrvector<node>::iterator iter(m.cVect1);
		this->clear();
		for(; iter; ++iter)
			this->insert(iter->key1, iter->key2, iter->data);
		return *this;
	}

	void clear()
	{
		size_t i=cVect1.size();
		while(i)
		{
			i--;
			delete cVect1[i];
			cVect1[i]=NULL;
		}
		cVect1.clear();
		cVect2.clear();
	}

	template<class K> bool exists(const K& key)
	{
		size_t pos;
		return this->find(key, pos);
	}
	bool insert(const K1& key1, const K2& key2, const D& data)
	{
		size_t pos1, pos2;
		bool f1=this->find(key1, pos1);
		bool f2=this->find(key2, pos2);
		if( f1 || f2 )
		{	// something found somewhere
			if( !f1 || !f2 || cVect1[pos1] != cVect2[pos2] )
			{	// tryed to insert data with wrong keys
				return false;
			}
			else
			{	// update the data
				cVect1[pos1]->data = data;
			}
		}
		else
		{	// create a new entry
			node* n = new node(key1, key2, data);
			if(n)
			{
				cVect1.insert(n, 1, pos1);
				cVect2.insert(n, 1, pos2);
			}
			else
				return false;
			//throw exception("out of memory");
		}
		return true;
	}
	bool erase(const K1& key1)
	{
		size_t pos1, pos2;
		if( this->find(key1, pos1) && this->find( cVect1[pos1].key2, pos2) )
		{
			delete cVect1[pos1];
			cVect1.removeindex(pos1);
			cVect2.removeindex(pos2);
			return true;
		}
		return false;
	}
	bool erase(const K2& key2)
	{
		size_t pos1, pos2;
		if( this->find(key2, pos2) && this->find( cVect2[pos2].key1, pos1) )
		{
			delete cVect1[pos1];
			cVect1.removeindex(pos1);
			cVect2.removeindex(pos2);
			return true;
		}
		return false;
	}

	D& operator[](const K1& key1)
	{
		size_t pos1;
		if( !this->find(key1, pos1) )
		{	// can only throw here
			vector_error("dualmap: key not found");
		}
		return cVect1[pos1]->data;
	}
	D& operator[](const K2& key2)
	{
		size_t pos2;
		if( !this->find(key2, pos2) )
		{	// can only throw here
			vector_error("dualmap: key not found");
		}
		return cVect2[pos2]->data;
	}
	const D& operator[](const K1& key1) const
	{
		return const_cast<dualmap<K1,K2,D>*>(this)->operator[](key1);
	}
	const D& operator[](const K2& key2) const
	{
		return const_cast<dualmap<K1,K2,D>*>(this)->operator[](key2);
	}

};
























///////////////////////////////////////////////////////////////////////////////
/// basic interface for arrays.
/// might be merged with vectors althought these are slightly faster
/// because of different memory allocation
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArray : public global
{
protected:
	virtual const T* array() const=0;

protected:
	///////////////////////////////////////////////////////////////////////////
	// copy, move, compare
	virtual void copy(T* tar, const T* src, size_t cnt) = 0;
	virtual void move(T* tar, const T* src, size_t cnt) = 0;
	virtual int  compare(const T& a, const T& b) const = 0;

public:
	virtual bool realloc(size_t newsize) = 0;
	virtual bool realloc(size_t expectaddition, size_t growsize) = 0;
	virtual bool realloc() = 0;

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArray()							{}
	virtual ~TArray()					{}

	///////////////////////////////////////////////////////////////////////////
	/// direct access to the buffer
	virtual const T* getreadbuffer(size_t &maxcnt) const=0;
	virtual T* getwritebuffer(size_t &maxcnt)			=0;

	virtual bool setreadsize(size_t cnt)				=0;
	virtual bool setwritesize(size_t cnt)				=0;

	///////////////////////////////////////////////////////////////////////////
	/// copy cnt elements from array to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)		=0;

	///////////////////////////////////////////////////////////////////////////
	/// access to element[inx]
	virtual const T& operator[](size_t inx) const 	=0;
	virtual T& operator[](size_t inx)		=0;	

	T& first()	{ return this->operator[](0); }
	T& last()	{ return this->operator[]((this->size()>0)?(this->size()-1):0); }


	///////////////////////////////////////////////////////////////////////////
	/// (re)allocates a array of cnt elements [0...cnt-1]. 
	/// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			=0;	


	///////////////////////////////////////////////////////////////////////////
	/// returns number of elements
	virtual size_t size() const				=0;	
	virtual size_t freesize() const			=0;	


	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	virtual bool push(const T& elem)		{ return append(elem); }
	virtual bool push(const TArray<T>& array){ return append(array); }
	virtual bool push(const T* elem, size_t cnt){ return append(elem,cnt); }
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from array
	virtual T pop()							=0;	
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)				=0;	
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from array
	virtual T& top() const					=0;	
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const			=0;	


	///////////////////////////////////////////////////////////////////////////
	/// add an element at the end
	virtual bool append(const T& elem, size_t cnt=1) =0;	
	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& array) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt) =0;	

	///////////////////////////////////////////////////////////////////////////
	/// remove elements from end of array
	virtual bool strip(size_t cnt=1) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	virtual bool removeindex(size_t inx)	=0;	
	///////////////////////////////////////////////////////////////////////////
	/// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)	=0;	
	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	virtual bool clear()					=0;	

	///////////////////////////////////////////////////////////////////////////
	/// move an element inside the buffer
	virtual bool move(size_t tarpos, size_t srcpos) = 0;
	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	/// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	/// add an array of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& array, size_t pos=~0)	=0;	

	///////////////////////////////////////////////////////////////////////////
	/// copy the given array
	virtual bool copy(const TArray<T>& array, size_t pos=0)	=0;
	///////////////////////////////////////////////////////////////////////////
	/// copy the given array
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)	=0;

	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with array
	virtual bool replace(const TArray<T>& array, size_t pos, size_t poscnt)	=0;	
	///////////////////////////////////////////////////////////////////////////
	/// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt) 	=0;	


	///////////////////////////////////////////////////////////////////////////
	/// find an element in the array
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const=0;
	virtual ssize_t find(const T& elem, size_t startpos=0) const=0;

	///////////////////////////////////////////////////////////////////////////
	/// basic default compare operators
	virtual bool operator==(const TArray<T>&e) const { return this==&e; }
	virtual bool operator< (const TArray<T>&e) const { return this< &e; }
};
///////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////
/// fixed size arrays.
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TArrayFST : public TArray<T>
{
	virtual const T* array() const	{return cField;}
protected:
	///////////////////////////////////////////////////////////////////////////
	// data elements
	T		cField[SZ];	// fixed size array
	size_t	cCnt;		// used elements

	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		memcpy(tar,src,cnt*sizeof(T));
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		memmove(tar,src,cnt*sizeof(T));
	}
	virtual int compare(const T& a, const T& b) const
	{	// dont have a working compare here
		// overload at slist
		return 0;
	}
public:
	virtual bool  realloc(size_t newsize) { return false; }
	virtual bool realloc(size_t expectaddition, size_t growsize)	{ return false; }
	virtual bool realloc()	{ return false; }

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayFST() : cCnt(0)	{}
	virtual ~TArrayFST()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayFST(const TArray<T>& arr):cCnt(0)					{ this->copy(arr); }
	const TArrayFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this;}
	TArrayFST(const TArrayFST<T,SZ>& arr):cCnt(0)			{ this->copy(arr); }
	const TArrayFST& operator=(const TArrayFST<T,SZ>& arr)	{ this->resize(0); this->copy(arr); return *this;}

	TArrayFST(const T* elem, size_t sz):cCnt(0)				{ this->copy(elem, sz); }

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer, return Pointer/max buffer size
	virtual const T* getreadbuffer(size_t &maxcnt) const
	{
		if( cCnt >0 )
		{
			maxcnt = cCnt;
			return const_cast<T*>(cField);
		}
		maxcnt = 0;
		return NULL;
	}
	virtual T* getwritebuffer(size_t &maxcnt)
	{
		if( cCnt < SZ )
		{
			maxcnt = SZ-cCnt;
			return const_cast<T*>(cField+cCnt);
		}
		maxcnt = 0;
		return NULL;
	}
	virtual bool setreadsize(size_t cnt)
	{
		bool ret = false;
		if( cnt <= cCnt )
		{
			if( cnt >0 )
			{
				move(cField+0, cField+cnt,cCnt-cnt);
				cCnt -= cnt;
			}
			ret = true;
		}
		return ret;
	}
	virtual bool setwritesize(size_t cnt)
	{
		bool ret = false;
		if( cCnt+cnt < SZ )
		{
			cCnt += cnt;
			ret = true;
		}
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from array to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)
	{
		if(buf)
		{
			if(cnt>cCnt) cnt = cCnt;
			copy(buf,cField,cnt);
			return cnt;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= SZ )
		{
			vector_error("TArrayF out of bound");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	virtual T &operator[](size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= SZ )
		{
			vector_error("TArrayF out of bound");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a array of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			
	{
		if( cnt < SZ )
		{
			cCnt = cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// returns number of used elements
	virtual size_t size() const				{ return cCnt; }	
	virtual size_t freesize() const			{ return SZ-cCnt; }


	///////////////////////////////////////////////////////////////////////////
	// return the last element and remove it from array
	virtual T pop()
	{
		if( cCnt > 0 )
		{
			return cField[--cCnt];
		}
#ifdef CHECK_BOUNDS
			vector_error("TArrayF underflow");
#ifndef CHECK_EXCEPTIONS
			static T dummy;
			return dummy;
#endif
#endif
		return cField[0];
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( cCnt > 0 )
		{
			elem = cField[--cCnt];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from array
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		if( cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			vector_error("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
			return const_cast<T&>(cField[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( cCnt > 0 )
		{
			elem = cField[0];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// move inside the array
	virtual bool move(size_t tarpos, size_t srcpos)
	{	
		if(srcpos>cCnt) srcpos=cCnt; 
		if( ( tarpos > srcpos && cCnt+tarpos < SZ+srcpos ) || // enlarge only up to limit
			( tarpos < srcpos                            ) )
		{
			move(cField+tarpos,cField+srcpos,cCnt-srcpos);
			cCnt += tarpos-srcpos;
			return true;
		}	
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T& elem, size_t cnt=1)
	{
		if(cCnt < SZ)
		{
			while(cnt--) cField[cCnt++] = elem;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& array)
	{	
		size_t cnt;
		const T* elem = array.getreadbuffer(cnt);
		return append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		if( elem && cCnt+cnt < SZ )
		{
			copy(cField+cCnt,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of array
	virtual bool strip(size_t cnt=1)
	{
		if( cnt <= cCnt )
		{
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		if(inx < cCnt)
		{
			move(cField+inx,cField+inx+1,cCnt-inx-1);
			cCnt--;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		if(inx < cCnt)
		{
			if(inx+cnt > cCnt)	cnt = cCnt-inx;
			move(cField+inx,cField+inx+cnt,cCnt-inx-cnt);
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()
	{
		cCnt = 0;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		if( cCnt+cnt < SZ )
		{
			if(pos >= cCnt) 
				pos = cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			while(cnt--) cField[pos+cnt] = elem;
			cCnt+=cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		if( elem && cCnt+cnt < SZ )
		{
			if(pos >= cCnt) 
				pos=cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			copy(cField+pos,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an array of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& array, size_t pos=~0)
	{	
		size_t cnt;
		const T* elem = array.getreadbuffer(cnt);
		return insert(elem,cnt, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// place cnt elements at position pos (at the end by default) overwriting existing elements
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		if( elem )
		{	
			if(pos > cCnt) pos = cCnt;

			if( pos+cnt < SZ )
			{
				copy(cField+pos,elem,cnt);
				cCnt = pos+cnt;
				return true;
			}
		}
		return false;
	}
	virtual bool copy(const TArray<T>& array, size_t pos=0)
	{	
		if(this!=&array)
		{
			size_t cnt;
			const T* elem = array.getreadbuffer(cnt);
			return copy(elem,cnt, pos);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		if(pos > cCnt)
		{
			pos = cCnt;
			poscnt = 0;
		}
		if(pos+poscnt > cCnt) 
		{
			poscnt=cCnt-pos;
		}
		if( elem && (cCnt+cnt < SZ+poscnt) )
		{
			move(cField+pos+cnt, cField+pos+poscnt,cCnt-pos-poscnt);
			copy(cField+pos,elem,cnt);
			cCnt += cnt-poscnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with array
	virtual bool replace(const TArray<T>& array, size_t pos, size_t poscnt)
	{	
		size_t cnt;
		const T* elem = array.getreadbuffer(cnt);
		return replace(elem,cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the array
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{
		for(size_t i=startpos; i<cCnt; ++i)
		{
			if( elem== cField[i] )
			{	pos = i;
				return true;
			}
		}
		return false;
	}
	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		for(size_t i=startpos; i<cCnt; ++i)
		{
			if( elem== cField[i] )
			{	
				return i;
			}
		}
		return -1;
	}
};




///////////////////////////////////////////////////////////////////////////////
/// fixed size fifo
template <class T, size_t SZ> class TfifoFST : public TArrayFST<T,SZ>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoFST()	{}
	virtual ~TfifoFST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoFST(const TArray<T>& arr)						{ this->copy(arr); }
	const TfifoFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TfifoFST(const TfifoFST<T,SZ>& arr)					{ this->copy(arr); }
	const TfifoFST& operator=(const TfifoFST<T,SZ>& arr){ this->resize(0); this->copy(arr); return *this; }


	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from array
	virtual T pop()
	{
		if( this->cCnt > 0 )
		{
			T elem = this->cField[0];
			this->move(this->cField+0, this->cField+1, --this->cCnt);
			return elem;
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		vector_error("TArrayF underflow");
#else
		static T dummy;
		return dummy;
#endif
#endif
		return this->cField[0];
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[0];
			move(this->cField+0, this->cField+1, --this->cCnt);
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TstackFST : public TArrayFST<T,SZ>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackFST()				{}
	virtual ~TstackFST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackFST(const TArray<T>& arr)							{ this->copy(arr); }
	const TstackFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TstackFST(const TstackFST<T,SZ>& arr)					{ this->copy(arr); }
	const TstackFST& operator=(const TstackFST<T,SZ>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from array
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			vector_error("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return const_cast<T&>(this->cField[this->cCnt]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[this->cCnt];
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
// fixed size sorted array.
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TslistFST : public TfifoFST<T,SZ>
{
	bool cAscending;// sorting order
	bool cAllowDup;	// allow duplicate entries (find might then not find specific elems)

	virtual int compare(const T&a, const T&b) const
	{
		if( a>b )		return (cAscending) ?  1:-1;
		else if( a<b )	return (cAscending) ? -1: 1;
		else			return 0;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// destructor
	TslistFST(bool as=true, bool ad=false):cAscending(as),cAllowDup(ad) {}
	virtual ~TslistFST() {}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistFST(const TArray<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)		{ this->copy(arr); }
	const TslistFST& operator=(const TArray<T>& arr)									{ this->resize(0); this->copy(arr); return *this; }
	TslistFST(const TslistFST<T,SZ>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistFST& operator=(const TslistFST<T,SZ>& arr)								{ this->resize(0); this->copy(arr); return *this; }

	TslistFST(const T* elem, size_t sz):cAscending(true),cAllowDup(false)				{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// add an element to the array
	virtual bool push(const T& elem) 				{ return insert(elem); }
	virtual bool push(const TArray<T>& array)		{ return insert(array); }
	virtual bool push(const T* elem, size_t cnt)	{ return insert(elem,cnt); }

	///////////////////////////////////////////////////////////////////////////
	// add an element to the array
	virtual bool append(const T& elem, size_t cnt=1){ bool ret=false; while(cnt--) ret=insert(elem); return ret;}
	virtual bool append(const TArray<T>& array)		{ return insert(array); }
	virtual bool append(const T* elem, size_t cnt)	{ return insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t pos=~0)
	{
		// ignore position, insert sorted
		if( this->cCnt <SZ )
		{
			bool f = this->find(elem, 0, pos);
			if( !f || cAllowDup )
			{
				move(this->cField+pos+1, this->cField+pos, this->cCnt-pos);
				this->cCnt++;
				this->cField[pos] = elem;
			}
			return true;
		}
		// else found
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		for(size_t i=0; i<cnt && this->cCnt<SZ; ++i)
		{	
			this->insert( elem[i] );
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an array of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& array, size_t pos=~0)
	{
		if(this!=&array)
		{
			for(size_t i=0; i<array.size() && this->cCnt<SZ; ++i)
			{
				this->insert( array[i] );
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given array
	virtual bool copy(const TArray<T>& array, size_t pos=0)
	{
		if(this!=&array)
		{
			this->clear();
			return this->insert(array);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy the given array
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		this->clear();
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with array
	virtual bool replace(const TArray<T>& array, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(array);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// find an element in the array
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{	
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1;
		size_t c;
		pos = 0;

		if( NULL==this->cField || this->cCnt < 1)
			ret = false;
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else if( cAscending )
		{	//smallest element first
			if( elem < this->cField[a] )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( elem > this->cField[b] )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem < this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( elem > this->cField[a] )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( elem < this->cField[b] )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem > this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		return ret;
	}
	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		size_t pos;
		if( this->find(elem,startpos, pos) )
			return pos;
		return -1;
	}
};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// dynamic size arrays.
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArrayDST : public TArray<T>
{
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class MiniString;
public:
	virtual const T* array() const	{ return cField; }
protected:
	///////////////////////////////////////////////////////////////////////////
	// data elements
	T		*cField;	// array
	size_t	cSZ;		// allocates array size
	size_t	cCnt;		// used elements

	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		memcpy(tar,src,cnt*sizeof(T));
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		memmove(tar,src,cnt*sizeof(T));
	}
	virtual int  compare(const T& a, const T& b) const	
	{	// dont have a working compare here
		// overload at slist
		return 0;
	}

public:
	virtual bool realloc(size_t newsize)
	{	
		if(  cSZ < newsize )
		{	// grow rule
			size_t tarsize = newsize;
			newsize = 2;
			while( newsize < tarsize ) newsize <<= 1;
		}
		else if( cSZ>8 && cCnt < (cSZ>>2) && newsize < (cSZ>>1))
		{	// shrink rule
			newsize = (cSZ>>1);
		}
		else // no change
			return true;


		T *newfield = new T[newsize];
		if(newfield==NULL)
			vector_error("TArrayDST: memory allocation failed");

		if(cField)
		{
			copy(newfield, cField, cCnt); // between read ptr and write ptr
			delete[] cField;
		}
		cSZ = newsize;
		cField = newfield;
		return cField!=NULL;
	}
	virtual bool realloc(size_t expectaddition, size_t growsize)
	{
		if(cCnt+expectaddition >= cSZ)
		{	// need to enlarge
			if(cSZ==0) cSZ=2;
			while(cCnt+expectaddition >= cSZ)
				cSZ+=(growsize)?growsize:cSZ;
			T* newfield = new T[cSZ];
			if(newfield)
			{
				if(cField) 
				{
					copy(newfield,cField,cCnt);
					delete[] cField;
				}
				cField = newfield;
			}
		}
		return cField!=NULL;
	}
	virtual bool realloc()
	{
		if(cCnt != cSZ)
		{	
			if(cCnt)
			{	// need to resize
				cSZ = cCnt;
				T* newfield = new T[cSZ];
				if(newfield)
				{
					if(cField) 
					{
						copy(newfield,cField,cCnt);
						delete[] cField;
					}
					cField = newfield;
				}
			}
			else
			{	// just clear all
				if(cField)
				{
					delete[] cField;
					cField=NULL;
					cSZ = 0;
				}
			}
		}
		return cField!=NULL;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayDST() : cField(NULL),cSZ(0),cCnt(0)	{}
	TArrayDST(size_t sz) : cField(NULL),cSZ(0),cCnt(0)	{ resize(sz); }
	virtual ~TArrayDST()	{ if(cField) delete[] cField; }

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayDST(const TArray<T>& arr) : cField(NULL),cSZ(0),cCnt(0)	{ this->copy(arr); }
	const TArrayDST& operator=(const TArray<T>& arr)				{ this->resize(0); this->copy(arr); return *this; }
	TArrayDST(const TArrayDST<T>& arr) : cField(NULL),cSZ(0),cCnt(0){ this->copy(arr); }
	const TArrayDST& operator=(const TArrayDST<T>& arr)				{ this->resize(0); this->copy(arr); return *this; }

	TArrayDST(const T* elem, size_t sz):cField(NULL),cSZ(0),cCnt(0)	{ printf("TArrayDST T* construct\n"); this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// put a element to the array
	virtual bool push(const T& elem)			{ return append(elem); }
	virtual bool push(const TArray<T>& array)	{ return append(array); }
	virtual bool push(const T* elem, size_t cnt){ return append(elem,cnt); }
	///////////////////////////////////////////////////////////////////////////
	// return the last element and remove it from array
	virtual T pop()
	{
		if( cCnt > 0 )
		{
			return cField[--cCnt];
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			vector_error("TArrayF underflow");
#else
		static T dummy;
		return dummy;
#endif
#endif
		return cField[0];

	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( cCnt > 0 )
		{
			elem = cField[--cCnt];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from array
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		if( cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			vector_error("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
			return const_cast<T&>(cField[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( cCnt > 0 )
		{
			elem = cField[0];
			return true;
		}
		return false;
	}


	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer, return Pointer and max buffer size
	virtual const T* getreadbuffer(size_t &maxcnt) const
	{
		if( cCnt >0 )
		{
			maxcnt = cCnt;
			return const_cast<T*>(cField);
		}
		maxcnt = 0;
		return NULL;
	}
	virtual T* getwritebuffer(size_t &maxcnt)
	{
		if( cCnt+maxcnt > cSZ )
			realloc( maxcnt+cCnt );
		return const_cast<T*>(cField+cCnt);
	}
	virtual bool setreadsize(size_t cnt)
	{
		bool ret = false;
		if( cnt <= cCnt)
		{
			if( cnt >0 )
			{
				move(cField+0, cField+cnt,cCnt-cnt);
				cCnt -= cnt;
			}
			ret = true;
		}
		return ret;
	}
	virtual bool setwritesize(size_t cnt)
	{
		bool ret = false;
		if( cCnt+cnt < cSZ )
		{
			cCnt += cnt;
			ret = true;
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from array to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)
	{
		if(buf)
		{
			if(cnt>cCnt) cnt = cCnt;
			copy(buf,cField,cnt);
			return cnt;
		}
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= cCnt )
		{
#ifdef CHECK_EXCEPTIONS
			vector_error("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	virtual T &operator[](size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= cCnt )
		{
#ifdef CHECK_EXCEPTIONS
			vector_error("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a array of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			
	{
		if(cnt ==0)
		{
			if(cField)
			{
				delete[] cField;
				cField=NULL;
			}
			cSZ = cCnt = 0;
			return true;
		}
		else
		{
			if(cnt > cSZ)
				realloc(cnt);
			cCnt = cnt;
			return NULL!=cField;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// returns number of used elements
	virtual size_t size() const				{ return cCnt; }	
	virtual size_t freesize() const			{ return cSZ-cCnt; }	


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T& elem, size_t cnt=1)
	{
		if(cCnt+cnt > cSZ)
			realloc(cCnt+cnt);
		while(cnt--) cField[cCnt++] = elem;
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& array)
	{	
		size_t cnt;
		const T* elem = array.getreadbuffer(cnt);
		return append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		if( elem )
		{
			if( cCnt+cnt > cSZ)
				realloc( cCnt+cnt );
			copy(cField+cCnt,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of array
	virtual bool strip(size_t cnt=1)
	{
		if( cnt <= cCnt )
		{
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		if(inx < cCnt)
		{
			move(cField+inx,cField+inx+1,cCnt-inx-1);
			cCnt--;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		if(inx < cCnt)
		{
			if(inx+cnt > cCnt)	cnt = cCnt-inx;
			move(cField+inx,cField+inx+cnt,cCnt-inx-cnt);
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()
	{
		cCnt = 0;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		if( cCnt+cnt > cSZ )
			realloc(cSZ+cnt);

		if(pos >= cCnt) 
			pos = cCnt;
		else
			move(cField+pos+cnt, cField+pos, cCnt-pos);

		cCnt += cnt;
		while(cnt--) cField[pos++] = elem;
		return true;

	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		if( elem )
		{	
			if( cCnt+cnt > cSZ )
				realloc(cCnt+cnt);

			if(pos >= cCnt) 
				pos=cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			copy(cField+pos,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an array of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& array, size_t pos=~0)
	{	
		size_t cnt;
		const T* elem = array.getreadbuffer(cnt);
		return insert(elem,cnt, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements at position pos (at the end by default) overwriting existing elements
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		if( elem )
		{	
			if(pos > cCnt) pos = cCnt;

			if( pos+cnt > cSZ )
				realloc(pos+cnt);
			copy(cField+pos,elem,cnt);
			cCnt = pos+cnt;
			return true;
		}
		return false;
	}
	virtual bool copy(const TArray<T>& array, size_t pos=0)
	{	
		if(this!=&array)
		{
			size_t cnt;
			const T* elem = array.getreadbuffer(cnt);
			return copy(elem,cnt, pos);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// Moving elements inside the buffer
	// always take the elements from 'from' up to 'cElements'

	virtual bool move(size_t tarpos, size_t srcpos)
	{	
		if(srcpos>cCnt) srcpos=cCnt; 
		if( cCnt+tarpos > cSZ+srcpos )
			realloc(cCnt+tarpos-srcpos);
		move(cField+tarpos,cField+srcpos,cCnt-srcpos);
		cCnt += tarpos-srcpos;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		if(pos > cCnt)
		{
			pos = cCnt;
			poscnt = 0;
		}
		if(pos+poscnt > cCnt) 
		{
			poscnt=cCnt-pos;
		}
		if( elem )
		{
			if( cCnt+cnt > cSZ+poscnt)
				realloc(cCnt+cnt-poscnt);

			move(cField+pos+cnt, cField+pos+poscnt,cCnt-pos-poscnt);
			copy(cField+pos,elem,cnt);
			cCnt += cnt-poscnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with array
	virtual bool replace(const TArray<T>& array, size_t pos, size_t poscnt)
	{	
		size_t cnt;
		const T* elem = array.getreadbuffer(cnt);
		return replace(elem,cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the array
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{
		for(size_t i=startpos; i<cCnt; ++i)
			if( elem == cField[i] )
			{	pos = i;
				return true;
			}
		return false;
	}
	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		for(size_t i=startpos; i<cCnt; ++i)
			if( elem == cField[i] )
			{	
				return i;
			}
		return -1;
	}
};
///////////////////////////////////////////////////////////////////////////////
/// dynamic size fifo
template <class T> class TfifoDST : public TArrayDST<T>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoDST()	{}
	virtual ~TfifoDST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoDST(const TArray<T>& arr)						{ this->copy(arr); }
	const TfifoDST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TfifoDST(const TfifoDST<T>& arr)					{ this->copy(arr); }
	const TfifoDST& operator=(const TfifoDST<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from array
	virtual T pop()
	{
		if( this->cCnt > 0 )
		{
			T elem = this->cField[0];
			this->move(this->cField+0, this->cField+1,--this->cCnt);
			return elem;
		}
#ifdef CHECK_BOUNDS
		vector_error("TArrayF underflow");
#endif
#ifdef CHECK_EXCEPTIONS
		static T dummy;
		return dummy;
#else
		return this->cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[0];
			move(this->cField+0, this->cField+1,--this->cCnt);
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
/// dynamic size stack
template <class T> class TstackDST : public TArrayDST<T>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackDST()				{}
	virtual ~TstackDST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackDST(const TArray<T>& arr)						{ this->copy(arr); }
	const TstackDST& operator=(const TArray<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }
	TstackDST(const TstackDST<T>& arr)					{ this->copy(arr); }
	const TstackDST& operator=(const TstackDST<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from array
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			vector_error("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return const_cast<T&>(this->cField[this->cCnt]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[this->cCnt];
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
/// dynamic size sorted array
template <class T> class TslistDST : public TfifoDST<T>
{
	bool cAscending;// sorting order
	bool cAllowDup;	// allow duplicate entries (find might then not find specific elems)

	virtual int compare(const T&a, const T&b) const
	{
		if( a>b )		return (cAscending) ?  1:-1;
		else if( a<b )	return (cAscending) ? -1: 1;
		else			return 0;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// destructor
	TslistDST(bool as=true, bool ad=false):cAscending(as),cAllowDup(ad) {}
	virtual ~TslistDST() {}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistDST(const TArray<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistDST& operator=(const TArray<T>& arr)								{ this->resize(0); this->copy(arr); return *this; }
	TslistDST(const TslistDST<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistDST& operator=(const TslistDST<T>& arr)								{ this->resize(0); this->copy(arr); return *this; }

	TslistDST(const T* elem, size_t sz):cAscending(true),cAllowDup(false)			{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// add an element to the array
	virtual bool push(const T& elem) 				{ return this->insert(elem); }
	virtual bool push(const TArray<T>& array)		{ return this->insert(array); }
	virtual bool push(const T* elem, size_t cnt)	{ return this->insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos at the end by default
	virtual bool append(const T& elem, size_t cnt=1){ bool ret=false; while(cnt--) ret=this->insert(elem); return ret;}
	virtual bool append(const TArray<T>& array) 		{ return this->insert(array); }
	virtual bool append(const T* elem, size_t cnt) 	{ return this->insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t pos=~0)
	{	// ignore position, insert sorted
		bool f = find(elem, 0, pos);
		if( !f || cAllowDup )
		{
			if( this->cCnt >= this->cSZ )
				this->realloc(this->cSZ+1);

			this->move(this->cField+pos+1, this->cField+pos, this->cCnt-pos);
			this->cCnt++;
			this->cField[pos] = elem;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		if( this->cCnt+cnt>this->cSZ )
			realloc(this->cCnt+cnt);

		for(size_t i=0; i<cnt; ++i)
		{	
			this->insert( elem[i] );
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an array of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& array, size_t pos=~0)
	{
		if(this!=&array)
		{
			if( this->cCnt+array.size()>this->cSZ )
				this->realloc(this->cCnt+array.size());

			for(size_t i=0; i<array.size(); ++i)
			{
				this->insert( array[i] );
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given array
	virtual bool copy(const TArray<T>& array, size_t pos=0)
	{
		if(this!=&array)
		{
			this->clear();
			return this->insert(array);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy the given array
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		this->clear();
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with array
	virtual bool replace(const TArray<T>& array, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(array);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// find an element in the array
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{	
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1;
		size_t c;
		pos = 0;

		if( NULL==this->cField || this->cCnt < 1)
			ret = false;
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else if( cAscending )
		{	//smallest element first
			if( elem < this->cField[a] )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( elem > this->cField[b] )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem < this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( elem > this->cField[a] )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( elem < this->cField[b] )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem > this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		return ret;
	}

	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		size_t pos;
		if( this->find(elem,startpos, pos) )
			return pos;
		return -1;
	}

};
///////////////////////////////////////////////////////////////////////////////
/// dynamic size array using assignment loops for copy
template <class T> class TArrayDCT : public TArrayDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; ++i)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; ++i)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayDCT()				{}
	virtual ~TArrayDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayDCT(const TArray<T>& arr)						{ TArrayDST<T>::copy(arr); }
	const TArrayDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TArrayDST<T>::copy(arr); return *this; }
	TArrayDCT(const TArrayDCT<T>& arr)					{ TArrayDST<T>::copy(arr); }
	const TArrayDCT& operator=(const TArrayDCT<T>& arr)	{ this->resize(0); TArrayDST<T>::copy(arr); return *this; }
};

/// dynamic size fifo using assignment loops for copy
template <class T> class TfifoDCT : public TfifoDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; ++i)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; ++i)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoDCT()			{}
	virtual ~TfifoDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoDCT(const TArray<T>& arr)							{ TfifoDST<T>::copy(arr); }
	const TfifoDCT& operator=(const TArray<T>& arr)			{ this->resize(0); TfifoDST<T>::copy(arr); return *this; }
	TfifoDCT(const TfifoDCT<T>& arr)						{ TfifoDST<T>::copy(arr); }
	const TfifoDCT& operator=(const TfifoDCT<T>& arr)		{ this->resize(0); TfifoDST<T>::copy(arr); return *this; }

};

/// dynamic size stack using assignment loops for copy
template <class T> class TstackDCT : public TstackDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; ++i)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; ++i)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackDCT()				{}
	virtual ~TstackDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackDCT(const TArray<T>& arr)						{ TstackDST<T>::copy(arr); }
	const TstackDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TstackDST<T>::copy(arr); return *this; }
	TstackDCT(const TstackDCT<T>& arr)					{ TstackDST<T>::copy(arr); }
	const TstackDCT& operator=(const TstackDCT<T>& arr)	{ this->resize(0); TstackDST<T>::copy(arr); return *this; }

};

/// dynamic size slist using assignment loops for copy
template <class T> class TslistDCT : public TslistDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for complex data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; ++i)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; ++i)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TslistDCT()				{}
	virtual ~TslistDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistDCT(const TArray<T>& arr)						{ TslistDST<T>::copy(arr); }
	const TslistDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TslistDST<T>::copy(arr); return *this; }
	TslistDCT(const TslistDCT<T>& arr)					{ TslistDST<T>::copy(arr); }
	const TslistDCT& operator=(const TslistDST<T>& arr)	{ this->resize(0); TslistDST<T>::copy(arr); return *this; }
};






///////////////////////////////////////////////////////////////////////////////
/// List of Objects. implemented as array of pointers to (single) objects
/// it actually owns the pointers and deletes them on exit
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArrayDPT
{
	typedef T* pT;

	pT*		cField;
	size_t	cCnt;
	size_t	cSZ;

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	TArrayDPT() : cField(NULL),cCnt(0),cSZ(0)			{  }
	TArrayDPT(size_t cnt) : cField(NULL),cCnt(0),cSZ(0)	{ resize(cnt); }
	~TArrayDPT()										{ clear(); }

	///////////////////////////////////////////////////////////////////////////
	// copy/assign
	TArrayDPT(const TArrayDPT& tp) : cField(NULL),cCnt(0),cSZ(0)
	{
		this->copy(tp);
	}

	const TArrayDPT& operator=(const TArrayDPT& tp)
	{
		this->resize(0); 
		this->copy(tp);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// access functions
	void copy(const TArrayDPT& tp)
	{
		if(this!=&tp)
		{
			size_t i;
			realloc(tp.cCnt);
			for(i=0; i<tp.cCnt; ++i)
			{
				if(tp.cField[i])
				{	// array element in source array, copy it
					if(!cField[i])
						cField[i] = new T(*tp.cField[i]);
					else
						*cField[i] = *tp.cField[i];
				}
				else
				{	// empty element in source array
					if(cField[i])
					{
						delete cField[i];
						cField[i] = NULL;
					}
				}

			}
			this->cCnt=tp.cCnt;
		}
	}
	void clear()
	{
		if(cField)
		{	
			size_t i;
			for(i=0; i<cSZ; ++i)
				if(cField[i]) delete cField[i];
			delete[] cField;
			cField=NULL;
			cCnt=0;
			cSZ=0;
		}
	}
	bool realloc(size_t newsize)
	{
		if(newsize >= cSZ)
		{	// need to enlarge
			size_t sz = (cSZ)?cSZ:2;
			while(newsize >= sz) sz*=2;
			pT* temp = new pT[sz];
			if(temp)
			{
				memset(temp,0,sz*sizeof(pT));
				if(cField) 
				{
					memcpy(temp,cField,cSZ*sizeof(pT));
					delete[] cField;
				}
				cField = temp;
				cSZ = sz;
			}
		}
		return cField!=NULL;
	}
	bool realloc(size_t expectaddition, size_t growsize)
	{
		if(cCnt+expectaddition >= cSZ)
		{	// need to enlarge
			size_t sz = (cSZ)?cSZ:2;
			while(cCnt+expectaddition >= sz)
				sz+=(growsize)?growsize:sz;
			pT* temp = new pT[sz];
			if(temp)
			{
				memset(temp,0,sz*sizeof(pT));
				if(cField) 
				{
					memcpy(temp,cField,cSZ*sizeof(pT));
					delete[] cField;
				}
				cField = temp;
				cSZ = sz;
			}
		}
		return cField!=NULL;
	}
	bool resize(size_t cnt)
	{
		if(cnt >= cSZ)
			realloc(cnt);
		cCnt = cnt;
		return NULL!=cField;
	}
	T& operator[](size_t inx)
	{
		// automatic resize on out-of-bound
		if( inx>=cCnt )
		{
			printf("autoresize %lu->%lu\n",(unsigned long)cCnt, (unsigned long)(inx+1));
			resize(inx+1);
		}

		if( !cField[inx] )
		{	// create a new element if not exist
			cField[inx] = new T;
		}
		return *cField[inx];
	}
	const T& operator[](size_t inx) const
	{	// throw something here
		if( inx>=cCnt || !cField[inx] )
			vector_error("TPointerList");

		return *cField[inx];
	}

	T& first()
	{
		return this->operator[](0);
	}
	T& last()
	{	
		return this->operator[]((cCnt>0)?(cCnt-1):0);
	}
	size_t size() const			{ return cCnt; }

	bool insert(const T& elem, size_t pos) // insert an element at position pos
	{
		insert(pos);
		if(cField[pos])
			*cField[pos] = elem;
		else
			cField[pos] = new T(elem);
		return true;
	}
	bool insert(size_t pos) // insert an element at position pos
	{
		if(pos>=cCnt)
		{
			resize(pos+1);
		}
		else
		{
			resize(cCnt+1);
			pT temp =(cField[cCnt]);
			memmove(cField+pos+1,cField+pos,(cCnt-pos)*sizeof(pT));
			cField[pos]=temp;
		}
		return true;
	}
	bool strip(size_t cnt, bool clear=false)
	{
		if(cCnt>=cnt)
		{
			if(clear)
			{	
				while(cnt>0)
				{
					cCnt--;
					if(cField[cCnt])
					{
						delete cField[cCnt];
						cField[cCnt] = NULL;
					}
				}
			}
			else
				cCnt-=cnt;
			return true;
		}
		return false;
	}

	bool removeindex(size_t inx)
	{
		if(inx<cCnt)
		{
			pT temp = cField[inx];
			memmove(cField+inx, cField+inx+1,(cCnt-inx-1)*sizeof(pT));
			cCnt--;
			cField[cCnt] = temp;
		}
		return false;
	}
	bool append(const T& elem)
	{
		realloc(cCnt+1);
		if(cField[cCnt])
			*cField[cCnt] = elem;
		else
			cField[cCnt]=new T(elem);
		cCnt++;
		return true;
	}
	bool append(const T* elem, size_t cnt)
	{
		realloc(cCnt+cnt);
		for(size_t i=0; i<cnt;++i)
		{
			if(cField[cCnt])
				*cField[cCnt] = elem[i];
			else
				cField[cCnt] = new T(elem[i]);
			cCnt++;
		}
		return true;
	}
	bool pop(bool clear=false)
	{
		if(cCnt>0)
		{
			cCnt--;
			if(clear && cField[cCnt])
			{
				delete cField[cCnt];
				cField[cCnt] = NULL;
			}
			return true;
		}
		return false;
	}
	bool push(const T& elem)
	{
		return append(elem);
	}
	bool push(const T* elem, size_t cnt)
	{
		return append(elem, cnt);
	}
};




///////////////////////////////////////////////////////////////////////////////
/// Multi-Indexed List Template.
/// using a growing base array and sorted pod lists for storing positions
/// base array cannot shrink on delete operations so this is only suitable for static array
/// usable classes need a "int compare(const T& elem, size_t inx) const" member
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiList
{
	vector<T>		cList;
	vector<size_t>	cIndex[CNT];

public:
	TMultiList()	{}
	~TMultiList()	{}

	size_t size() const					{ return cIndex[0].size(); }
	const T& operator[](size_t i) const	{ return cList[ cIndex[0][i] ]; }
	T& operator[](size_t i)				{ return cList[ cIndex[0][i] ]; }

	const T& operator()(size_t p,size_t i=0) const	{ return cList[ cIndex[(i<CNT)?i:0][p] ]; }
	T& operator()(size_t p,size_t i=0)				{ return cList[ cIndex[(i<CNT)?i:0][p] ]; }


	///////////////////////////////////////////////////////////////////////////
	/// add an element
	virtual bool insert(const T& elem)
	{
		size_t i, pos = cList.size();
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; ++i)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			for(i=0; i<CNT; ++i)
			{
				cIndex[i].insert(pos, 1, ipos[i]);
			}
			return cList.append(elem);
		}

		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; ++i)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		T& elem = cList[ cIndex[(inx<CNT)?inx:0][pos] ];
		size_t temppos, i;
		pos = cIndex[inx][pos];

		for(i=0; i<CNT; ++i)
		{
			if( binsearch(elem, 0, i, temppos, true, &T::compare) )
				cIndex[i].removeindex(temppos);
		}
		return cList.removeindex(pos);
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	bool clear()
	{
		size_t i;
		for(i=0; i<CNT; ++i)
			cIndex[i].clear();
		return cList.clear();
	}
	///////////////////////////////////////////////////////////////////////////
	/// find an element in the array
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	/// binsearch.
	/// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)(cList[ cIndex[inx][a] ], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)(cList[ cIndex[inx][a] ], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)(cList[ cIndex[inx][a] ], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};




///////////////////////////////////////////////////////////////////////////////
/// Multi-Indexed List Template.
/// using Pointers to stored objects for internal lists, 
/// subsequent insert/delete does new/delete the objects, 
/// for performance use a managed memory derived classes
/// usable classes need a "int compare(const T& elem, size_t inx) const" member
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiListP
{

	ptrvector<T>	cIndex[CNT];

public:
	TMultiListP()			{}
	virtual ~TMultiListP()	{ clear(); }

	size_t size() const					{ return cIndex[0].size(); }
	const T& operator[](size_t p) const	{ return *cIndex[0][p]; }
	T& operator[](size_t p)				{ return *cIndex[0][p]; }

	const T& operator()(size_t p,size_t i=0) const	{ return *cIndex[(i<CNT)?i:0][p]; }
	T& operator()(size_t p,size_t i=0)				{ return *cIndex[(i<CNT)?i:0][p]; }


	///////////////////////////////////////////////////////////////////////////
	/// add an element
	virtual bool insert(const T& elem)
	{
		size_t i;
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; ++i)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			T* newelem = new T(elem);
			for(i=0; i<CNT; ++i)
			{
				ok &= cIndex[i].insert(newelem, 1, ipos[i]);
			}
		}

		return ok;
	}
	///////////////////////////////////////////////////////////////////////////
	/// add and take over the element pointer
	virtual bool insert(T* elem)
	{
		bool ok=false;
		if(elem)
		{
			size_t i;
			size_t ipos[CNT];
			ok = true;
			for(i=0; i<CNT; ++i)
			{
				ok &= !binsearch(*elem, 0, i, ipos[i], true, &T::compare);
			}
			if(ok)
			{
				for(i=0; i<CNT; ++i)
				{
					ok &= cIndex[i].insert(elem, 1, ipos[i]);
				}
			}
		}
		return ok;
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; ++i)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		T* elem = cIndex[(inx<CNT)?inx:0][pos];
		size_t temppos, i;
		bool ret = true;

		for(i=0; i<CNT; ++i)
		{
			if( binsearch(*elem, 0, i, temppos, true, &T::compare) )
				ret &= cIndex[i].removeindex(temppos);
		}
		// free the removed element
		delete elem;
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	bool clear()
	{
		bool ret = true;
		size_t i;
		// free elements
		for(i=0; i<cIndex[0].size(); ++i)
			delete cIndex[0][i];
		// clear pointer array
		for(i=0; i<CNT; ++i)
			ret &= cIndex[i].clear();
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// find an element in the array
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	/// binsearch.
	/// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)( *cIndex[inx][a], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)( *cIndex[inx][b], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)( *cIndex[inx][b], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)( *cIndex[inx][b], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};



///////////////////////////////////////////////////////////////////////////////
/// Multi-Indexed List Template.
/// using SavePointers to stored objects for internal lists, 
/// subsequent insert/delete does new/delete the objects, 
/// for performance use managed memory derived classes
/// usable classes need a "int compare(const T& elem, size_t inx) const" member
///
/// needs evaluation
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiListSP
{
	vector< TPtrAutoCount<T> >	cIndex[CNT];

public:
	TMultiListSP()	{}
	~TMultiListSP()	{ clear(); }

	size_t size() const												{ return cIndex[0].size(); }
	const TPtrAutoCount<T> operator[](size_t p) const				{ return cIndex[0][p]; }
	TPtrAutoCount<T> operator[](size_t p)							{ return cIndex[0][p]; }

	const TPtrAutoCount<T> operator()(size_t p,size_t i=0) const	{ return cIndex[(i<CNT)?i:0][p]; }
	TPtrAutoCount<T> operator()(size_t p,size_t i=0)				{ return cIndex[(i<CNT)?i:0][p]; }

	///////////////////////////////////////////////////////////////////////////
	/// add an element
	virtual bool insert(const T& elem)
	{
		size_t i;
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; ++i)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			TPtrAutoCount<T> newelem(new T(elem));
			for(i=0; i<CNT; ++i)
			{
				ok &= cIndex[i].insert(newelem, 1, ipos[i]);
			}
		}
		return ok;
	}
	///////////////////////////////////////////////////////////////////////////
	/// add and take over the element pointer
	virtual bool insert(T* elem)
	{
		bool ok=false;
		if(elem)
		{
			size_t i;
			size_t ipos[CNT];
			ok = true;
			for(i=0; i<CNT; ++i)
			{
				ok &= !binsearch(*elem, 0, i, ipos[i], true, &T::compare);
			}
			if(ok)
			{
				TPtrAutoCount<T> xx(elem);
				for(i=0; i<CNT; ++i)
				{
					ok &= cIndex[i].insert(xx, 1, ipos[i]);
				}
			}
		}
		return ok;
	}

	///////////////////////////////////////////////////////////////////////////
	/// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; ++i)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		TPtrAutoCount<T> elem = cIndex[(inx<CNT)?inx:0][pos];
		size_t temppos, i;
		bool ret = true;

		for(i=0; i<CNT; ++i)
		{
			if( binsearch(*elem, 0, i, temppos, true, &T::compare) )
				ret &= cIndex[i].removeindex(temppos);
		}
		// free the removed element
		delete elem;
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	bool clear()
	{
		bool ret = true;
		size_t i;
		// free elements
		for(i=0; i<cIndex[0].size(); ++i)
			cIndex[0][i].clear();
		// clear pointer array
		for(i=0; i<CNT; ++i)
			ret &= cIndex[i].clear();
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	/// find an element in the array
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	/// binsearch.
	/// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)( *cIndex[inx][a], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)( *cIndex[inx][b], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)( *cIndex[inx][b], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)( *cIndex[inx][b], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)

#endif//__BASEARRAY_H__
