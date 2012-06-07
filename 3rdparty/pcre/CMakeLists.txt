
#
# system
#
message( STATUS "Detecting system PCRE" )
unset( PCRE_LIBRARIES CACHE )
unset( PCRE_INCLUDE_DIRS CACHE )
find_package( PCRE )
set( PCRE_LIBRARIES_SYSTEM "${PCRE_LIBRARIES}"
	CACHE PATH "system pcre libraries" )
set( PCRE_INCLUDE_DIRS_SYSTEM "${PCRE_INCLUDE_DIRS}"
	CACHE PATH "system pcre include directories" )
mark_as_advanced( PCRE_LIBRARIES_SYSTEM )
mark_as_advanced( PCRE_INCLUDE_DIRS_SYSTEM )

if( PCRE_LIBRARIES_SYSTEM AND PCRE_INCLUDE_DIRS_SYSTEM )
	set( HAVE_PCRE_SYSTEM ON
		CACHE BOOL "pcre library is available on the system" )
	mark_as_advanced( HAVE_PCRE_SYSTEM )
else()
	unset( HAVE_PCRE_SYSTEM CACHE )
endif()
message( STATUS "Detecting system PCRE - done" )

function( CONFIGURE_WITH_PCRE_SYSTEM )
	if( INSTALL_COMPONENT_RUNTIME )
		string( REPLACE ".lib" ".dll" _dll "${PCRE_LIBRARIES_SYSTEM}" )
		if( EXISTS "${_dll}" AND "${_dll}" MATCHES "^.*dll$" )
			install( FILES "${_dll}"
				DESTINATION "."
				COMPONENT Runtime_base )
		endif()
	endif( INSTALL_COMPONENT_RUNTIME )
endfunction()



#
# external_pcre
#
message( STATUS "Detecting external PCRE" )
set( HAVE_PCRE_EXTERNAL ON
	CACHE BOOL "pcre library is available as an external project" )
mark_as_advanced( HAVE_PCRE_EXTERNAL )
message( STATUS "Detecting external PCRE - done" )

function( CONFIGURE_WITH_PCRE_EXTERNAL )
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
	set( PCRE_DEPENDENCIES_EXTERNAL external_pcre
		CACHE STRING "external pcre dependencies" )
	set( PCRE_LIBRARIES_EXTERNAL "${_LIBRARY}"
		CACHE PATH "external pcre libraries" )
	set( PCRE_INCLUDE_DIRS_EXTERNAL "${_INSTALL_DIR}/include"
		CACHE PATH "external pcre include directories" )
	set( PCRE_DEFINITIONS_EXTERNAL "-DPCRE_STATIC"
		CACHE STRING "external pcre definitions" )
	mark_as_advanced( PCRE_DEPENDENCIES_EXTERNAL )
	mark_as_advanced( PCRE_LIBRARIES_EXTERNAL )
	mark_as_advanced( PCRE_INCLUDE_DIRS_EXTERNAL )
	mark_as_advanced( PCRE_DEFINITIONS_EXTERNAL )
	message( STATUS "Creating target external_pcre - done" )
endfunction()



#
# configure
#
CONFIGURE_WITH( PCRE SYSTEM EXTERNAL )
