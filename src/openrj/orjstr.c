/* /////////////////////////////////////////////////////////////////////////////
 * File:    orjstr.c
 *
 * Purpose: Provides (English) string mappings for errors
 *
 * Created: 26th July 2004
 * Updated: 26th December 2006
 *
 * Home:    http://openrj.org/
 *
 * Copyright 2004-2005, Matthew Wilson and Synesis Software
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


/** \file orjstr.c Provides (English) string mappings for errors
 *
 */

/* /////////////////////////////////////////////////////////////////////////////
 * Version information
 */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
# define OPENRJ_VER_C_ORJMEM_MAJOR      1
# define OPENRJ_VER_C_ORJMEM_MINOR      3
# define OPENRJ_VER_C_ORJMEM_REVISION   1
# define OPENRJ_VER_C_ORJMEM_EDIT       11
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////////
 * Includes
 */

#include <openrj/openrj.h>
/* #include <openrj/openrj_assert.h> */
/* #include <openrj/openrj_memory.h> */

#include <stdlib.h> /* for size_t */

/* /////////////////////////////////////////////////////////////////////////////
 * Macros
 */

#ifndef NUM_ELEMENTS
# define NUM_ELEMENTS(x)    (sizeof(x) / sizeof((x)[0]))
#endif /* !NUM_ELEMENTS */

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef OPENRJ_DOCUMENTATION_SKIP_SECTION
struct ErrorString
#else /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */
typedef struct ErrorString  ErrorString;
struct ErrorString
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */
{
    int         errorCode;      /*!< The error code.    */
    char const  *errorString;   /*!< The string.        */
    size_t      len;            /*!< The string length. */
};



#define RC_STR_DECL(rc, desc)                                                           \
                                                                                        \
    static const char           s_str##rc[] =   desc;                                   \
    static const ErrorString    s_rct##rc = { rc, s_str##rc, NUM_ELEMENTS(s_str##rc) - 1 }


#define RC_STR_ENTRY(rc)                                                                \
                                                                                        \
    &s_rct##rc


static char const *ORJ_LookupCodeA_(int error, ErrorString const **mappings, size_t cMappings, size_t *len)
{
    /* Use Null Object (Variable) here for len, so do not need to check
     * elsewhere.
     */
    size_t  len_;

    if(NULL == len)
    {
        len = &len_;
    }

    { size_t i; for(i = 0; i < cMappings; ++i)
    {
        if(error == mappings[i]->errorCode)
        {
            return (*len = mappings[i]->len, mappings[i]->errorString);
        }
    }}

    return (*len = 0, "");
}

static char const *ORJ_LookupErrorStringA_(int error, size_t *len)
{
/*[OPENRJ:Errors-decl-start]*/
    RC_STR_DECL(ORJ_RC_SUCCESS              ,   "Operation was successful"                                   );
    RC_STR_DECL(ORJ_RC_CANNOTOPENJARFILE    ,   "The given file does not exist, or cannot be accessed"       );
    RC_STR_DECL(ORJ_RC_NORECORDS            ,   "The database file contained no records"                     );
    RC_STR_DECL(ORJ_RC_OUTOFMEMORY          ,   "The API suffered memory exhaustion"                         );
    RC_STR_DECL(ORJ_RC_BADFILEREAD          ,   "A read operation failed"                                    );
    RC_STR_DECL(ORJ_RC_PARSEERROR           ,   "Parsing of the database file failed due to a syntax error"  );
    RC_STR_DECL(ORJ_RC_INVALIDINDEX         ,   "An invalid index was specified"                             );
    RC_STR_DECL(ORJ_RC_UNEXPECTED           ,   "An unexpected condition was encountered"                    );
    RC_STR_DECL(ORJ_RC_INVALIDCONTENT       ,   "The database file contained invalid content"                );
/*[OPENRJ:Errors-decl-end]*/

    static const ErrorString    *s_strings[] = 
    {
/*[OPENRJ:Errors-entry-start]*/
        RC_STR_ENTRY(ORJ_RC_SUCCESS),
        RC_STR_ENTRY(ORJ_RC_CANNOTOPENJARFILE),
        RC_STR_ENTRY(ORJ_RC_NORECORDS),
        RC_STR_ENTRY(ORJ_RC_OUTOFMEMORY),
        RC_STR_ENTRY(ORJ_RC_BADFILEREAD),
        RC_STR_ENTRY(ORJ_RC_PARSEERROR),
        RC_STR_ENTRY(ORJ_RC_INVALIDINDEX),
        RC_STR_ENTRY(ORJ_RC_UNEXPECTED),
        RC_STR_ENTRY(ORJ_RC_INVALIDCONTENT),
/*[OPENRJ:Errors-entry-end]*/
    };

    return ORJ_LookupCodeA_(error, s_strings, NUM_ELEMENTS(s_strings), len);
}

static char const *ORJ_LookupParseErrorStringA_(int error, size_t *len)
{
/*[OPENRJ:ParseErrors-start]*/
    RC_STR_DECL(ORJ_PARSE_SUCCESS                       ,   "Parsing was successful"                                                            );
    RC_STR_DECL(ORJ_PARSE_RECORDSEPARATORINCONTINUATION ,   "A record separator was encountered during a content line continuation"             );
    RC_STR_DECL(ORJ_PARSE_UNFINISHEDLINE                ,   "The last line in the database was not terminated by a line-feed"                   );
    RC_STR_DECL(ORJ_PARSE_UNFINISHEDFIELD               ,   "The last field in the database file was not well-formed"                           );
    RC_STR_DECL(ORJ_PARSE_UNFINISHEDRECORD              ,   "The last record in the database file was not terminated by a record separator"     );
    RC_STR_DECL(ORJ_PARSE_INVALIDFIELDNAME              ,   "The field name was not valid"                                                      );
/*[OPENRJ:ParseErrors-end]*/

    static const ErrorString    *s_strings[] = 
    {
/*[OPENRJ:ParseErrors-entry-start]*/
        RC_STR_ENTRY(ORJ_PARSE_SUCCESS),
        RC_STR_ENTRY(ORJ_PARSE_RECORDSEPARATORINCONTINUATION),
        RC_STR_ENTRY(ORJ_PARSE_UNFINISHEDLINE),
        RC_STR_ENTRY(ORJ_PARSE_UNFINISHEDFIELD),
        RC_STR_ENTRY(ORJ_PARSE_UNFINISHEDRECORD),
        RC_STR_ENTRY(ORJ_PARSE_INVALIDFIELDNAME),
/*[OPENRJ:ParseErrors-entry-end]*/
    };

    return ORJ_LookupCodeA_(error, s_strings, NUM_ELEMENTS(s_strings), len);
}


ORJ_CALL(char const *) ORJ_GetErrorStringA( /* [in] */ ORJRC errorCode)
{
    return ORJ_LookupErrorStringA_((int)errorCode, NULL);
}

ORJ_CALL(size_t) ORJ_GetErrorStringLengthA( /* [in] */ ORJRC errorCode)
{
    size_t  len;

    return (ORJ_LookupErrorStringA_((int)errorCode, &len), len);
}


ORJ_CALL(char const *) ORJ_GetParseErrorStringA( /* [in] */ ORJ_PARSE_ERROR errorCode)
{
    return ORJ_LookupParseErrorStringA_((int)errorCode, NULL);
}

ORJ_CALL(size_t) ORJ_GetParseErrorStringLengthA( /* [in] */ ORJ_PARSE_ERROR errorCode)
{
    size_t  len;

    return (ORJ_LookupParseErrorStringA_(errorCode, &len), len);
}

/* ////////////////////////////////////////////////////////////////////////// */
