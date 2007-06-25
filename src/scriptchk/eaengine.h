// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EAENGINE_
#define _EAENGINE_

#include "basesync.h"
#include "basevariant.h"


#include "eaprogram.h"


///////////////////////////////////////////////////////////////////////////////
/// stack engine.
class CStackEngine : private basics::noncopyable
{
	ICL_EMPTY_COPYCONSTRUCTOR(CStackEngine)
public:
	typedef scriptprog::script						script;
	enum state_t
	{
		OFFLINE,
		RUNNING,
		PAUSED
	};
	enum limit_t
	{
		max_stack=1000,
		max_frame=1000
	};
	///////////////////////////////////////////////////////////////////////////
	/// parameters for buildin functions.
	/// provides access to the calling arguments and to engine control
	struct callparameter
	{
	public:
		mutable CStackEngine& engine;
	private:
		mutable basics::variant param_dummy;
		basics::variant* param_start;
		size_t param_cnt;
	public:
		callparameter(CStackEngine& e, basics::variant* start, size_t cnt)
			: engine(e), param_start(start), param_cnt(cnt)
		{}
		size_t size() const	{ return param_cnt; }
		basics::variant& operator[](size_t i)
		{
			return (i<param_cnt)?param_start[i]:param_dummy;
		}
		basics::variant& operator[](size_t i) const
		{
			return (i<param_cnt)?param_start[i]:param_dummy;
		}
	};
	///////////////////////////////////////////////////////////////////////////
	/// stackframe. contains all info of the current script process
	struct callframe
	{
		script							cProg;		///< programm
		scriptdecl						cDecl;		///< declaration
		basics::vector<basics::variant>	cStack;		///< stack
		basics::vector<basics::variant>	cTemp;		///< temp vars
		basics::vector<basics::variant>	cPara;		///< parameter vars
		size_t							cPC;		///< programm counter
		size_t							cCC;		///< stack counter

		basics::variant get_parameter(size_t i) const
		{
			if( i<this->cPara.size() )
				return this->cPara[i];
			return basics::variant();
		}
	};
	///////////////////////////////////////////////////////////////////////////
	/// host for external variables.
	struct global_variables
	{
		///////////////////////////////////////////////////////////////////////
		/// get a variable
		bool get_variable(const basics::string<>& name, basics::variant& target);
		basics::variant get_variable(const basics::string<>& name)
		{
			basics::variant ret;
			get_variable(name, ret);
			return ret;
		}
		///////////////////////////////////////////////////////////////////////
		/// set a variable
		bool set_variable(const basics::string<>& name, const basics::variant& target);
	};
private:
	///////////////////////////////////////////////////////////////////////////
	typedef basics::map<basics::string<>,basics::variant> external_map_t;
	state_t								cState;		///< engine state
	callframe*							pCall;		///< current call
	basics::vector<callframe*>			cCallStack;	///< the callframe
	basics::vector<callframe*>			cCallQueue;	///< the callqueue
	external_map_t						cExternal;	///temp storage for external variables
	basics::Mutex						cMtx;
	bool								cDebugMode;
public:
	static global_variables				sGlobalVariables;	///< Entry for external variables

public:
	///////////////////////////////////////////////////////////////////////////
	/// construct/destruct
	CStackEngine(bool dm=false) : cState(OFFLINE), pCall(NULL), cDebugMode(dm)
	{}
	~CStackEngine()
	{
		this->clear();
	}
	///////////////////////////////////////////////////////////////////////////
	/// start a programm
	bool start(const scriptprog::script& prog, const basics::string<>& startlabel="main");
	///////////////////////////////////////////////////////////////////////////
	/// coming back from a callback
	bool cont(const basics::variant& retvalue=basics::variant());

	bool is_running() const	{ return this->cState == RUNNING; }
	bool is_paused() const	{ return this->cState == PAUSED; }
	bool is_offline() const	{ return this->cState == OFFLINE; }

	///////////////////////////////////////////////////////////////////////////
	/// size of stackwalk.
	size_t get_frame_size() const
	{
		return this->pCall?1+this->cCallStack.size():0;
	}
	///////////////////////////////////////////////////////////////////////////
	/// stackwalk frame.
	const callframe* get_frame(size_t i) const
	{
		if( i==0 )
			return this->pCall;
		else if( i<=this->cCallStack.size() )
			return this->cCallStack[i-1];
		else
			return NULL;
	}
private:

	///////////////////////////////////////////////////////////////////////////
	/// select from array.
	void array_select(basics::variant& arr, const basics::variant* selectlist, int elems);
	///////////////////////////////////////////////////////////////////////////
	/// splice an array.
	void array_splice(basics::variant& arr, int start, int end, int ofs=1, int cnt=1);
	///////////////////////////////////////////////////////////////////////////
	/// process loop.
	state_t process();
	///////////////////////////////////////////////////////////////////////////
	/// get a variable.
	bool get_variable(const char* name, bool as_value);
	///////////////////////////////////////////////////////////////////////////
	/// check if can further increase stackframe.
	bool is_frame_limit() const;
	///////////////////////////////////////////////////////////////////////////
	/// run the currently selected script
	bool run_script();
	///////////////////////////////////////////////////////////////////////////
	/// gosub to a label inside the current script.
	bool call_gosub(size_t startpos);
	///////////////////////////////////////////////////////////////////////////
	/// calls a script function from the current script (function call)
	bool call_function(const basics::string<>& name, const basics::string<>& entry, const uint paramstart, const uint paramcnt);
	///////////////////////////////////////////////////////////////////////////
	/// calls a script function from the current script (function call)
	bool call_buildin(const basics::string<>& name, const uint paramstart, const uint paramcnt);
	///////////////////////////////////////////////////////////////////////////
	/// return from a script
	bool return_script(basics::variant retvalue);
	///////////////////////////////////////////////////////////////////////////
	/// clear the complete engine.
	void clear();
	///////////////////////////////////////////////////////////////////////////
	/// clear external variables.
	void clear_externals();
	///////////////////////////////////////////////////////////////////////////
	/// clear current callstack only.
	void clear_stack();


public:
	static void self_test(const char* name, bool debug);
};






///////////////////////////////////////////////////////////////////////////////
#endif//_EAENGINE_
