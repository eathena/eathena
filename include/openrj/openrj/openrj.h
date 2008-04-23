/* /////////////////////////////////////////////////////////////////////////////
 * File:    openrj/openrj.h
 *
 * Purpose: Root header file for the Open-RJ library
 *
 * Created: 11th June 2004
 * Updated: 30th April 2007
 *
 * Home:    http://openrj.org/
 *
 * Copyright (c) 2004-2007, Matthew Wilson and Synesis Software
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


/** \file openrj/openrj.h
 *
 * \brief [C, C++] This is the root file of the Open-RJ C-API.
 */

#ifndef OPENRJ_INCL_OPENRJ_H_OPENRJ
#define OPENRJ_INCL_OPENRJ_H_OPENRJ

/* /////////////////////////////////////////////////////////////////////////////
 * Version information
 */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
# define OPENRJ_VER_OPENRJ_H_OPENRJ_MAJOR       1
# define OPENRJ_VER_OPENRJ_H_OPENRJ_MINOR       24
# define OPENRJ_VER_OPENRJ_H_OPENRJ_REVISION    2
# define OPENRJ_VER_OPENRJ_H_OPENRJ_EDIT        60
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/** \def OPENRJ_VER_MAJOR
 * \brief The major version number of Open-RJ
 */

/** \def OPENRJ_VER_MINOR
 * \brief The minor version number of Open-RJ
 */

/** \def OPENRJ_VER_REVISION
 * \brief The revision version number of Open-RJ
 */

/** \def OPENRJ_VER
 * \brief The current composite version number of Open-RJ
 */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
# define OPENRJ_VER_1_0_1       0x01000100
# define OPENRJ_VER_1_1_1       0x01010100
# define OPENRJ_VER_1_1_2       0x01010200
# define OPENRJ_VER_1_2_1       0x01020100
# define OPENRJ_VER_1_2_2       0x01020200
# define OPENRJ_VER_1_3_1       0x01030100
# define OPENRJ_VER_1_3_2       0x01030200
# define OPENRJ_VER_1_3_3       0x01030300
# define OPENRJ_VER_1_3_4       0x01030400
# define OPENRJ_VER_1_4_1       0x01040100
# define OPENRJ_VER_1_5_1       0x01050100
# define OPENRJ_VER_1_5_2       0x01050200
# define OPENRJ_VER_1_5_3       0x01050300
# define OPENRJ_VER_1_5_4       0x01050400
# define OPENRJ_VER_1_6_1       0x01060100
# define OPENRJ_VER_1_6_2       0x01060200
# define OPENRJ_VER_1_6_3       0x01060300
# define OPENRJ_VER_1_6_4       0x01060400
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

#define OPENRJ_VER_MAJOR    1
#define OPENRJ_VER_MINOR    6
#define OPENRJ_VER_REVISION 4

#define OPENRJ_VER  OPENRJ_VER_1_6_4

/* /////////////////////////////////////////////////////////////////////////////
 * Includes
 */

#include <stddef.h>     /* for size_t */

/* /////////////////////////////////////////////////////////////////////////////
 * Documentation
 */

/* /////////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(__cplusplus) && \
    !defined(OPENRJ_DOCUMENTATION_SKIP_SECTION) && \
    !defined(ORJ_NO_NAMESPACE)
# define ORJ_NO_NAMESPACE
#endif /* __cplusplus, etc. */

#if !defined(ORJ_NO_NAMESPACE)
/** \brief The Open-RJ root namespace.
 *
 * This the the root namespace for Open-RJ, and contains the C-API functions, along
 * with enumerations and structures.
 */
namespace openrj
{
#endif /* !ORJ_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////////
 * Macros and symbols
 */

#if defined(_CH_)
# define OPENRJ_NO_STRUCT_TAG_PREFIX
#endif /* OPENRJ_DOCUMENTATION_SKIP_SECTION || _CH_ */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
# if defined(OPENRJ_NO_STRUCT_TAG_PREFIX)
#  define ORJ_TAG_NAME(x)                   x
# else /* ? OPENRJ_NO_STRUCT_TAG_PREFIX */
#  define ORJ_TAG_NAME(x)                   tag ## x
# endif /* OPENRJ_NO_STRUCT_TAG_PREFIX */
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/* OPENRJ_NO_STDIO */

/* OPENRJ_NO_FILE_HANDLING */

#if defined(OPENRJ_NO_FILE_HANDLING)
# define OPENRJ_NO_STDIO
#endif /* OPENRJ_NO_FILE_HANDLING */

/* /////////////////////////////////////////////////////////////////////////////
 * Ch
 */

#ifdef _CH_
# include <chdl.h>
LOAD_CHDL_CODE(openrjch, OpenRJ)
#endif /* _CH_ */

/* /////////////////////////////////////////////////////////////////////////////
 * Types
 */

/***************************************
 * Common types
 */

/** \name Common types
 *
 * \ingroup group_openrj
 */
/** @{ */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef unsigned char           orj_byte_t;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/** \brief Character type used by Open-RJ
 *
 * \note As of the current implementation, this is always defined to char
 */
typedef char                    orj_char_t;
/* typedef wchar_t           orj_char_t; */

#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION

# ifdef __cplusplus
#  define ORJ_EXTERN_C_         extern "C"
# else /* ? __cplusplus */
#  define ORJ_EXTERN_C_         extern
# endif /* !__cplusplus */
# define ORJ_CALL(rt)           ORJ_EXTERN_C_ rt

#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/** \brief The flags passed to the database creation functions
 */
enum ORJ_TAG_NAME(ORJ_FLAG)
{
        ORJ_FLAG_ORDERFIELDS                =   0x0001  /*!< Arranges the fields in alphabetical order */
    ,   ORJ_FLAG_ELIDEBLANKRECORDS          =   0x0002  /*!< Causes blank records to be ignored */
    ,   ORJ_FLAG_IGNORECASEONLOOKUP         =   0x0004  /*!< Ignores case when looking up field names. */
    ,   ORJ_FLAG_NOREINTERPRETFIELDIDS      =   0x0100  /*!< Suppresses Field Identifier Reinterpretation (see help file) */
    ,   ORJ_FLAG_FORCEALLFIELDSINTO1RECORD  =   0x0008  /*!< Causes record separators to be ignored, and all fields to be forced into a single record. */
#ifdef __cplusplus
    ,   ORDER_FIELDS                    =   ORJ_FLAG_ORDERFIELDS
    ,   ELIDE_BLANK_RECORDS             =   ORJ_FLAG_ELIDEBLANKRECORDS
    ,   IGNORE_CASE_ON_LOOKUP           =   ORJ_FLAG_IGNORECASEONLOOKUP
    ,   NO_REINTERPRET_FIELD_IDS        =   ORJ_FLAG_NOREINTERPRETFIELDIDS
    ,   FORCE_ALL_FIELDS_INTO_1_RECORD  =   ORJ_FLAG_FORCEALLFIELDSINTO1RECORD
#endif /* __cplusplus */
};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef enum ORJ_TAG_NAME(ORJ_FLAG)         ORJ_FLAG;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/** \brief The return code type of the Open-RJ API
 */
enum ORJ_TAG_NAME(ORJRC)
{
/*[OPENRJ:Errors-start]*/
        ORJ_RC_SUCCESS              =   0       /*!< Operation was successful                                   */
    ,   ORJ_RC_CANNOTOPENJARFILE                /*!< The given file does not exist, or cannot be accessed       */
    ,   ORJ_RC_NORECORDS                        /*!< The database file contained no records                     */
    ,   ORJ_RC_OUTOFMEMORY                      /*!< The API suffered memory exhaustion                         */
    ,   ORJ_RC_BADFILEREAD                      /*!< A read operation failed                                    */
    ,   ORJ_RC_PARSEERROR                       /*!< Parsing of the database file failed due to a syntax error  */
    ,   ORJ_RC_INVALIDINDEX                     /*!< An invalid index was specified                             */
    ,   ORJ_RC_UNEXPECTED                       /*!< An unexpected condition was encountered                    */
    ,   ORJ_RC_INVALIDCONTENT                   /*!< The database file contained invalid content                */
/*[OPENRJ:Errors-end]*/
};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef enum ORJ_TAG_NAME(ORJRC)            ORJRC;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/** \brief Parsing error constants
 */
enum ORJ_TAG_NAME(ORJ_PARSE_ERROR)
{
/*[OPENRJ:ParseErrors-start]*/
        ORJ_PARSE_SUCCESS           =   0       /*!< Parsing was successful                                                         */
    ,   ORJ_PARSE_RECORDSEPARATORINCONTINUATION /*!< A record separator was encountered during a content line continuation          */
    ,   ORJ_PARSE_UNFINISHEDLINE                /*!< The last line in the database was not terminated by a line-feed                */
    ,   ORJ_PARSE_UNFINISHEDFIELD               /*!< The last record in the database file was not terminated by a record separator  */
    ,   ORJ_PARSE_UNFINISHEDRECORD              /*!< The last record in the database file was not terminated by a record separator  */
    ,   ORJ_PARSE_INVALIDFIELDNAME              /*!< The field name was not valid                                                   */
/*[OPENRJ:ParseErrors-end]*/
};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef enum ORJ_TAG_NAME(ORJ_PARSE_ERROR)  ORJ_PARSE_ERROR;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */


/** \brief Allocator interface for Open-RJ
 */
struct ORJ_TAG_NAME(IORJAllocator)
{
    /** \brief Defines the "member" function for allocating memory */
    void *(*pfnAlloc)(struct ORJ_TAG_NAME(IORJAllocator) *ator, size_t cb);
    /** \brief Defines the "member" function for reallocating memory */
    void *(*pfnRealloc)(struct ORJ_TAG_NAME(IORJAllocator) *ator, void *pv, size_t cb);
    /** \brief Defines the "member" function for freeing memory */
    void (*pfnFree)(struct ORJ_TAG_NAME(IORJAllocator) *ator, void *pv);
};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef struct ORJ_TAG_NAME(IORJAllocator)  IORJAllocator;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/** \brief The string type (for ANSI coding)
 */
struct ORJ_TAG_NAME(ORJStringA)
{
    size_t      len;        /*!< \brief The number of characters in the string     */
    char const  *ptr;       /*!< \brief Pointer to the first element in the string */
};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef struct ORJ_TAG_NAME(ORJStringA)     ORJStringA;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

typedef ORJStringA                          ORJString;
/* typedef ORJStringW                         ORJString; */

/** \brief The field type (for ANSI coding)
 *
 * \note This structure definition is subject to change, and is not guaranteed
 * to be available in future versions. You are advised to use the Field
 * manipulation functions.
 */
struct ORJ_TAG_NAME(ORJFieldA)
{
    size_t      mbz0;       /*!< \brief Reserved: must be 0 */
    ORJStringA  name;       /*!< \brief The field name */
    ORJStringA  value;      /*!< \brief The field value */
    void        *reserved0; /*!< \brief Reserved: cannot be used by client code */

};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef struct ORJ_TAG_NAME(ORJFieldA)      ORJFieldA;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

typedef ORJFieldA                           ORJField;
/* typedef ORJFieldW                          ORJField; */

/** \brief The record type (for ANSI coding)
 *
 * \note This structure definition is subject to change, and is not guaranteed
 * to be available in future versions. You are advised to use the Record
 * manipulation functions.
 */

struct ORJ_TAG_NAME(ORJRecordA)
{
    size_t      mbz0;       /*!< \brief Reserved: must be 0 */
    size_t      numFields;  /*!< \brief The number of fields in the record */
    ORJFieldA   *fields;    /*!< \brief The field array */
    void        *reserved0; /*!< \brief Reserved: cannot be used by client code */
    ORJStringA  comment;    /*!< \brief The record comment */

};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef struct ORJ_TAG_NAME(ORJRecordA)     ORJRecordA;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

typedef ORJRecordA                          ORJRecord;
/* typedef ORJRecordW                         ORJRecord; */


/** \brief The database type (for ANSI coding)
 *
 * \note This structure definition is subject to change, and is not guaranteed
 * to be available in future versions. You are advised to use the Database
 * manipulation functions.
 */

struct ORJ_TAG_NAME(ORJDatabaseA)
{
    size_t          mbz0;       /*!< \brief Reserved: must be 0 */
    size_t          flags;      /*!< \brief Holds the flags passed to the function used to create the database */
    size_t          numLines;   /*!< \brief The number of lines in the database */
    size_t          numFields;  /*!< \brief The number of fields in the database */
    ORJFieldA       *fields;    /*!< \brief The record array */
    size_t          numRecords; /*!< \brief The number of records in the database */
    ORJRecordA      *records;   /*!< \brief The record array */
    IORJAllocator   *ator;      /*!< \brief The allocator */

};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef struct ORJ_TAG_NAME(ORJDatabaseA)   ORJDatabaseA;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

typedef ORJDatabaseA                        ORJDatabase;
/* typedef ORJDatabaseW                       ORJDatabase; */

/** \brief Parsing error structure
 */

struct ORJ_TAG_NAME(ORJError)
{
    size_t          reserved0;      /*!< \brief Reserved: must not be accessed                         */
    unsigned        invalidLine;    /*!< \brief The line on which the parsing error was encountered    */
    unsigned        invalidColumn;  /*!< \brief The column on which the parsing error was encountered  */
    ORJ_PARSE_ERROR parseError;     /*!< \brief The type of the parsing error                          */
};
#ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION
typedef struct ORJ_TAG_NAME(ORJError)       ORJError;
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

/** @} */

/* /////////////////////////////////////////////////////////////////////////////
 * API
 */

/***************************************
 * Database manipulation functions
 */

/** \name Database manipulation functions
 *
 * \ingroup group_openrj
 */
/** @{ */

#ifndef OPENRJ_NO_FILE_HANDLING
/** \brief Reads the records from the given file into an Open-RJ databsae
 *
 * \param jarName Name of the Record-JAR file. May not be NULL
 * \param ator The allocator to use for allocating memory, May be NULL, in which case CRT malloc() / realloc() / free() are used
 * \param flags Combination of the \link openrj::ORJ_FLAG ORJ_FLAG\endlink enumeration
 * \param error Pointer to an error structure, which is filled out with information if a parsing error is encountered
 * \param pdatabase Pointer to a database pointer, in which will be returned the database. May not be NULL. The returned pointer
 * must be freed using ORJ_FreeDatabaseA().
 */
ORJ_CALL(ORJRC) ORJ_ReadDatabaseA(  /* [in] */ char const           *jarName
                                ,   /* [in] */ IORJAllocator        *ator
                                ,   /* [in] */ unsigned             flags
                                ,   /* [out] */ ORJDatabaseA const  **pdatabase
                                ,   /* [out] */ ORJError            *error);
#endif /* !OPENRJ_NO_FILE_HANDLING */

/** \brief Reads the records from the memory block into an Open-RJ databsae
 *
 * \param contents Pointer to the base of the memory contents to parse. May not be NULL
 * \param cbContents Number of bytes in the memory contents to parse
 * \param ator The allocator to use for allocating memory, May be NULL, in which case CRT malloc() / realloc() / free() are used
 * \param flags Combination of the ORJ_FLAG enumeration
 * \param error Pointer to an error structure, which is filled out with information if a parsing error is encountered
 * \param pdatabase Pointer to a database pointer, in which will be returned the database. May not be NULL. The returned pointer
 * must be freed using ORJ_FreeDatabaseA().
 */
ORJ_CALL(ORJRC) ORJ_CreateDatabaseFromMemoryA(  /* [in] */ char const           *contents
                                            ,   /* [in] */ size_t               cbContents
                                            ,   /* [in] */ IORJAllocator        *ator
                                            ,   /* [in] */ unsigned             flags
                                            ,   /* [out] */ ORJDatabaseA const  **pdatabase
                                            ,   /* [out] */ ORJError            *error);


/** \brief Frees the resources associated with the database
 *
 * \param database The database. May not be NULL
 */
ORJ_CALL(ORJRC) ORJ_FreeDatabaseA(/* [in] */ ORJDatabaseA const     *database);


/** \brief Gives the number of lines in a database
 *
 * \param database The database. May not be NULL
 * \note The database is assumed valid. There is no error return
 */
ORJ_CALL(size_t) ORJ_Database_GetNumLinesA(/* [in] */ ORJDatabaseA const    *database);

/** \brief Gives the number of fields in a database
 *
 * \param database The database. May not be NULL
 * \note The database is assumed valid. There is no error return
 */
ORJ_CALL(size_t) ORJ_Database_GetNumFieldsA(/* [in] */ ORJDatabaseA const   *database);

/** \brief Gives the number of records in a database
 *
 * \param database The database. May not be NULL
 * \note The database is assumed valid. There is no error return
 */
ORJ_CALL(size_t) ORJ_Database_GetNumRecordsA(/* [in] */ ORJDatabaseA const  *database);

/** \brief Retrieves a record in the database
 *
 * \param database The database from which the record is to be retrieved
 * \param iRecord Index of the record to be retrieved
 * \param precord Pointer to a record pointer. The returned value points at the requested record stucture
 */
ORJ_CALL(ORJRC) ORJ_Database_GetRecordA(/* [in] */ ORJDatabaseA const   *database
                                    ,   /* [in] */ size_t               iRecord
                                    ,   /* [in] */ ORJRecordA const     **precord);

/** \brief Retrieves a field in the database
 *
 * \param database The database from which the field is to be retrieved
 * \param iField Index of the field to be retrieved
 * \param pfield Pointer to a field pointer. The returned value points at the requested field stucture
 */
ORJ_CALL(ORJRC) ORJ_Database_GetFieldA( /* [in] */ ORJDatabaseA const   *database
                                    ,   /* [in] */ size_t               iField
                                    ,   /* [in] */ ORJFieldA const      **pfield);


/** @} */

/***************************************
 * Record manipulation functions
 */

/** \name Record manipulation functions
 *
 * These functions may be used instead of direct manipulation of the data
 * structures, which may be necessary when mapping the API to other languages
 * which can only deal in opaque values and function calls (e.g. JNI).
 *
 * \ingroup group_openrj
 */
/** @{ */

/** \brief Gives the number of fields in a record
 *
 * \param record The record. May not be NULL
 * \note The record is assumed valid. There is no error return
 */
ORJ_CALL(size_t) ORJ_Record_GetNumFieldsA(  /* [in] */ ORJRecordA const *record);

/** \brief Gives the number of field in a record
 *
 * \param record The record from which to retrieve the field
 * \param iField Index of the field to be retrieved
 * \param pfield Pointer to a field pointer. The returned value points at the requested field stucture
 */
ORJ_CALL(ORJRC) ORJ_Record_GetFieldA(       /* [in] */ ORJRecordA const *record
                                        ,   /* [in] */ size_t           iField
                                        ,   /* [in] */ ORJFieldA const  **pfield);

/** \brief Finds a field in a record, based on the name and, optionally, the value
 *
 * \param record The record from which to retrieve the field
 * \param fieldName The name of the field
 * \param fieldValue The value of the field. May be NULL, in which case the first record with the given
 * name will be returned, irrespective of its value.
 */
ORJ_CALL(ORJFieldA const*) ORJ_Record_FindFieldByNameA( /* [in] */ ORJRecordA const *record
                                                    ,   /* [in] */ char const       *fieldName
                                                    ,   /* [in] */ char const       *fieldValue);

/** \brief Finds a field in a record, relative to a specified field, based on optional name and/or value
 *
 * \param record The record from which to retrieve the field
 * \param fieldAfter The field to start the search from. NULL to get the first field.
 * \param fieldName The name of the field. May be NULL for no filtering on name
 * \param fieldValue The value of the field. May be NULL for no filtering on value
 */
ORJ_CALL(ORJFieldA const*) ORJ_Record_FindNextFieldA(   /* [in] */ ORJRecordA const *record
                                                    ,   /* [in] */ ORJFieldA const  *fieldAfter /* = NULL */
                                                    ,   /* [in] */ char const       *fieldName  /* = NULL */
                                                    ,   /* [in] */ char const       *fieldValue /* = NULL */);

/** \brief Gives the database associated with the record
 *
 * \param record The record. May not be NULL
 * \note The record is assumed valid. There is no error return
 */
ORJ_CALL(ORJDatabaseA const*) ORJ_Record_GetDatabaseA(  /* [in] */ ORJRecordA const *record);


/** \brief Gives the record comment
 *
 * \param record The record from which to retrieve the field
 * \param pcomment Pointer to a comment pointer. The returned value points at the comment
 */
ORJ_CALL(ORJRC) ORJ_Record_GetCommentA(     /* [in] */ ORJRecordA const *record
                                        ,   /* [in] */ ORJStringA const **pcomment);


/** @} */

/***************************************
 * Field manipulation functions
 */

/** \name Field manipulation functions
 *
 * These functions may be used instead of direct manipulation of the data
 * structures, which may be necessary when mapping the API to other languages
 * which can only deal in opaque values and function calls (e.g. JNI).
 *
 * \ingroup group_openrj
 */
/** @{ */

/** \brief Gives the name of a field
 *
 * \param field The field whose name is to be retrieved
 * \param pname Pointer to a string pointer. The returned value points at a string
 * structure representing the name
 * \note In the current implementation, the field is assumed valid, and the return value
 * is always ORJ_RC_SUCCESS
 */
ORJ_CALL(ORJRC) ORJ_Field_GetNameA(         /* [in] */ ORJFieldA const  *field
                                        ,   /* [in] */ ORJStringA const **pname);

/** \brief Gives the value of a field
 *
 * \param field The field whose value is to be retrieved
 * \param pvalue Pointer to a string pointer. The returned value points at a string
 * structure representing the value
 * \note In the current implementation, the field is assumed valid, and the return value
 * is always ORJ_RC_SUCCESS
 */
ORJ_CALL(ORJRC) ORJ_Field_GetValueA(        /* [in] */ ORJFieldA const  *field
                                        ,   /* [in] */ ORJStringA const **pvalue);

/** \brief Gives the name and value of a field
 *
 * \param field The field whose value is to be retrieved
 * \param pname Pointer to a string pointer. The returned value points at a string
 * structure representing the name
 * \param pvalue Pointer to a string pointer. The returned value points at a string
 * structure representing the value
 * \note In the current implementation, the field is assumed valid, and the return value
 * is always ORJ_RC_SUCCESS
 */
ORJ_CALL(ORJRC) ORJ_Field_GetNameAndValueA( /* [in] */ ORJFieldA const  *field
                                        ,   /* [in] */ ORJStringA const **pname
                                        ,   /* [in] */ ORJStringA const **pvalue);

/** \brief Gives the record associated with the field
 *
 * \param field The field. May not be NULL
 * \note The record is assumed valid. There is no error return
 */
ORJ_CALL(ORJRecordA const*) ORJ_Field_GetRecordA(  /* [in] */ ORJFieldA const *field);

/** @} */


/***************************************
 * Error functions
 */

/** \name Error functions
 *
 * \ingroup group_openrj
 */
/** @{ */

/** \brief Gives the name of the error
 *
 * \param errorCode The error whose name is to be retrieved
 * \note If the error is not recognised, the function returns the empty string ("")
 */
ORJ_CALL(char const *) ORJ_GetErrorStringA( /* [in] */ ORJRC            errorCode);

/** \brief Gives the length of the error string returned by ORJ_GetErrorStringA()
 *
 * \param errorCode The error whose name is to be retrieved
 * \note If the error is not recognised, the function returns 0
 */
ORJ_CALL(size_t) ORJ_GetErrorStringLengthA( /* [in] */ ORJRC            errorCode);

/** \brief Gives the name of the parse-error
 *
 * \param errorCode The parse-error whose name is to be retrieved
 * \note If the error is not recognised, the function returns the empty string ("")
 */
ORJ_CALL(char const *) ORJ_GetParseErrorStringA( /* [in] */ ORJ_PARSE_ERROR errorCode);

/** \brief Gives the length of the error string returned by ORJ_GetParseErrorStringA()
 *
 * \param errorCode The error whose name is to be retrieved
 * \note If the error is not recognised, the function returns 0
 */
ORJ_CALL(size_t) ORJ_GetParseErrorStringLengthA( /* [in] */ ORJ_PARSE_ERROR errorCode);


/* \brief Formats an error string 
 *
 * Formats an error string, specifying information for error code and parse
 * error structure information.
 *
 * \param dest Pointer to a character buffer to receive the formatted results
 * \param cchDest Length of dest in characters
 * \param rc The Open-RJ result code
 * \param error The Open-RJ parsing error structure. Only consulted if 
 *   rc == ORJ_RC_PARSEERROR. May be NULL.
 * \param fmt Format string. May contain the special format specifier %E, which
 *   the formatted error string will replace. If the %E is not specified, then
 *   ": " and the formatted error string will be appended to the end of the
 *   string.
 *
 * \note Length of fmt string must be <= 768 characters
 *
 */

ORJ_CALL(int) ORJ_FormatErrorA( /* [in] */ char             *dest
                            ,   /* [in] */ size_t           cchDest
                            ,   /* [in] */ ORJRC            rc
                            ,   /* [in] */ ORJError const   *error  /* = NULL */
                            ,   /* [in] */ char const       *fmt
                            ,   /* [in] */ ...
                            );

/** @} */


# if !defined(OPENRJ_DOCUMENTATION_SKIP_SECTION) && \
     !defined(OPENRJ_PURE_API)
/***************************************
 * Implementation functions
 *
 * \note These are subject to change in a future version
 */

# ifndef __cplusplus
#  define ORJ_Database_GetNumLinesA(database)   ((size_t)(database)->numLines)
#  define ORJ_Database_GetNumFieldsA(database)  ((size_t)(database)->numFields)
#  define ORJ_Database_GetNumRecordsA(database) ((size_t)(database)->numRecords)
#  define ORJ_Record_GetNumFieldsA(record)      ((size_t)(record)->numFields)
#else /* ? __cplusplus */

#  define ORJ_Database_GetNumLinesA             ORJ_Database_GetNumLinesA_
inline size_t ORJ_Database_GetNumLinesA_(/* [in] */ ORJDatabaseA const *database)
{
    return database->numLines;
}

#  define ORJ_Database_GetNumFieldsA            ORJ_Database_GetNumFieldsA_
inline size_t ORJ_Database_GetNumFieldsA_(/* [in] */ ORJDatabaseA const *database)
{
    return database->numFields;
}
#  define ORJ_Database_GetNumRecordsA           ORJ_Database_GetNumRecordsA_
inline size_t ORJ_Database_GetNumRecordsA_(/* [in] */ ORJDatabaseA const *database)
{
    return database->numRecords;
}

#  define ORJ_Record_GetNumFieldsA              ORJ_Record_GetNumFieldsA_
inline size_t ORJ_Record_GetNumFieldsA_(/* [in] */ ORJRecordA const *record)
{
    return record->numFields;
}

# endif /* !__cplusplus */
#endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION && !OPENRJ_PURE_API */


#ifdef __cplusplus
# ifndef OPENRJ_NO_FILE_HANDLING
/** \brief Reads the records from the given file into an Open-RJ databsae
 *
 * \note An inline wrapper for ORJ_ReadDatabaseA()
 */
inline ORJRC ORJ_ReadDatabase(  /* [in] */ orj_char_t const     *jarName
                            ,   /* [in] */ IORJAllocator        *ator
                            ,   /* [in] */ unsigned             flags
                            ,   /* [out] */ ORJDatabase const   **pdatabase
                            ,   /* [out] */ ORJError            *error)
{
    return ORJ_ReadDatabaseA(jarName, ator, flags, pdatabase, error);
}
# endif /* !OPENRJ_NO_FILE_HANDLING */
/** \brief Reads the records from the memory block into an Open-RJ databsae
 *
 * \note An inline wrapper for ORJ_CreateDatabaseFromMemoryA()
 */
inline ORJRC ORJ_CreateDatabaseFromMemory(  /* [in] */ orj_char_t const     *contents
                                        ,   /* [in] */ size_t               cbContents
                                        ,   /* [in] */ IORJAllocator        *ator
                                        ,   /* [in] */ unsigned             flags
                                        ,   /* [out] */ ORJDatabaseA const  **pdatabase
                                        ,   /* [out] */ ORJError            *error)
{
    return ORJ_CreateDatabaseFromMemoryA(contents, cbContents, ator, flags, pdatabase, error);
}

/** \brief Frees the resources associated with the database
 *
 * \note An inline wrapper for ORJ_FreeDatabaseA()
 */
inline ORJRC ORJ_FreeDatabase(/* [in] */ ORJDatabase const *database)
{
    return ORJ_FreeDatabaseA(database);
}
#else /* ? __cplusplus */
# ifndef OPENRJ_NO_FILE_HANDLING
#  define ORJ_ReadDatabase                      ORJ_ReadDatabaseA
# endif /* !OPENRJ_NO_FILE_HANDLING */
# define ORJ_CreateDatabaseFromMemory           ORJ_CreateDatabaseFromMemoryA
# define ORJ_FreeDatabase                       ORJ_FreeDatabaseA
#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////////
 * C++ shorter name functions
 */

#ifdef __cplusplus

# ifndef OPENRJ_DOCUMENTATION_SKIP_SECTION

inline ORJRC ReadDatabase(  /* [in] */ char const           *jarName
                        ,   /* [in] */ IORJAllocator        *ator
                        ,   /* [in] */ unsigned             flags
                        ,   /* [out] */ ORJDatabaseA const  **pdatabase
                        ,   /* [out] */ ORJError            *error = NULL)
{
    return ORJ_ReadDatabaseA(jarName, ator, flags, pdatabase, error);
}

inline ORJRC ReadDatabase(  /* [in] */ char const           *jarName
                        ,   /* [in] */ unsigned             flags
                        ,   /* [out] */ ORJDatabaseA const  **pdatabase
                        ,   /* [out] */ ORJError            *error = NULL)
{
    return ORJ_ReadDatabaseA(jarName, NULL, flags, pdatabase, error);
}

inline ORJRC CreateDatabaseFromMemory(  /* [in] */ char const           *contents
                                    ,   /* [in] */ size_t               cbContents
                                    ,   /* [in] */ IORJAllocator        *ator
                                    ,   /* [in] */ unsigned             flags
                                    ,   /* [out] */ ORJDatabaseA const  **pdatabase
                                    ,   /* [out] */ ORJError            *error)
{
    return ORJ_CreateDatabaseFromMemoryA(contents, cbContents, ator, flags, pdatabase, error);
}

inline ORJRC FreeDatabase(/* [in] */ ORJDatabaseA const *database)
{
    return ORJ_FreeDatabaseA(database);
}

inline size_t Database_GetNumLines(/* [in] */ ORJDatabaseA const *database)
{
    return ORJ_Database_GetNumLinesA(database);
}

inline size_t Database_GetNumFields(/* [in] */ ORJDatabaseA const   *database)
{
    return ORJ_Database_GetNumFieldsA(database);
}

inline size_t Database_GetNumRecords(/* [in] */ ORJDatabaseA const *database)
{
    return ORJ_Database_GetNumRecordsA(database);
}

inline ORJRC Database_GetRecord(/* [in] */ ORJDatabaseA const   *database
                            ,   /* [in] */ size_t               iRecord
                            ,   /* [in] */ ORJRecordA const     **precord)
{
    return ORJ_Database_GetRecordA(database, iRecord, precord);
}

inline ORJRC Database_GetField( /* [in] */ ORJDatabaseA const   *database
                            ,   /* [in] */ size_t               iField
                            ,   /* [in] */ ORJFieldA const      **pfield)
{
    return ORJ_Database_GetFieldA(database, iField, pfield);
}

inline size_t Record_GetNumFields(/* [in] */ ORJRecordA const *record)
{
    return ORJ_Record_GetNumFieldsA(record);
}

inline ORJRC Record_GetField(   /* [in] */ ORJRecordA const *record
                            ,   /* [in] */ size_t           iField
                            ,   /* [in] */ ORJFieldA const  **pfield)
{
    return ORJ_Record_GetFieldA(record, iField, pfield);
}

inline ORJFieldA const* Record_FindFieldByName( /* [in] */ ORJRecordA const *record
                                            ,   /* [in] */ char const       *fieldName
                                            ,   /* [in] */ char const       *fieldValue)
{
    return ORJ_Record_FindFieldByNameA(record, fieldName, fieldValue);
}

inline ORJFieldA const* Record_FindNextField(   /* [in] */ ORJRecordA const *record
                                            ,   /* [in] */ ORJFieldA const  *fieldAfter = NULL
                                            ,   /* [in] */ char const       *fieldName  = NULL
                                            ,   /* [in] */ char const       *fieldValue = NULL)
{
    return ORJ_Record_FindNextFieldA(record, fieldAfter, fieldName, fieldValue);
}

inline ORJDatabaseA const* Record_GetDatabase(/* [in] */ ORJRecordA const *record)
{
    return ORJ_Record_GetDatabaseA(record);
}

inline ORJRC Record_GetComment( /* [in] */ ORJRecordA const *record
                            ,   /* [in] */ ORJStringA const **pcomment)
{
    return ORJ_Record_GetCommentA(record, pcomment);
}

inline ORJRC Field_GetName( /* [in] */ ORJFieldA const  *field
                        ,   /* [in] */ ORJStringA const **pname)
{
    return ORJ_Field_GetNameA(field, pname);
}

inline ORJRC Field_GetValue(/* [in] */ ORJFieldA const  *field
                        ,   /* [in] */ ORJStringA const **pvalue)
{
    return ORJ_Field_GetValueA(field, pvalue);
}

inline ORJRC Field_GetNameAndValue( /* [in] */ ORJFieldA const  *field
                                ,   /* [in] */ ORJStringA const **pname
                                ,   /* [in] */ ORJStringA const **pvalue)
{
    return ORJ_Field_GetNameAndValueA(field, pname, pvalue);
}

inline ORJRecordA const* Field_GetRecord(/* [in] */ ORJFieldA const *field)
{
    return ORJ_Field_GetRecordA(field);
}

inline char const *GetErrorString(/* [in] */ ORJRC errorCode)
{
    return ORJ_GetErrorStringA(errorCode);
}

inline size_t GetErrorStringLength(/* [in] */ ORJRC errorCode)
{
    return ORJ_GetErrorStringLengthA(errorCode);
}

inline char const *GetParseErrorString(/* [in] */ ORJ_PARSE_ERROR errorCode)
{
    return ORJ_GetParseErrorStringA(errorCode);
}

inline size_t GetParseErrorStringLength(/* [in] */ ORJ_PARSE_ERROR errorCode)
{
    return ORJ_GetParseErrorStringLengthA(errorCode);
}

#if 0
ORJ_CALL(int) ORJ_FormatErrorA( /* [in] */ char             *dest
                            ,   /* [in] */ size_t           cchDest
                            ,   /* [in] */ ORJRC            rc
                            ,   /* [in] */ ORJError const   *error  /* = NULL */
                            ,   /* [in] */ char const       *fmt
                            ,   /* [in] */ ...
                            );
#endif /* 0 */



# endif /* !OPENRJ_DOCUMENTATION_SKIP_SECTION */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifdef __cplusplus

/** \defgroup group_openrj_stringaccessshims String access shims
 *
 */

/** \name String access shims
 *
 * These functions are <a href = "http://www.cuj.com/documents/s=8681/cuj0308wilson/">string access shims</a>
 * for injection into the <b>stlsoft</b> namespace, in order to facilitate
 * generalised manipulation of the \link openrj::ORJStringA ORJStringA\endlink, ORJRC and ORJ_PARSE_ERROR types.
 * 
 * @{
 */

/** \brief Nul-terminated, possibly NULL, C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_null_a(ORJStringA const &s)
{
    return (0 != s.len) ? s.ptr : NULL;
}
/** \brief Nul-terminated, possibly NULL, C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_null(ORJStringA const &s)
{
    return c_str_ptr_null_a(s);
}

/** \brief Nul-terminated C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_a(ORJStringA const &s)
{
    return s.ptr;   /* This is ok, because Open-RJ guarantees that strings will be nul-terminated */
}
/** \brief Nul-terminated C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr(ORJStringA const &s)
{
    return c_str_ptr_a(s);
}

/** \brief Nul-terminated C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_data_a(ORJStringA const &s)
{
    return s.ptr;
}
/** \brief Nul-terminated C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_data(ORJStringA const &s)
{
    return c_str_data_a(s);
}

/** \brief Length of the C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline size_t c_str_len_a(ORJStringA const &s)
{
    return s.len;
}
/** \brief Length of the C-string representation of the given ORJStringA instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline size_t c_str_len(ORJStringA const &s)
{
    return s.len;
}

/** \brief Nul-terminated, possibly NULL, C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_null_a(ORJRC rc)
{
    char const *s   =   ORJ_GetErrorStringA(rc);

    return ('\0' != s) ? s : NULL;
}
/** \brief Nul-terminated, possibly NULL, C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_null(ORJRC rc)
{
    return c_str_ptr_null_a(rc);
}

/** \brief Nul-terminated C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_a(ORJRC rc)
{
    return ORJ_GetErrorStringA(rc);
}
/** \brief Nul-terminated C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr(ORJRC rc)
{
    return c_str_ptr_a(rc);
}

/** \brief Not necessarily nul-terminated C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_data_a(ORJRC rc)
{
    return ORJ_GetErrorStringA(rc);
}
/** \brief Not necessarily nul-terminated C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_data(ORJRC rc)
{
    return c_str_data_a(rc);
}

/** \brief Length of the C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline size_t c_str_len_a(ORJRC rc)
{
    return ORJ_GetErrorStringLengthA(rc);
}
/** \brief Length of the C-string representation of the given ORJRC instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline size_t c_str_len(ORJRC rc)
{
    return ORJ_GetErrorStringLengthA(rc);
}

/** \brief Nul-terminated, possibly NULL, C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_null_a(ORJ_PARSE_ERROR pe)
{
    char const *s   =   ORJ_GetParseErrorStringA(pe);

    return ('\0' != s) ? s : NULL;
}
/** \brief Nul-terminated, possibly NULL, C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_null(ORJ_PARSE_ERROR pe)
{
    return c_str_ptr_null_a(pe);
}

/** \brief Nul-terminated C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr_a(ORJ_PARSE_ERROR pe)
{
    return ORJ_GetParseErrorStringA(pe);
}
/** \brief Nul-terminated C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_ptr(ORJ_PARSE_ERROR pe)
{
    return c_str_ptr_a(pe);
}

/** \brief Not necessarily nul-terminated C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_data_a(ORJ_PARSE_ERROR pe)
{
    return ORJ_GetParseErrorStringA(pe);
}
/** \brief Not necessarily nul-terminated C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline char const *c_str_data(ORJ_PARSE_ERROR pe)
{
    return c_str_data_a(pe);
}

/** \brief Length of the C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline size_t c_str_len(ORJ_PARSE_ERROR pe)
{
    return ORJ_GetParseErrorStringLengthA(pe);
}
/** \brief Length of the C-string representation of the given ORJ_PARSE_ERROR instance
 *
 * \ingroup group_openrj_stringaccessshims
 */
inline size_t c_str_len_a(ORJ_PARSE_ERROR pe)
{
    return ORJ_GetParseErrorStringLengthA(pe);
}

/** @} String access shims */

/** \defgroup group_openrj_streaminsertionshimfunctiontemplates Stream insertion shim function templates
 *
 */

/** \name Stream insertion shims
 *
 * These functions are <a href = "http://www.cuj.com/documents/s=8681/cuj0308wilson/">stream insertion shims</a>
 * for allowing the ORJStringA, ORJRC and ORJ_PARSE_ERROR types to be used
 * with stream instances, a la the IOStreams.
 *
 * For example:
 *
 * \htmlonly
 * <pre>
 *
 * ORJRecord *record = . . . ;
 *
 * std::cout << record->fields[0] << std::end; // Prints the value of the 0th field in the record
 *
 * ORJRC rc = ORJ_RC_INVALIDINDEX;
 *
 * std::cout << rc << std::end; // Prints "An invalid index was specified"
 *
 * ORJ_PARSE_ERROR pe = ORJ_PARSE_UNFINISHEDLINE;
 *
 * std::cout << pe << std::end; // Prints "The last line in the database was not terminated by a line-feed"
 *
 * </pre>
 * \endhtmlonly
 * 
 * @{
 */

/** \brief Inserts an ORJStringA instance into a stream
 *
 * \ingroup group_openrj_streaminsertionshimfunctiontemplates
 *
 * \note stm may be of any type that supports a write(char const *str, size_t len); method
 */
template <class S>
inline S &operator <<(S &stm, ORJStringA const &s)
{
    stm.write(s.ptr, s.len);

    return stm;
}

/** \brief Inserts an ORJRC instance into a stream
 *
 * \ingroup group_openrj_streaminsertionshimfunctiontemplates
 *
 * \note stm may be of any type that supports an insertion operator taking char const*
 */
template <class S>
inline S &operator <<(S &stm, ORJRC rc)
{
    return stm << c_str_ptr(rc);
}

/** \brief Inserts an ORJ_PARSE_ERROR instance into a stream
 *
 * \ingroup group_openrj_streaminsertionshimfunctiontemplates
 *
 * \note stm may be of any type that supports an insertion operator taking char const*
 */
template <class S>
inline S &operator <<(S &stm, ORJ_PARSE_ERROR pe)
{
    return stm << c_str_ptr(pe);
}

/** @} String insertion shim function templates */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(ORJ_NO_NAMESPACE)
} /* namespace openrj */

#ifdef OPENRJ_DOCUMENTATION_SKIP_SECTION
/** \brief The Open-RJ project inserts c_str_ptr, c_str_data and c_str_len 
 * <a href = "http://www.cuj.com/documents/s=8681/cuj0308wilson/">string access shims</a> 
 * into the STLSoft namespace, for generalised manipulation of its
 * \link openrj::ORJStringA ORJStringA\endlink,
 * \link openrj::ORJRC ORJRC\endlink
 * and
 * \link openrj::ORJ_PARSE_ERROR ORJ_PARSE_ERROR\endlink
 * types.
 */
#endif /* OPENRJ_DOCUMENTATION_SKIP_SECTION */
namespace stlsoft
{
    using ::openrj::c_str_ptr_null_a;
    using ::openrj::c_str_ptr_null;
    using ::openrj::c_str_ptr_a;
    using ::openrj::c_str_ptr;
    using ::openrj::c_str_data_a;
    using ::openrj::c_str_data;
    using ::openrj::c_str_len_a;
    using ::openrj::c_str_len;
}

#endif /* !ORJ_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////////// */

#endif /* !OPENRJ_INCL_OPENRJ_H_OPENRJ */

/* ////////////////////////////////////////////////////////////////////////// */
