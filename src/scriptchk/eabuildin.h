// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EABUILDIN_
#define _EABUILDIN_


#include "basevariant.h"
#include "eaprogram.h"

///////////////////////////////////////////////////////////////////////////////
class CStackEngine;


///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin
{

	typedef basics::variant (*buildin_function)(CStackEngine& st);

	///////////////////////////////////////////////////////////////////////////
	/// 
	struct declaration
	{
		buildin_function	function;	// the buildin
		scriptdecl*			overload;	// the overload
		size_t				minparam;	// minimum number of parameter

		declaration() : function(NULL), overload(NULL), minparam(0)
		{}
	};


	///////////////////////////////////////////////////////////////////////////
	/// 
	buildin()
	{}
	~buildin()
	{}
public:
	static bool exists(const basics::string<>& name);
	static size_t parameter_count(const basics::string<>& name);
	static bool create(const basics::string<>& name, buildin_function f, size_t param=0);
	static bool create(const basics::string<>& name, scriptdecl* o);
	static bool erase(const basics::string<>& name);

private:
	static basics::smap<basics::string<>, declaration>	table;
};






///////////////////////////////////////////////////////////////////////////////
#endif//_EABUILDIN_
