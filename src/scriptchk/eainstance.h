// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EAINSTANCE_
#define _EAINSTANCE_


#include "basestring.h"
#include "eaprogram.h"




///////////////////////////////////////////////////////////////////////////////
/// script instance.
/// an instance is an object that carries a script together with it's properties
struct scriptinstance
{	
	typedef basics::TObjPtrCount<scriptinstance>			instance;
	typedef basics::smap<basics::string<>,basics::variant>	property;

	basics::string<>	cType;		///< instance type
	property			cProperty;	///< instance properties
	basics::string<>	cScript;	///< instance script
	basics::string<>	cStart;		///< instance start label

	scriptinstance()
	{}
	scriptinstance(const basics::string<>& s, const basics::string<>& p)
		: cScript(s), cStart(p)
	{}
};


///////////////////////////////////////////////////////////////////////////////
#endif//_EAINSTANCE_
