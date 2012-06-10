#
# Tests is inline keywords or attributes are available:
#   HAVE_INLINE                   -  inline
#   HAVE___INLINE__               -  __inline__
#   HAVE___INLINE                 -  __inline
#   HAVE___FORCEINLINE            -  __forceinline
#   HAVE_ATTRIBUTE_NOINLINE       - __attribute__((noinline))
#   HAVE_ATTRIBUTE_ALWAYS_INLINE  - __attribute__((always_inline))
#   HAVE_ATTRIBUTE_GNU_INLINE     - __attribute__((gnu_inline))
#
#   INLINE_LIST - list of variables that evaluate to true
#
set( INLINE_LIST )
foreach( _keyword "inline" "__inline__" "__inline" "__forceinline" "__attribute__((always_inline))")
	set( _define "HAVE_${_keyword}" )
	string( TOUPPER "${_define}" _define )
	string( REGEX REPLACE "[^A-Z]" "_" _define "${_define}" )
	if( NOT DEFINED ${_define} )
		try_compile( ${_define} "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_LIST_DIR}/tests/try_inline.c"
			COMPILE_DEFINITIONS "-Dinline=${_keyword}" )
	endif()
	if( ${_define} )
		list( APPEND INLINE_LIST ${_define} )
	endif()
endforeach()
foreach( _attribute "noinline" "always_inline" "gnu_inline" )
	set( _define "HAVE_ATTRIBUTE_${_attribute}" )
	string( TOUPPER "${_define}" _define )
	string( REGEX REPLACE "[^A-Z]" "_" _define "${_define}" )
	if( NOT DEFINED ${_define} )
		try_compile( ${_define} "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_LIST_DIR}/tests/try_inline.c"
			COMPILE_DEFINITIONS "-Dinline=__attribute__((${_attribute}))" )
	endif()
	if( ${_define} )
		list( APPEND INLINE_LIST ${_define} )
	endif()
endforeach()
