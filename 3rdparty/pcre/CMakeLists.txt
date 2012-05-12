
#
# external_pcre
#
message( STATUS "Creating target external_pcre" )
set( _URL "${CMAKE_CURRENT_SOURCE_DIR}/pcre-8.30.tar.gz" )
set( _URL_MD5 "d5ee0d9f6d2f0b7489331d04b6c182ef" )
set( _INSTALL_DIR "${CMAKE_BINARY_DIR}/external/pcre" )
set( _CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR}"
	"-DPCRE_BUILD_PCRECPP=OFF"
	"-DPCRE_BUILD_PCREGREP=OFF"
	"-DPCRE_BUILD_TESTS=OFF"
	)

message( STATUS "Using URL=${_URL}" )
message( STATUS "Using URL_MD5=${_URL_MD5}" )
message( STATUS "Using INSTALL_DIR=${_INSTALL_DIR}" )
message( STATUS "Using CMAKE_ARGS=${_CMAKE_ARGS}" )
include(ExternalProject)
ExternalProject_Add( external_pcre
	URL "${_URL}"
	URL_MD5 "${_URL_MD5}"
	CMAKE_ARGS "${_CMAKE_ARGS}"
	BUILD_IN_SOURCE 1
)
set( TARGET_LIST ${TARGET_LIST} external_pcre  CACHE INTERNAL "" )
message( STATUS "Creating target external_pcre - done" )


#
# pcre
#
message( STATUS "Creating import static library pcre" )
add_library( pcre STATIC IMPORTED GLOBAL )
set_target_properties( pcre PROPERTIES
	IMPORTED_LOCATION "${_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}pcre${CMAKE_STATIC_LIBRARY_SUFFIX}" )
if( WIN32 )
	set_target_properties( pcre PROPERTIES
		IMPORTED_LOCATION_DEBUG "${_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}pcred${CMAKE_STATIC_LIBRARY_SUFFIX}" )
endif( WIN32 )
message( STATUS "Creating import static library pcre - done" )


#
# configure
#
set( WITH_PCRE ON
	CACHE BOOL "use PCRE" )
set( PCRE_DEPENDENCIES external_pcre
	CACHE STRING "PCRE dependencies" )
set( PCRE_LIBRARIES pcre
	CACHE PATH "PCRE libraries" )
set( PCRE_INCLUDE_DIRS "${_INSTALL_DIR}/include"
	CACHE PATH "PCRE include directories" )
set( PCRE_DEFINITIONS "-DPCRE_STATIC"
	CACHE STRING "PCRE definitions" )
mark_as_advanced( PCRE_DEPENDENCIES )
mark_as_advanced( PCRE_LIBRARIES )
mark_as_advanced( PCRE_INCLUDE_DIRS )
mark_as_advanced( PCRE_DEFINITIONS )
