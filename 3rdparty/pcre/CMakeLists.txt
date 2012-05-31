
#
# external_pcre
#
message( STATUS "Creating target external_pcre" )
set( _URL "${CMAKE_CURRENT_SOURCE_DIR}/pcre-8.30" )
set( _INSTALL_DIR "${CMAKE_BINARY_DIR}/external/pcre" )
set( _CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR}" )
set( _LIBRARY "${_INSTALL_DIR}/lib/pcre.lib" )
set( _INCLUDE_DIR "${_INSTALL_DIR}/include" )

message( STATUS "Using URL: ${_URL}" )
message( STATUS "Using LIBRARY: ${_LIBRARY}" )
message( STATUS "Using INCLUDE_DIR: ${_INCLUDE_DIR}" )
include(ExternalProject)
ExternalProject_Add( external_pcre
	URL "${_URL}"
	CMAKE_ARGS "${_CMAKE_ARGS}"
	BUILD_IN_SOURCE 1
)
set_target_properties( external_pcre PROPERTIES EXCLUDE_FROM_ALL "TRUE" )
set( TARGET_LIST ${TARGET_LIST} external_pcre  CACHE INTERNAL "" )
message( STATUS "Creating target external_pcre - done" )


#
# configure
#
set( WITH_PCRE ON
	CACHE BOOL "use PCRE" )
set( PCRE_DEPENDENCIES external_pcre
	CACHE STRING "PCRE dependencies" )
set( PCRE_LIBRARIES "${_LIBRARY}"
	CACHE PATH "PCRE libraries" )
set( PCRE_INCLUDE_DIRS "${_INCLUDE_DIR}"
	CACHE PATH "PCRE include directories" )
set( PCRE_DEFINITIONS "-DPCRE_STATIC"
	CACHE STRING "PCRE definitions" )
mark_as_advanced( PCRE_DEPENDENCIES )
mark_as_advanced( PCRE_LIBRARIES )
mark_as_advanced( PCRE_INCLUDE_DIRS )
mark_as_advanced( PCRE_DEFINITIONS )
