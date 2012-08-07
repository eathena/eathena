// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CONFIG_H_
#define _CONFIG_H_

// default configuration for Visual C++ 6 and later
//
// _MSC_VER version is compiler version in format XXYY, where
//     XX = major version
//     YY = minor version
//
// XXYY | Ver. | Product
// -----+------+---------------------------
// 1200 |  6.0 | Visual Studio 6
// 1300 |  7.0 | Visual Studio .NET
// 1310 |  7.1 | Visual Studio .NET 2003
// 1400 |  8.0 | Visual Studio 2005
// 1500 |  9.0 | Visual Studio 2008
// 1600 | 10.0 | Visual Studio 2010
// 1700 | 11.0 | Visual Studio 2012

// compiler keywords and attributes
#define inline __inline
/* #undef HAVE_INLINE */
/* #undef HAVE___INLINE__ */
#define HAVE___INLINE 1
#define HAVE___FORCEINLINE 1
/* #undef HAVE_ATTRIBUTE_NOINLINE */
/* #undef HAVE_ATTRIBUTE_ALWAYS_INLINE */
/* #undef HAVE_ATTRIBUTE_GNU_INLINE */

// headers
/* #undef STDC_HEADERS */
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
/* #undef HAVE_STRINGS_H */
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_EXECINFO_H */
/* #undef HAVE_NET_SOCKET_H */

// functions
/* #undef HAVE_SETRLIMIT */
/* #undef HAVE_STRNLEN */
/* #undef HAVE_GETPID */
/* #undef HAVE_GETTID */

// other
/* #undef SVNVERSION */
/* #undef PACKETVER */

#endif /* _CONFIG_H_ */
