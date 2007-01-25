#include "status.h"
#include "clif.h"
#include "itemdb.h"






///////////////////////////////////////////////////////////////////////////////
/// temporary copy from basics to play with
namespace minimath
{
template<typename T>
struct signed_type
{
	typedef T Type;
};
template<>
struct signed_type<unsigned char>
{
	typedef signed char Type;
};
template<>
struct signed_type<unsigned short>
{
	typedef signed short Type;
};
template<>
struct signed_type<unsigned int>
{
	typedef signed int Type;
};
template<>
struct signed_type<unsigned long>
{
	typedef signed long Type;
};
template<>
struct signed_type<uint64>
{
	typedef sint64 Type;
};

template<typename T>
struct unsigned_type
{
	typedef T Type;
};
template<>
struct unsigned_type<signed char>
{
	typedef unsigned char Type;
};
template<>
struct unsigned_type<signed short>
{
	typedef unsigned short Type;
};
template<>
struct unsigned_type<signed int>
{
	typedef unsigned int Type;
};
template<>
struct unsigned_type<signed long>
{
	typedef unsigned long Type;
};
template<>
struct unsigned_type<sint64>
{
	typedef uint64 Type;
};

}


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
	operator const T&()		{ return object; }
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
/// simple definition for linear equations could be:\n
/// final = base*factor + addition + final*scale\n
/// where\n
/// - base is some base value
/// - factor is the percentage of the base included in the result
/// - addition is some linear addition independend from factor
/// - scale is the relative scale factor of the result
/// -
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
/// -
///
template <typename BT, size_t B=100, typename FT=PARAMETER_TYPENAME minimath::signed_type<BT>::Type>
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
struct stats
{
	// it would be actually enough to have them as chars as the client can only display chars
	unsigned short str;
	unsigned short agi;
	unsigned short vit;
	unsigned short wis;
	unsigned short dex;
	unsigned short luk;
};




///////////////////////////////////////////////////////////////////////////////
struct testobject
{
	typedef recalculator<testobject, stats> subscript_t;

	stats	base_stats;	// base stat values (comming from underlying object)

	stats	bonusstats;	// pure additive stat modifiers

	// values which base is depending on stats
	linearvalue<ushort,100> batk;
	linearvalue<ushort,100> ratk;
	linearvalue<ushort,100> latk;
	linearvalue<ushort,100> matk1;
	linearvalue<ushort,100> matk2;
	linearvalue<ushort,100> hit;
	linearvalue<ushort,100> flee1;
	linearvalue<ushort,100> flee2;
	linearvalue<ushort,100> def1;
	linearvalue<ushort,100> def2;
	linearvalue<ushort,100> mdef1;
	linearvalue<ushort,100> mdef2;
	linearvalue<ushort,100> crit;
	linearvalue<ushort,100> aspd;

	void calculate_base()
	{	// whatever
	}
	subscript_t bonus()
	{
		return subscript_t(*this,this->bonusstats,&testobject::calculate_base);
	}
};









///////////////////////////////////////////////////////////////////////////////
/// status change object.
/// offers the basic functionality for creating/removing status changes
class status_change_if : public basics::global, public basics::noncopyable
{
protected:
	affectable& object;	///< the affected object

	/// constructor.
	/// only derived classes can be created
	status_change_if(affectable& o) : object(o)
	{}
public:
	// destructor.
	virtual ~status_change_if()
	{}

	/// return the id of the current object
	/// mandatory on derived classes
	virtual status_t status_id() const=0;
	/// true when can be saved.
	virtual bool savable() const	{ return false; }
	/// return remaining time.
	virtual ulong remaining() const	{ return 0; }
	/// activate a status.
	void activate()
	{	// tell the client to activate
		clif_status_change(this->object,this->status_id(),1);
		// call user dependend code
		this->start();
	}
	/// activate a status.
	void deactivate()
	{	// tell the client to deactivate
		clif_status_change(this->object,this->status_id(),0);
		// call user dependend code
		this->stop();
	}
	/// executed when starting the status change.
	virtual void start()	{}
	/// executed when another status has started/stopped.
	/// does recalculation when status changes affect each other
	virtual void restart()	{}
	/// executed when stopping the status change.
	virtual void stop()		{}
};

///////////////////////////////////////////////////////////////////////////////
/// specialized status change object.
/// adds functionality for timed removal of status changes
class status_change_timed : public virtual status_change_if
{
	int timerid;
protected:
	/// constructor.
	status_change_timed(affectable& object, ulong tick) : status_change_if(object), timerid(-1)
	{
		this->timerid=add_timer(tick, status_change_timed::timer_entry, object.block_list::id, basics::numptr(this));
	}
public:
	/// destructor.
	virtual ~status_change_timed()
	{
		if(this->timerid!=-1)
			delete_timer(this->timerid, status_change_timed::timer_entry);
	}
	/// return remaining time.
	virtual ulong remaining() const
	{
		TimerData* td;
		return ( timerid!=-1 && (td=get_timer(timerid)) ) ? td->tick-GetTickCount() : 0;
	}
private:
	/// timer entry.
	/// removes the status changes from the affected object
	static int timer_entry(int tid, unsigned long tick, int id, basics::numptr data)
	{
		//affectable* mv = affectable::from_blid(id);
		status_change_timed* ptr = (status_change_timed*)data.ptr;
		if(ptr)
		{
			if( tid!=ptr->timerid )
			{	
				if(config.error_log)
					printf("statustimerentry %d != %d\n",ptr->timerid,tid);
			}
			else
			{	// erase the status change
				ptr->object.remove_status(ptr);
			}
		}
		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// status change object.
/// offers the basic functionality for creating/removing status changes
class status_change_restartable : public virtual status_change_if
{
protected:
	/// constructor.
	/// only derived classes can be created
	status_change_restartable(affectable& o) : status_change_if(o)
	{}
public:
	// destructor.
	virtual ~status_change_restartable()
	{}

	/// executed when another status has started/stopped.
	/// simply stops the status and starts it again
	virtual void restart()
	{
		this->stop();
		this->start();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// implementation of the different status objects



 /*
	integration into fightable object:
	delta_def2
	delta_base_atk
	delta_right_weapon_watk
	delta_left_weapon_watk
*/




///////////////////////////////////////////////////////////////////////////////
/// SC_PROVOKE.
class sc_provoke : public status_change_restartable
{
	int def2;
	int base_atk;
	int right_weapon_watk;
	int left_weapon_watk;
public:
	enum { ID=SC_PROVOKE };
	enum { val1 = 10 };

	sc_provoke(affectable&	o) : status_change_if(o), status_change_restartable(o),
		def2(0),base_atk(0),right_weapon_watk(0),left_weapon_watk(0)
	{}
	virtual ~sc_provoke()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }

	/// executed when starting the status change.
	virtual void start()
	{
		if( this->object == BL_PC )
		{
			map_session_data& sd = *this->object.get_sd();
			sd.def2					-= this->def2				= sd.def2*(5+5*val1)/100;
			sd.base_atk				+= this->base_atk			= sd.base_atk*(2+3*val1)/100;
			sd.right_weapon.watk	+= this->right_weapon_watk	= sd.right_weapon.watk*(2+3*val1)/100;
			const size_t index = sd.equip_index[8];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->type == 4)
				sd.left_weapon.watk	+= this->left_weapon_watk	= sd.left_weapon.watk*(2+3*val1)/100;
		}
	}
	/// executed when stopping the status change.
	virtual void stop()
	{
		if( this->object == BL_PC )
		{
			map_session_data& sd = *this->object.get_sd();
			sd.def2					+= this->def2;
			sd.base_atk				-= this->base_atk;
			sd.right_weapon.watk	-= this->right_weapon_watk;
			sd.left_weapon.watk		-= this->left_weapon_watk;
		}
	}

};

map_session_data *sd;
sc_provoke cc(*sd);

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_endure : public status_change_if
{
public:
	enum { ID=SC_ENDURE };

	sc_endure(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_endure()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_twohandquicken : public status_change_if
{
public:
	enum { ID=SC_TWOHANDQUICKEN };

	sc_twohandquicken(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_twohandquicken()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_concentrate : public status_change_if
{
public:
	enum { ID=SC_CONCENTRATE };

	sc_concentrate(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_concentrate()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_hiding : public status_change_if
{
public:
	enum { ID=SC_HIDING };

	sc_hiding(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_hiding()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_cloaking : public status_change_if
{
public:
	enum { ID=SC_CLOAKING };

	sc_cloaking(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_cloaking()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_encpoison : public status_change_if
{
public:
	enum { ID=SC_ENCPOISON };

	sc_encpoison(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_encpoison()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_poisonreact : public status_change_if
{
public:
	enum { ID=SC_POISONREACT };

	sc_poisonreact(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_poisonreact()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_quagmire : public status_change_if
{
public:
	enum { ID=SC_QUAGMIRE };

	sc_quagmire(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_quagmire()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_angelus : public status_change_if
{
public:
	enum { ID=SC_ANGELUS };

	sc_angelus(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_angelus()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_blessing : public status_change_if
{
public:
	enum { ID=SC_BLESSING };

	sc_blessing(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_blessing()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_signumcrucis : public status_change_if
{
public:
	enum { ID=SC_SIGNUMCRUCIS };

	sc_signumcrucis(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_signumcrucis()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_increaseagi : public status_change_if
{
public:
	enum { ID=SC_INCREASEAGI };

	sc_increaseagi(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_increaseagi()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_decreaseagi : public status_change_if
{
public:
	enum { ID=SC_DECREASEAGI };

	sc_decreaseagi(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_decreaseagi()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_slowpoison : public status_change_if
{
public:
	enum { ID=SC_SLOWPOISON };

	sc_slowpoison(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_slowpoison()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_impositio : public status_change_if
{
public:
	enum { ID=SC_IMPOSITIO };

	sc_impositio(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_impositio()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_suffragium : public status_change_if
{
public:
	enum { ID=SC_SUFFRAGIUM };

	sc_suffragium(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_suffragium()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_aspersio : public status_change_if
{
public:
	enum { ID=SC_ASPERSIO };

	sc_aspersio(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_aspersio()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_benedictio : public status_change_if
{
public:
	enum { ID=SC_BENEDICTIO };

	sc_benedictio(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_benedictio()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_kyrie : public status_change_if
{
public:
	enum { ID=SC_KYRIE };

	sc_kyrie(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_kyrie()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_magnificat : public status_change_if
{
public:
	enum { ID=SC_MAGNIFICAT };

	sc_magnificat(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_magnificat()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_gloria : public status_change_if
{
public:
	enum { ID=SC_GLORIA };

	sc_gloria(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_gloria()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_aeterna : public status_change_if
{
public:
	enum { ID=SC_AETERNA };

	sc_aeterna(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_aeterna()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_adrenaline : public status_change_if
{
public:
	enum { ID=SC_ADRENALINE };

	sc_adrenaline(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_adrenaline()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_weaponperfection : public status_change_if
{
public:
	enum { ID=SC_WEAPONPERFECTION };

	sc_weaponperfection(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_weaponperfection()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_overthrust : public status_change_if
{
public:
	enum { ID=SC_OVERTHRUST };

	sc_overthrust(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_overthrust()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_maximizepower : public status_change_if
{
public:
	enum { ID=SC_MAXIMIZEPOWER };

	sc_maximizepower(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_maximizepower()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_riding : public status_change_if
{
public:
	enum { ID=SC_RIDING };

	sc_riding(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_riding()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_falcon : public status_change_if
{
public:
	enum { ID=SC_FALCON };

	sc_falcon(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_falcon()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_trickdead : public status_change_if
{
public:
	enum { ID=SC_TRICKDEAD };

	sc_trickdead(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_trickdead()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_loud : public status_change_if
{
public:
	enum { ID=SC_LOUD };

	sc_loud(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_loud()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_energycoat : public status_change_if
{
public:
	enum { ID=SC_ENERGYCOAT };

	sc_energycoat(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_energycoat()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_broknarmor : public status_change_if
{
public:
	enum { ID=SC_BROKNARMOR };

	sc_broknarmor(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_broknarmor()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_broknweapon : public status_change_if
{
public:
	enum { ID=SC_BROKNWEAPON };

	sc_broknweapon(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_broknweapon()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_hallucination : public status_change_if
{
public:
	enum { ID=SC_HALLUCINATION };

	sc_hallucination(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_hallucination()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};


///////////////////////////////////////////////////////////////////////////////
/// SC_WEIGHT50.
/// displays a 50% icon
class sc_weight50 : public status_change_if
{
public:
	enum { ID=SC_WEIGHT50 };

	sc_weight50(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_weight50()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// SC_WEIGHT50.
/// displays a 90% icon
class sc_weight90 : public status_change_if
{
public:
	enum { ID=SC_WEIGHT90 };

	sc_weight90(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_weight90()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
	virtual void stop()
	{	// check for starting the 50% status
		if( this->object.is_50overweight() )
			this->object.create_status(SC_WEIGHT50);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion0 : public status_change_if
{
public:
	enum { ID=SC_SPEEDPOTION0 };

	sc_speedpotion0(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_speedpotion0()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion1 : public status_change_if
{
public:
	enum { ID=SC_SPEEDPOTION1 };

	sc_speedpotion1(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_speedpotion1()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion2 : public status_change_if
{
public:
	enum { ID=SC_SPEEDPOTION2 };

	sc_speedpotion2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_speedpotion2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion3 : public status_change_if
{
public:
	enum { ID=SC_SPEEDPOTION3 };

	sc_speedpotion3(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_speedpotion3()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedup0 : public status_change_if
{
public:
	enum { ID=SC_SPEEDUP0 };

	sc_speedup0(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_speedup0()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedup1 : public status_change_if
{
public:
	enum { ID=SC_SPEEDUP1 };

	sc_speedup1(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_speedup1()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_atkpot : public status_change_if
{
public:
	enum { ID=SC_ATKPOT };

	sc_atkpot(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_atkpot()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_matkpot : public status_change_if
{
public:
	enum { ID=SC_MATKPOT };

	sc_matkpot(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_matkpot()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_wedding : public status_change_if
{
public:
	enum { ID=SC_WEDDING };

	sc_wedding(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_wedding()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_slowdown : public status_change_if
{
public:
	enum { ID=SC_SLOWDOWN };

	sc_slowdown(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_slowdown()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_ankle : public status_change_if
{
public:
	enum { ID=SC_ANKLE };

	sc_ankle(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_ankle()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_keeping : public status_change_if
{
public:
	enum { ID=SC_KEEPING };

	sc_keeping(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_keeping()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_barrier : public status_change_if
{
public:
	enum { ID=SC_BARRIER };

	sc_barrier(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_barrier()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stripweapon : public status_change_if
{
public:
	enum { ID=SC_STRIPWEAPON };

	sc_stripweapon(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_stripweapon()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stripshield : public status_change_if
{
public:
	enum { ID=SC_STRIPSHIELD };

	sc_stripshield(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_stripshield()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_striparmor : public status_change_if
{
public:
	enum { ID=SC_STRIPARMOR };

	sc_striparmor(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_striparmor()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_striphelm : public status_change_if
{
public:
	enum { ID=SC_STRIPHELM };

	sc_striphelm(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_striphelm()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_weapon : public status_change_if
{
public:
	enum { ID=SC_CP_WEAPON };

	sc_cp_weapon(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_cp_weapon()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_shield : public status_change_if
{
public:
	enum { ID=SC_CP_SHIELD };

	sc_cp_shield(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_cp_shield()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_armor : public status_change_if
{
public:
	enum { ID=SC_CP_ARMOR };

	sc_cp_armor(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_cp_armor()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_helm : public status_change_if
{
public:
	enum { ID=SC_CP_HELM };

	sc_cp_helm(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_cp_helm()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autoguard : public status_change_if
{
public:
	enum { ID=SC_AUTOGUARD };

	sc_autoguard(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_autoguard()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_reflectshield : public status_change_if
{
public:
	enum { ID=SC_REFLECTSHIELD };

	sc_reflectshield(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_reflectshield()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_splasher : public status_change_if
{
public:
	enum { ID=SC_SPLASHER };

	sc_splasher(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_splasher()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_providence : public status_change_if
{
public:
	enum { ID=SC_PROVIDENCE };

	sc_providence(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_providence()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_defender : public status_change_if
{
public:
	enum { ID=SC_DEFENDER };

	sc_defender(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_defender()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_magicrod : public status_change_if
{
public:
	enum { ID=SC_MAGICROD };

	sc_magicrod(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_magicrod()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_spellbreaker : public status_change_if
{
public:
	enum { ID=SC_SPELLBREAKER };

	sc_spellbreaker(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_spellbreaker()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autospell : public status_change_if
{
public:
	enum { ID=SC_AUTOSPELL };

	sc_autospell(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_autospell()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sighttrasher : public status_change_if
{
public:
	enum { ID=SC_SIGHTTRASHER };

	sc_sighttrasher(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_sighttrasher()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autoberserk : public status_change_if
{
public:
	enum { ID=SC_AUTOBERSERK };

	sc_autoberserk(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_autoberserk()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_spearsquicken : public status_change_if
{
public:
	enum { ID=SC_SPEARSQUICKEN };

	sc_spearsquicken(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_spearsquicken()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autocounter : public status_change_if
{
public:
	enum { ID=SC_AUTOCOUNTER };

	sc_autocounter(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_autocounter()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sight : public status_change_if
{
public:
	enum { ID=SC_SIGHT };

	sc_sight(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_sight()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_safetywall : public status_change_if
{
public:
	enum { ID=SC_SAFETYWALL };

	sc_safetywall(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_safetywall()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_ruwach : public status_change_if
{
public:
	enum { ID=SC_RUWACH };

	sc_ruwach(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_ruwach()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_pneuma : public status_change_if
{
public:
	enum { ID=SC_PNEUMA };

	sc_pneuma(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_pneuma()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stone : public status_change_if
{
public:
	enum { ID=SC_STONE };

	sc_stone(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_stone()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_freeze : public status_change_if
{
public:
	enum { ID=SC_FREEZE };

	sc_freeze(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_freeze()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stun : public status_change_if
{
public:
	enum { ID=SC_STUN };

	sc_stun(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_stun()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sleep : public status_change_if
{
public:
	enum { ID=SC_SLEEP };

	sc_sleep(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_sleep()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_poison : public status_change_if
{
public:
	enum { ID=SC_POISON };

	sc_poison(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_poison()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_curse : public status_change_if
{
public:
	enum { ID=SC_CURSE };

	sc_curse(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_curse()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_silence : public status_change_if
{
public:
	enum { ID=SC_SILENCE };

	sc_silence(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_silence()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_confusion : public status_change_if
{
public:
	enum { ID=SC_CONFUSION };

	sc_confusion(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_confusion()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_blind : public status_change_if
{
public:
	enum { ID=SC_BLIND };

	sc_blind(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_blind()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_divina : public status_change_if
{
public:
	enum { ID=SC_DIVINA };

	sc_divina(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_divina()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_bleeding : public status_change_if
{
public:
	enum { ID=SC_BLEEDING };

	sc_bleeding(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_bleeding()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dpoison : public status_change_if
{
public:
	enum { ID=SC_DPOISON };

	sc_dpoison(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_dpoison()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_extremityfist : public status_change_if
{
public:
	enum { ID=SC_EXTREMITYFIST };

	sc_extremityfist(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_extremityfist()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_explosionspirits : public status_change_if
{
public:
	enum { ID=SC_EXPLOSIONSPIRITS };

	sc_explosionspirits(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_explosionspirits()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_combo : public status_change_if
{
public:
	enum { ID=SC_COMBO };

	sc_combo(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_combo()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_bladestop_wait : public status_change_if
{
public:
	enum { ID=SC_BLADESTOP_WAIT };

	sc_bladestop_wait(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_bladestop_wait()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_bladestop : public status_change_if
{
public:
	enum { ID=SC_BLADESTOP };

	sc_bladestop(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_bladestop()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_flamelauncher : public status_change_if
{
public:
	enum { ID=SC_FLAMELAUNCHER };

	sc_flamelauncher(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_flamelauncher()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_frostweapon : public status_change_if
{
public:
	enum { ID=SC_FROSTWEAPON };

	sc_frostweapon(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_frostweapon()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_lightningloader : public status_change_if
{
public:
	enum { ID=SC_LIGHTNINGLOADER };

	sc_lightningloader(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_lightningloader()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_seismicweapon : public status_change_if
{
public:
	enum { ID=SC_SEISMICWEAPON };

	sc_seismicweapon(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_seismicweapon()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_volcano : public status_change_if
{
public:
	enum { ID=SC_VOLCANO };

	sc_volcano(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_volcano()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_deluge : public status_change_if
{
public:
	enum { ID=SC_DELUGE };

	sc_deluge(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_deluge()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_violentgale : public status_change_if
{
public:
	enum { ID=SC_VIOLENTGALE };

	sc_violentgale(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_violentgale()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_watk_element : public status_change_if
{
public:
	enum { ID=SC_WATK_ELEMENT };

	sc_watk_element(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_watk_element()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_landprotector : public status_change_if
{
public:
	enum { ID=SC_LANDPROTECTOR };

	sc_landprotector(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_landprotector()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_nochat : public status_change_if
{
public:
	enum { ID=SC_NOCHAT };

	sc_nochat(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_nochat()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_baby : public status_change_if
{
public:
	enum { ID=SC_BABY };

	sc_baby(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_baby()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_aurablade : public status_change_if
{
public:
	enum { ID=SC_AURABLADE };

	sc_aurablade(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_aurablade()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_parrying : public status_change_if
{
public:
	enum { ID=SC_PARRYING };

	sc_parrying(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_parrying()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_concentration : public status_change_if
{
public:
	enum { ID=SC_CONCENTRATION };

	sc_concentration(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_concentration()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_tensionrelax : public status_change_if
{
public:
	enum { ID=SC_TENSIONRELAX };

	sc_tensionrelax(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_tensionrelax()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_berserk : public status_change_if
{
public:
	enum { ID=SC_BERSERK };

	sc_berserk(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_berserk()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_fury : public status_change_if
{
public:
	enum { ID=SC_FURY };

	sc_fury(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_fury()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_gospel : public status_change_if
{
public:
	enum { ID=SC_GOSPEL };

	sc_gospel(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_gospel()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_assumptio : public status_change_if
{
public:
	enum { ID=SC_ASSUMPTIO };

	sc_assumptio(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_assumptio()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_basilica : public status_change_if
{
public:
	enum { ID=SC_BASILICA };

	sc_basilica(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_basilica()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_guildaura : public status_change_if
{
public:
	enum { ID=SC_GUILDAURA };

	sc_guildaura(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_guildaura()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_magicpower : public status_change_if
{
public:
	enum { ID=SC_MAGICPOWER };

	sc_magicpower(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_magicpower()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_edp : public status_change_if
{
public:
	enum { ID=SC_EDP };

	sc_edp(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_edp()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_truesight : public status_change_if
{
public:
	enum { ID=SC_TRUESIGHT };

	sc_truesight(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_truesight()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_windwalk : public status_change_if
{
public:
	enum { ID=SC_WINDWALK };

	sc_windwalk(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_windwalk()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_meltdown : public status_change_if
{
public:
	enum { ID=SC_MELTDOWN };

	sc_meltdown(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_meltdown()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cartboost : public status_change_if
{
public:
	enum { ID=SC_CARTBOOST };

	sc_cartboost(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_cartboost()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_chasewalk : public status_change_if
{
public:
	enum { ID=SC_CHASEWALK };

	sc_chasewalk(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_chasewalk()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_rejectsword : public status_change_if
{
public:
	enum { ID=SC_REJECTSWORD };

	sc_rejectsword(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_rejectsword()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_marionette : public status_change_if
{
public:
	enum { ID=SC_MARIONETTE };

	sc_marionette(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_marionette()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_marionette2 : public status_change_if
{
public:
	enum { ID=SC_MARIONETTE2 };

	sc_marionette2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_marionette2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_moonlit : public status_change_if
{
public:
	enum { ID=SC_MOONLIT };

	sc_moonlit(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_moonlit()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_headcrush : public status_change_if
{
public:
	enum { ID=SC_HEADCRUSH };

	sc_headcrush(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_headcrush()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_jointbeat : public status_change_if
{
public:
	enum { ID=SC_JOINTBEAT };

	sc_jointbeat(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_jointbeat()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_mindbreaker : public status_change_if
{
public:
	enum { ID=SC_MINDBREAKER };

	sc_mindbreaker(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_mindbreaker()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_memorize : public status_change_if
{
public:
	enum { ID=SC_MEMORIZE };

	sc_memorize(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_memorize()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_fogwall : public status_change_if
{
public:
	enum { ID=SC_FOGWALL };

	sc_fogwall(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_fogwall()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_spiderweb : public status_change_if
{
public:
	enum { ID=SC_SPIDERWEB };

	sc_spiderweb(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_spiderweb()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_devotion : public status_change_if
{
public:
	enum { ID=SC_DEVOTION };

	sc_devotion(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_devotion()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sacrifice : public status_change_if
{
public:
	enum { ID=SC_SACRIFICE };

	sc_sacrifice(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_sacrifice()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_steelbody : public status_change_if
{
public:
	enum { ID=SC_STEELBODY };

	sc_steelbody(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_steelbody()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readystorm : public status_change_if
{
public:
	enum { ID=SC_READYSTORM };

	sc_readystorm(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_readystorm()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stormkick : public status_change_if
{
public:
	enum { ID=SC_STORMKICK };

	sc_stormkick(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_stormkick()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readydown : public status_change_if
{
public:
	enum { ID=SC_READYDOWN };

	sc_readydown(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_readydown()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_downkick : public status_change_if
{
public:
	enum { ID=SC_DOWNKICK };

	sc_downkick(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_downkick()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readycounter : public status_change_if
{
public:
	enum { ID=SC_READYCOUNTER };

	sc_readycounter(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_readycounter()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_counter : public status_change_if
{
public:
	enum { ID=SC_COUNTER };

	sc_counter(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_counter()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readyturn : public status_change_if
{
public:
	enum { ID=SC_READYTURN };

	sc_readyturn(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_readyturn()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_turnkick : public status_change_if
{
public:
	enum { ID=SC_TURNKICK };

	sc_turnkick(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_turnkick()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dodge : public status_change_if
{
public:
	enum { ID=SC_DODGE };

	sc_dodge(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_dodge()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_run : public status_change_if
{
public:
	enum { ID=SC_RUN };

	sc_run(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_run()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_adrenaline2 : public status_change_if
{
public:
	enum { ID=SC_ADRENALINE2 };

	sc_adrenaline2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_adrenaline2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dancing : public status_change_if
{
public:
	enum { ID=SC_DANCING };

	sc_dancing(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_dancing()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_lullaby : public status_change_if
{
public:
	enum { ID=SC_LULLABY };

	sc_lullaby(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_lullaby()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_richmankim : public status_change_if
{
public:
	enum { ID=SC_RICHMANKIM };

	sc_richmankim(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_richmankim()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_eternalchaos : public status_change_if
{
public:
	enum { ID=SC_ETERNALCHAOS };

	sc_eternalchaos(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_eternalchaos()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_drumbattle : public status_change_if
{
public:
	enum { ID=SC_DRUMBATTLE };

	sc_drumbattle(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_drumbattle()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_nibelungen : public status_change_if
{
public:
	enum { ID=SC_NIBELUNGEN };

	sc_nibelungen(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_nibelungen()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_rokisweil : public status_change_if
{
public:
	enum { ID=SC_ROKISWEIL };

	sc_rokisweil(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_rokisweil()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_intoabyss : public status_change_if
{
public:
	enum { ID=SC_INTOABYSS };

	sc_intoabyss(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_intoabyss()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_siegfried : public status_change_if
{
public:
	enum { ID=SC_SIEGFRIED };

	sc_siegfried(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_siegfried()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dissonance : public status_change_if
{
public:
	enum { ID=SC_DISSONANCE };

	sc_dissonance(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_dissonance()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_whistle : public status_change_if
{
public:
	enum { ID=SC_WHISTLE };

	sc_whistle(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_whistle()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_assncros : public status_change_if
{
public:
	enum { ID=SC_ASSNCROS };

	sc_assncros(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_assncros()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_poembragi : public status_change_if
{
public:
	enum { ID=SC_POEMBRAGI };

	sc_poembragi(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_poembragi()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_appleidun : public status_change_if
{
public:
	enum { ID=SC_APPLEIDUN };

	sc_appleidun(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_appleidun()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_uglydance : public status_change_if
{
public:
	enum { ID=SC_UGLYDANCE };

	sc_uglydance(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_uglydance()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_humming : public status_change_if
{
public:
	enum { ID=SC_HUMMING };

	sc_humming(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_humming()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dontforgetme : public status_change_if
{
public:
	enum { ID=SC_DONTFORGETME };

	sc_dontforgetme(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_dontforgetme()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_fortune : public status_change_if
{
public:
	enum { ID=SC_FORTUNE };

	sc_fortune(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_fortune()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_service4u : public status_change_if
{
public:
	enum { ID=SC_SERVICE4U };

	sc_service4u(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_service4u()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incallstatus : public status_change_if
{
public:
	enum { ID=SC_INCALLSTATUS };

	sc_incallstatus(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incallstatus()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_inchit : public status_change_if
{
public:
	enum { ID=SC_INCHIT };

	sc_inchit(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_inchit()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incflee : public status_change_if
{
public:
	enum { ID=SC_INCFLEE };

	sc_incflee(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incflee()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incmhp2 : public status_change_if
{
public:
	enum { ID=SC_INCMHP2 };

	sc_incmhp2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incmhp2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incmsp2 : public status_change_if
{
public:
	enum { ID=SC_INCMSP2 };

	sc_incmsp2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incmsp2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incatk2 : public status_change_if
{
public:
	enum { ID=SC_INCATK2 };

	sc_incatk2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incatk2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incmatk2 : public status_change_if
{
public:
	enum { ID=SC_INCMATK2 };

	sc_incmatk2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incmatk2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_inchit2 : public status_change_if
{
public:
	enum { ID=SC_INCHIT2 };

	sc_inchit2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_inchit2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incflee2 : public status_change_if
{
public:
	enum { ID=SC_INCFLEE2 };

	sc_incflee2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incflee2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_preserve : public status_change_if
{
public:
	enum { ID=SC_PRESERVE };

	sc_preserve(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_preserve()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_battleorders : public status_change_if
{
public:
	enum { ID=SC_BATTLEORDERS };

	sc_battleorders(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_battleorders()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_regeneration : public status_change_if
{
public:
	enum { ID=SC_REGENERATION };

	sc_regeneration(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_regeneration()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_doublecast : public status_change_if
{
public:
	enum { ID=SC_DOUBLECAST };

	sc_doublecast(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_doublecast()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_gravitation : public status_change_if
{
public:
	enum { ID=SC_GRAVITATION };

	sc_gravitation(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_gravitation()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_maxoverthrust : public status_change_if
{
public:
	enum { ID=SC_MAXOVERTHRUST };

	sc_maxoverthrust(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_maxoverthrust()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_longing : public status_change_if
{
public:
	enum { ID=SC_LONGING };

	sc_longing(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_longing()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_hermode : public status_change_if
{
public:
	enum { ID=SC_HERMODE };

	sc_hermode(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_hermode()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_tarot : public status_change_if
{
public:
	enum { ID=SC_TAROT };

	sc_tarot(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_tarot()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incdef2 : public status_change_if
{
public:
	enum { ID=SC_INCDEF2 };

	sc_incdef2(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incdef2()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incstr : public status_change_if
{
public:
	enum { ID=SC_INCSTR };

	sc_incstr(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incstr()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incagi : public status_change_if
{
public:
	enum { ID=SC_INCAGI };

	sc_incagi(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incagi()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incvit : public status_change_if
{
public:
	enum { ID=SC_INCVIT };

	sc_incvit(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incvit()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incint : public status_change_if
{
public:
	enum { ID=SC_INCINT };

	sc_incint(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incint()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incdex : public status_change_if
{
public:
	enum { ID=SC_INCDEX };

	sc_incdex(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incdex()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incluk : public status_change_if
{
public:
	enum { ID=SC_INCLUK };

	sc_incluk(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_incluk()				{}
	virtual status_t status_id() const	{ return (status_t)ID; }
};






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// affectable implementation

///////////////////////////////////////////////////////////////////////////////
/// create a new status. or replace an existing
bool affectable::create_status(uint32 status_id)
{
	status_change_if* status=NULL;
	switch(status_id)
	{
	case sc_weight50::ID:	status = new sc_weight50(*this); break;
	case sc_weight90::ID:	status = new sc_weight90(*this); break;

	}
	if(status)
	{
		status_change_if*& ps = this->statusmap[status->status_id()];
		if( ps )
		{	// remove the previous status
			ps->deactivate();
			delete ps;
		}
		ps = status;
		status->activate();

		// restart all other status changes
		basics::smap<uint32, status_change_if*>::iterator iter(this->statusmap);
		for(; iter; ++iter)
		{
			if( iter->key != status_id)
				iter->data->restart();
		}
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// remove a status
bool affectable::remove_status(uint32 status_id)
{
	status_change_if** ps = this->statusmap.search(status_id);
	if( ps )
		remove_status(*ps);
	return true;
}
///////////////////////////////////////////////////////////////////////////////
/// remove a status
bool affectable::remove_status(status_change_if* status)
{
	if(status)
	{
		status->deactivate();
		this->statusmap.erase(status->status_id());
		delete status;
		// restart all changes
		basics::smap<uint32, status_change_if*>::iterator iter(this->statusmap);
		for(; iter; ++iter)
		{
			iter->data->restart();
		}
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// remove all status changes
bool affectable::remove_status()
{	// remove any remaining status change
	basics::smap<uint32, status_change_if*>::iterator iter(this->statusmap);
	for(; iter; ++iter)
	{
		iter->data->deactivate();
		delete iter->data;
	}
	statusmap.clear();
	return true;
}
