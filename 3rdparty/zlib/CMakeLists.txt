
#
# external_zlib
#
message( STATUS "Creating target external_zlib" )
set( _URL "${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.7" )
set( _PATCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/Patch.cmake" )
set( _SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/source" )
set( _INSTALL_DIR "${CMAKE_BINARY_DIR}/external/zlib" )
set( _CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR}" )
set( _LIBRARY "${_INSTALL_DIR}/lib/zlibstatic.lib" )
set( _INCLUDE_DIR "${_INSTALL_DIR}/include" )
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/Patch.cmake.in" "${_PATCH_FILE}" @ONLY )

message( STATUS "Using URL=${_URL}" )
message( STATUS "Using LIBRARY=${_LIBRARY}" )
message( STATUS "Using INCLUDE_DIR=${_INCLUDE_DIR}" )
include(ExternalProject)
ExternalProject_Add( external_zlib
	URL "${_URL}"
	PATCH_COMMAND "${CMAKE_COMMAND};-P;${_PATCH_FILE}"
	SOURCE_DIR "${_SOURCE_DIR}"
	CMAKE_ARGS "${_CMAKE_ARGS}"
	BUILD_IN_SOURCE 1
)
set( TARGET_LIST ${TARGET_LIST} external_zlib  CACHE INTERNAL "" )
message( STATUS "Creating target external_zlib - done" )


#
# configure
#
set( WITH_ZLIB ON
	CACHE BOOL "use ZLIB" )
set( ZLIB_DEPENDENCIES external_zlib
	CACHE STRING "ZLIB dependencies" )
set( ZLIB_LIBRARIES "${_LIBRARY}"
	CACHE PATH "ZLIB libraries" )
set( ZLIB_INCLUDE_DIRS "${_INCLUDE_DIR}"
	CACHE PATH "ZLIB include directories" )
set( ZLIB_DEFINITIONS
	CACHE STRING "ZLIB definitions" )
mark_as_advanced( ZLIB_DEPENDENCIES )
mark_as_advanced( ZLIB_LIBRARIES )
mark_as_advanced( ZLIB_INCLUDE_DIRS )
mark_as_advanced( ZLIB_DEFINITIONS )
