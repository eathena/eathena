/* /////////////////////////////////////////////////////////////////////////////
 * File:    openrj/openrj_assert.h
 *
 * Purpose: Assertions for the Open-RJ C-API
 *
 * Created: 11th June 2004
 * Updated: 28th May 2006
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


/** \file openrj/openrj_assert.h
 *
 * \brief [C, C++] Assertions for the Open-RJ C-API
 */

#ifndef OPENRJ_INCL_OPENRJ_H_OPENRJ_ASSERT
#define OPENRJ_INCL_OPENRJ_H_OPENRJ_ASSERT

/* /////////////////////////////////////////////////////////////////////////////
 * Version information
 */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
# define OPENRJ_VER_OPENRJ_H_OPENRJ_ASSERT_MAJOR    1
# define OPENRJ_VER_OPENRJ_H_OPENRJ_ASSERT_MINOR    3
# define OPENRJ_VER_OPENRJ_H_OPENRJ_ASSERT_REVISION 1
# define OPENRJ_VER_OPENRJ_H_OPENRJ_ASSERT_EDIT     13
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////////
 * Includes
 */

#if defined(_MSC_VER)
# include <crtdbg.h>    /* for _ASSERTE() */
#else /* ? compiler */
# include <assert.h>    /* for assert() */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////////
 * Functions and macros
 */

/** \def OPENRJ_ASSERT(expr)
 *
 * Evaluates the expression, and aborts the program if it evaluates to zero/false
 *
 * \note Resolves to an underlying assertion macro, and therefore will be
 * inactive when the the underlying assertion macro is; usually in presence of
 * NDEBUG or absence of _DEBUG.
 */

#if defined(_MSC_VER)
# define OPENRJ_ASSERT(expr)    _ASSERTE(expr)
#else /* ? compiler */
# define OPENRJ_ASSERT(expr)    assert(expr)
#endif /* compiler */

/** \def openrj_assert(expr)
 *
 * Defines a compile-time assertion.
 *
 * \param expr Must be non-zero, or compilation will fail.
 *
 * \deprecated This is deprecated in favour of OPENRJ_ASSERT().
 *
 * \note This is a simple \#define for OPENRJ_ASSERT().
 */
#define openrj_assert(expr)         OPENRJ_ASSERT(expr)

/** \def OPENRJ_MESSAGE_ASSERT
 * \brief Assert macro for the Open-RJ API
 *
 * If the given expressions evaluates to false (0), the execution is halted and
 * an error message given.
 *
 * \param m The literal string describing the failed condition
 * \param x The expression that must evaluate to \c true
 */

#if defined(__WATCOMC__)
# define OPENRJ_MESSAGE_ASSERT(m, x)        OPENRJ_ASSERT(x)
#elif defined(__COMO__) || \
       defined(__GNUC__) || \
       defined(__MWERKS__)
# define OPENRJ_MESSAGE_ASSERT(m, x)        OPENRJ_ASSERT(((m) && (x)))
#else /* ? compiler */
# define OPENRJ_MESSAGE_ASSERT(m, x)        OPENRJ_ASSERT(((m), (x)))
#endif /* compiler */

/* ////////////////////////////////////////////////////////////////////////// */

#endif /* !OPENRJ_INCL_OPENRJ_H_OPENRJ_ASSERT */

/* ////////////////////////////////////////////////////////////////////////// */
