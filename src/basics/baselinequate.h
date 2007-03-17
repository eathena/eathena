#ifndef __BASEOTHER_H__
#define __BASEOTHER_H__


#include "basetypes.h"
#include "basemath.h"



///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// subscript type with optional callback.
/// does an assignment to a type and calls a parent object,
/// underlying type has to implement full math operation set,
/// the recalculator itself behaves like the given value_type
template<typename P, typename T>
struct recalculator
{
	typedef T value_type;
private:
	P&	parent;
	T&	object;
	void (P::*func)(void);
public:
	recalculator(P& p, T& o, void (P::*f)(void)=NULL) : parent(p), object(o), func(f)
	{}
	
	// pure read access
	const T& operator()()	{ return object; }

	// assignment
	const recalculator& operator=(const T& o);
	// unary math operations
	T operator!()	{ return !this->object; }
	T operator-()	{ return -this->object; }
	T operator~()	{ return ~this->object; }
	// postop/preop
	const recalculator& operator++();
	const recalculator& operator--();
	T operator++(int);
	T operator--(int);
	// binary math operations
	const recalculator& operator+=(const T& o);
	const recalculator& operator-=(const T& o);
	const recalculator& operator*=(const T& o);
	const recalculator& operator/=(const T& o);
	const recalculator& operator%=(const T& o);
	const recalculator& operator&=(const T& o);
	const recalculator& operator|=(const T& o);
	const recalculator& operator^=(const T& o);
	const recalculator& operator<<=(const T& o);
	const recalculator& operator>>=(const T& o);
};



template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator=(const T& o)
{
	this->object = o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator++()
{
	++this->object;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator--()
{
	--this->object;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
T recalculator<P,T>::operator++(int)
{
	T temp = this->object;
	++this->object;
	if(func)
		(this->parent.*func)();
	return temp;
}

template<typename P, typename T>
T recalculator<P,T>::operator--(int)
{
	T temp = this->object;
	--this->object;
	if(func)
		(this->parent.*func)();
	return temp;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator+=(const T& o)
{
	this->object += o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator-=(const T& o)
{
	this->object -= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator*=(const T& o)
{
	this->object *= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator/=(const T& o)
{
	this->object /= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator%=(const T& o)
{
	this->object %= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator&=(const T& o)
{
	this->object &= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator|=(const T& o)
{
	this->object |= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator^=(const T& o)
{
	this->object ^= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator<<=(const T& o)
{
	this->object <<= o;
	if(func)
		(this->parent.*func)();
	return *this;
}

template<typename P, typename T>
const recalculator<P,T>& recalculator<P,T>::operator>>=(const T& o)
{
	this->object >>= o;
	if(func)
		(this->parent.*func)();
	return *this;
}












///////////////////////////////////////////////////////////////////////////////
/// simple definition for linear equations. could be:\n
/// final = base*factor + addition + final*scale\n
/// where\n
/// - base is some base value
/// - factor is the percentage of the base included in the result
/// - addition is some linear addition independend from factor
/// - scale is the relative scale factor of the result
///
/// meaning that eg. base=100, factor = 0.80, addition = 10, scale=0.50
/// results in final=180\n
///
/// calculating the final as (base*factor + addition)/(1-scale)\n
/// after having checked that scale is smaller than unity (otherwise it describes an oscillating system)\n
/// when using fixedpoint integers with base X as unity for factor and scale this is going to:\n
/// final = (base*factor + X*addition)/(X-scale)\n
/// usefull ranges are:\n
/// - base         0...+inf
/// - addition  -inf...+inf
/// - factor       0...+inf
/// - scale     -inf...1
///
template <typename BT, size_t B=100, typename FT=PARAMETER_TYPENAME signed_type<BT>::Type>
struct linearvalue
{
	typedef recalculator<linearvalue, BT> bsubscript_t;
	typedef recalculator<linearvalue, FT> fsubscript_t;

	FT cAddition;
	FT cScale;
	BT cBase;
	BT cFinal;
	BT cFactor;
	unsigned char cSetzero : 1;
	unsigned char cInvalid : 1;

	/// default construction.
	linearvalue() :
		cAddition(0),cScale(0),cBase(0),cFinal(0),cFactor(B),cSetzero(0),cInvalid(0)
	{}
	/// conversion construction.
	linearvalue(const BT& b) :
		cAddition(0),cScale(0),cBase(b),cFinal(b),cFactor(B),cSetzero(0),cInvalid(0)
	{}
	/// value construction.
	linearvalue(const BT& b, const BT& a, const FT& f, const FT& s) :
		cAddition(a),cScale(s),cBase(b),cFinal(0),cFactor(f),cSetzero(0),cInvalid(0)
	{
		this->refresh();
	}
	// using default copy/assign

	/// function for recalculating the result.
	void refresh();

	/// set forced zero
	void set_zero(bool on)		{ this->cSetzero=on; this->refresh(); }

	/// returns unity value.
	const BT unity() const		{ return B; }
	
	/// default conversion.
	operator const BT&()		{ return this->cFinal; }
	/// read access.
	const BT& operator()()		{ return this->cFinal; }

	/// value assignment operator assigns to base.
	const linearvalue& operator=(const BT& b)		{ this->cBase=b; this->refresh; return *this; }

	// access to the elements
	bsubscript_t base()			{ return bsubscript_t(*this, this->cBase, &linearvalue::refresh); }
	fsubscript_t addition()		{ return fsubscript_t(*this, this->cAddition, &linearvalue::refresh); }
	bsubscript_t factor()		{ return bsubscript_t(*this, this->cFactor, &linearvalue::refresh); }
	fsubscript_t scale()		{ return fsubscript_t(*this, this->cScale, &linearvalue::refresh); }
};

template <typename BT, size_t B, typename FT>
void linearvalue<BT,B,FT>::refresh()
{
	if( this->cSetzero )
		this->cFinal = 0;
	else if( this->cScale<(ssize_t)B )
		this->cFinal = (this->cBase*this->cFactor + B*this->cAddition)/(B-this->cScale), this->cInvalid=0;
	else
		this->cFinal = 0, this->cInvalid=1;
}




///////////////////////////////////////////////////////////////////////////////
/// linear equation without feedback.
/// offset intrusion here is on the amplifier input
/// final = (base+addition)*factor\n
/// where\n
/// - base is some base value
/// - addition is some linear addition
/// - factor is the amplification
///
template <typename BT, size_t B=100, typename FT=PARAMETER_TYPENAME signed_type<BT>::Type>
struct addmulvalue
{
	typedef recalculator<addmulvalue, BT> bsubscript_t;
	typedef recalculator<addmulvalue, FT> fsubscript_t;

	FT cAddition;
	BT cBase;
	BT cFinal;
	BT cFactor;

	/// default construction.
	addmulvalue()
		: cAddition(0),cBase(0),cFinal(0),cFactor(B)
	{}
	/// conversion construction.
	addmulvalue(const BT& b)
		: cAddition(0),cBase(b),cFinal(b),cFactor(B)
	{}
	/// value construction.
	addmulvalue(const BT& b, const FT& a, const FT& f)
		: cAddition(a),cBase(b),cFinal(0),cFactor(f)
	{
		this->refresh();
	}
	// using default copy/assign

	/// function for recalculating the result.
	void refresh();

	/// returns unity value.
	const BT unity() const		{ return B; }
	
	/// default conversion.
	operator const BT&()		{ return this->cFinal; }
	/// read access.
	const BT& operator()()		{ return this->cFinal; }

	/// value assignment operator assigns to base.
	const addmulvalue& operator=(const BT& b)		{ this->cBase=b; this->refresh; return *this; }

	// access to the elements
	bsubscript_t base()			{ return bsubscript_t(*this, this->cBase, &addmulvalue::refresh); }
	fsubscript_t addition()		{ return fsubscript_t(*this, this->cAddition, &addmulvalue::refresh); }
	bsubscript_t factor()		{ return bsubscript_t(*this, this->cFactor, &addmulvalue::refresh); }
};

template <typename BT, size_t B, typename FT>
void addmulvalue<BT,B,FT>::refresh()
{
	this->cFinal = (BT)(((ssize_t)(size_t)this->cBase+(ssize_t)this->cAddition)*(size_t)this->cFactor/B);
}

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASEOTHER_H__
