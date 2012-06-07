
#
# system
#
message( STATUS "Detecting system ZLIB" )
unset( ZLIB_LIBRARIES CACHE )
unset( ZLIB_INCLUDE_DIRS CACHE )
find_package( ZLIB )
set( ZLIB_LIBRARIES_SYSTEM "${ZLIB_LIBRARIES}"
	CACHE PATH "system zlib libraries" )
set( ZLIB_INCLUDE_DIRS_SYSTEM "${ZLIB_INCLUDE_DIRS}"
	CACHE PATH "system zlib include directories" )
mark_as_advanced( ZLIB_LIBRARIES_SYSTEM )
mark_as_advanced( ZLIB_INCLUDE_DIRS_SYSTEM )

if( ZLIB_LIBRARIES_SYSTEM AND ZLIB_INCLUDE_DIRS_SYSTEM )
	set( HAVE_ZLIB_SYSTEM ON
		CACHE BOOL "zlib library is available on the system" )
	mark_as_advanced( HAVE_ZLIB_SYSTEM )
else()
	unset( HAVE_ZLIB_SYSTEM CACHE )
endif()
message( STATUS "Detecting system ZLIB - done" )

function( CONFIGURE_WITH_ZLIB_SYSTEM )
	if( INSTALL_COMPONENT_RUNTIME )
		string( REPLACE ".lib" ".dll" _dll "${ZLIB_LIBRARIES_SYSTEM}" )
		if( EXISTS "${_dll}" AND "${_dll}" MATCHES "^.*dll$" )
			install( FILES "${_dll}"
				DESTINATION "."
				COMPONENT Runtime_base )
		endif()
	endif( INSTALL_COMPONENT_RUNTIME )
endfunction()



#
# external_zlib
#
message( STATUS "Detecting external ZLIB" )
set( HAVE_ZLIB_EXTERNAL ON
	CACHE BOOL "zlib library is available as an external project" )
mark_as_advanced( HAVE_ZLIB_EXTERNAL )
message( STATUS "Detecting external ZLIB - done" )

function( CONFIGURE_WITH_ZLIB_EXTERNAL )
	message( STATUS "Creating target external_zlib" )
	set( _URL "${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.7" )
	set( _INSTALL_DIR "${CMAKE_BINARY_DIR}/external/zlib" )
	set( _CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR}" )
	set( _LIBRARY "${_INSTALL_DIR}/lib/zlibstatic.lib" )
	set( _INCLUDE_DIR "${_INSTALL_DIR}/include" )
	
	message( STATUS "Using URL: ${_URL}" )
	message( STATUS "Using LIBRARY: ${_LIBRARY}" )
	message( STATUS "Using INCLUDE_DIR: ${_INCLUDE_DIR}" )
	include(ExternalProject)
	ExternalProject_Add( external_zlib
		URL "${_URL}"
		CMAKE_ARGS "${_CMAKE_ARGS}"
		BUILD_IN_SOURCE 1
	)
	set_target_properties( external_zlib PROPERTIES EXCLUDE_FROM_ALL "TRUE" )
	set( TARGET_LIST ${TARGET_LIST} external_zlib  CACHE INTERNAL "" )
	set( ZLIB_DEPENDENCIES_EXTERNAL external_zlib
		CACHE STRING "external zlib dependencies" )
	set( ZLIB_LIBRARIES_EXTERNAL "${_LIBRARY}"
		CACHE PATH "external zlib libraries" )
	set( ZLIB_INCLUDE_DIRS_EXTERNAL "${_INSTALL_DIR}/include"
		CACHE PATH "external zlib include directories" )
	set( ZLIB_DEFINITIONS_EXTERNAL
		CACHE STRING "external zlib definitions" )
	mark_as_advanced( ZLIB_DEPENDENCIES_EXTERNAL )
	mark_as_advanced( ZLIB_LIBRARIES_EXTERNAL )
	mark_as_advanced( ZLIB_INCLUDE_DIRS_EXTERNAL )
	mark_as_advanced( ZLIB_DEFINITIONS_EXTERNAL )
	message( STATUS "Creating target external_zlib - done" )
endfunction()



#
# configure
#
CONFIGURE_WITH( ZLIB SYSTEM EXTERNAL )
