// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NULLPO_H_
#define _NULLPO_H_

#include "cbasetypes.h"

/// enabled by default on debug builds
#if defined(DEBUG) && !defined(NULLPO_CHECK)
	#define NULLPO_CHECK
#endif

#ifdef NULLPO_CHECK

#define NLP_MARK __FILE__, __LINE__, __func__

/// Returns 0 in case of a NULL pointer.
#define nullpo_ret(t)       if( nullpo_chk(NLP_MARK, (void *)(t), #t) ) { return(0); }

/// Returns from a 'void' function in case of a NULL pointer.
#define nullpo_retv(t)      if( nullpo_chk(NLP_MARK, (void *)(t), #t) ) { return; }

/// Returns given value in case of a NULL pointer.
#define nullpo_retr(ret, t) if( nullpo_chk(NLP_MARK, (void *)(t), #t) ) { return(ret); }

/// Breaks out of a loop/switch in case of a NULL pointer.
#define nullpo_retb(t)      if( nullpo_chk(NLP_MARK, (void *)(t), #t) ) { break; }

/// Checks for and reports NULL pointers.
/// Used by nullpo_ret* macros.
/// @param  file    set to __FILE__
/// @param  line    set to __LINE__
/// @param  func    set to __func__
/// @param  target  check target
/// @param  targetname  check target name
/// @return
///     0 = OK
///     1 = NULL
/// @note   Prefer using NLP_MARK for file, line, func.
int nullpo_chk(const char* file, int line, const char* func, const void* target, const char* targetname);

#else /* NULLPO_CHECK */

/// Disabled nullpo checks.
#define nullpo_ret(t)       (void)(t)
#define nullpo_retv(t)      (void)(t)
#define nullpo_retr(ret, t) (void)(t)
#define nullpo_retb(t)      (void)(t)

#endif /* NULLPO_CHECK */

#endif /* _NULLPO_H_ */
