// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EAENGINE_
#define _EAENGINE_

#include "basesync.h"
#include "basevariant.h"


#include "eaprogram.h"


///////////////////////////////////////////////////////////////////////////////
struct objectmap : public basics::variant_host
{
	uint32  val;
	virtual ~objectmap()
	{}

	virtual bool get_member(const basics::string<>& name, basics::value_empty& target)
	{
		if( name=="val" )
			return basics::set_externmember(target, *this, &objectmap::val);
		return false;
	}
	virtual bool set_member(const basics::string<>& name, const basics::value_empty& target)
	{
		if( name=="val" )
			val = target.get_int();
		else
			return false;
		return true;
	}
};





///////////////////////////////////////////////////////////////////////////////
/// stack engine.
class CStackEngine : private basics::noncopyable
{
public:
	typedef scriptprog::script						script;
	enum state_t
	{
		OFFLINE,
		RUNNING,
		PAUSED
	};
private:
	///////////////////////////////////////////////////////////////////////////
	script								cProg;		///< the current programm
	scriptdecl							cDecl;		///< the current declaration
	basics::vector<basics::variant>		cStack;		///< the stack
	basics::vector<basics::variant>		cTemp;		///< temp variables
	basics::vector<basics::variant>		cPara;		///< parameter variables
	size_t								cPC;		///< Programm Counter
	size_t								cCC;		///< Stack Counter
	state_t								cState;		///< engine state

	void array_select(basics::variant& arr, const basics::variant* selectlist, int elems);
	void array_splice(basics::variant& arr, int start, int end, int ofs=1, int cnt=1);
	bool process();
public:
	///////////////////////////////////////////////////////////////////////////
	/// construct/destruct
	CStackEngine() : cPC(0), cCC(0), cState(OFFLINE)
	{}

	~CStackEngine()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// start a programm
	bool start(const scriptprog::script& prog, const basics::string<>& startlabel="main");
	///////////////////////////////////////////////////////////////////////////
	/// comming back from a callback
	bool cont(const basics::variant& retvalue);

	bool is_running() const	{ return this->cState == RUNNING; }
	bool is_paused() const	{ return this->cState == PAUSED; }
private:
	///////////////////////////////////////////////////////////////////////////
	/// run the currently selected script
	bool run_script();
	///////////////////////////////////////////////////////////////////////////
	/// calls a script from the current script (gosub)
	bool call_script(const char* name);
	///////////////////////////////////////////////////////////////////////////
	/// calls a script function from the current script (function call)
	bool call_function(const uint param);
	///////////////////////////////////////////////////////////////////////////
	/// return from a script
	bool return_script(const basics::variant& retvalue);
};






///////////////////////////////////////////////////////////////////////////////
#endif//_EAENGINE_
