/* /////////////////////////////////////////////////////////////////////////////
 * File:    openrj/openrj_memory.h
 *
 * Purpose: Memory aspect of the Open-RJ C-API
 *
 * Created: 11th June 2004
 * Updated: 5th March 2006
 *
 * Home:    http://openrj.org/
 *
 * Copyright (c) 2004-2006, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the names of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////////// */


/** \file openrj/openrj_memory.h Memory aspect of the Open-RJ C-API
 *
 */

#ifndef OPENRJ_INCL_OPENRJ_H_OPENRJ_MEMORY
#define OPENRJ_INCL_OPENRJ_H_OPENRJ_MEMORY

/* /////////////////////////////////////////////////////////////////////////////
 * Version information
 */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
# define OPENRJ_VER_OPENRJ_H_OPENRJ_MEMORY_MAJOR    1
# define OPENRJ_VER_OPENRJ_H_OPENRJ_MEMORY_MINOR    0
# define OPENRJ_VER_OPENRJ_H_OPENRJ_MEMORY_REVISION 5
# define OPENRJ_VER_OPENRJ_H_OPENRJ_MEMORY_EDIT     9
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef OPENRJ_INCL_OPENRJ_H_OPENRJ
# error Must include openrj.h before openrj_memory.h. (Actually, you shouldn not really be including this anyway!)
#endif /* !OPENRJ_INCL_OPENRJ_H_OPENRJ */

#include <stdlib.h>

/* /////////////////////////////////////////////////////////////////////////////
 * Functions
 */

ORJ_CALL(void*) openrj_ator_alloc_(IORJAllocator *ator, size_t cb);

ORJ_CALL(void*) openrj_ator_realloc_(IORJAllocator *ator, void *pv, size_t cb);

ORJ_CALL(void) openrj_ator_free_(IORJAllocator *ator, void *pv);

/* ////////////////////////////////////////////////////////////////////////// */

#endif /* !OPENRJ_INCL_OPENRJ_H_OPENRJ_MEMORY */

/* ////////////////////////////////////////////////////////////////////////// */
