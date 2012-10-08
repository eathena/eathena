
#
# system
#
message( STATUS "Detecting system LUA" )
unset( LUA_LIBRARIES CACHE )
unset( LUA_INCLUDE_DIRS CACHE )
set( Lua_FIND_VERSION_MAJOR 5 )
set( Lua_FIND_VERSION_MINOR 2 )
find_package( LUA )
set( LUA_LIBRARIES_SYSTEM "${LUA_LIBRARIES}"
	CACHE PATH "system lua libraries" )
set( LUA_INCLUDE_DIRS_SYSTEM "${LUA_INCLUDE_DIRS}"
	CACHE PATH "system lua include directories" )
mark_as_advanced( LUA_LIBRARIES_SYSTEM )
mark_as_advanced( LUA_INCLUDE_DIRS_SYSTEM )

if( LUA_LIBRARIES_SYSTEM AND LUA_INCLUDE_DIRS_SYSTEM )
	set( HAVE_LUA_SYSTEM ON
		CACHE BOOL "lua library is available on the system" )
	mark_as_advanced( HAVE_LUA_SYSTEM )
else()
	unset( HAVE_LUA_SYSTEM CACHE )
endif()
message( STATUS "Detecting system LUA - done" )

function( CONFIGURE_WITH_LUA_SYSTEM )
	if( INSTALL_COMPONENT_RUNTIME )
		string( REPLACE ".lib" ".dll" _dll "${LUA_LIBRARIES_SYSTEM}" )
		if( EXISTS "${_dll}" AND "${_dll}" MATCHES "^.*dll$" )
			install( FILES "${_dll}"
				DESTINATION "."
				COMPONENT Runtime_base )
		endif()
	endif( INSTALL_COMPONENT_RUNTIME )
endfunction()



#
# external_lua
#
message( STATUS "Detecting external LUA" )
set( HAVE_LUA_EXTERNAL ON
	CACHE BOOL "lua library is available as an external project" )
mark_as_advanced( HAVE_LUA_EXTERNAL )
message( STATUS "Detecting external LUA - done" )

function( CONFIGURE_WITH_LUA_EXTERNAL )
	message( STATUS "Creating target external_lua" )
	set( _URL "${CMAKE_CURRENT_SOURCE_DIR}/lua-5.2.1-59da638" )
	set( _INSTALL_DIR "${CMAKE_BINARY_DIR}/external/lua" )
	set( _CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR}" "-DINSTALL_BIN=${_INSTALL_DIR}" "-DINSTALL_LIB=${_INSTALL_DIR}" )
	set( _SHARED_LIBRARY "${_INSTALL_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}lua${CMAKE_SHARED_LIBRARY_SUFFIX}" )
	set( _LINK_LIBRARY   "${_INSTALL_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}lua${CMAKE_LINK_LIBRARY_SUFFIX}" )
	if( "${CMAKE_LINK_LIBRARY_SUFFIX}" STREQUAL "" )
		set( _LINK_LIBRARY ${_SHARED_LIBRARY} )
	endif()
	set( _INCLUDE_DIR "${_INSTALL_DIR}/include" )

	message( STATUS "Using URL: ${_URL}" )
	message( STATUS "Using SHARED_LIBRARY: ${_SHARED_LIBRARY}" )
	message( STATUS "Using LINK_LIBRARY: ${_LINK_LIBRARY}" )
	message( STATUS "Using INCLUDE_DIR: ${_INCLUDE_DIR}" )
	include(ExternalProject)
	ExternalProject_Add( external_lua
		URL "${_URL}"
		CMAKE_ARGS "${_CMAKE_ARGS}"
		BUILD_IN_SOURCE 1
	)
	set_target_properties( external_lua PROPERTIES EXCLUDE_FROM_ALL "TRUE" )
	set( TARGET_LIST ${TARGET_LIST} external_lua  CACHE INTERNAL "" )
	set( LUA_DEPENDENCIES_EXTERNAL external_lua
		CACHE STRING "external lua dependencies" )
	set( LUA_LIBRARIES_EXTERNAL "${_LINK_LIBRARY}"
		CACHE PATH "external lua libraries" )
	set( LUA_INCLUDE_DIRS_EXTERNAL "${_INSTALL_DIR}/include"
		CACHE PATH "external lua include directories" )
	set( LUA_DEFINITIONS_EXTERNAL
		CACHE STRING "external lua definitions" )
	mark_as_advanced( LUA_DEPENDENCIES_EXTERNAL )
	mark_as_advanced( LUA_LIBRARIES_EXTERNAL )
	mark_as_advanced( LUA_INCLUDE_DIRS_EXTERNAL )
	mark_as_advanced( LUA_DEFINITIONS_EXTERNAL )
	message( STATUS "Creating target external_lua - done" )
	if( INSTALL_COMPONENT_RUNTIME )
		# install the shared library (dll) next to the executables
		install( FILES ${SHARED_LIBRARY}
			DESTINATION "."
			COMPONENT Runtime_base )
		# install the link library
		install( FILES ${_LINK_LIBRARY}
			DESTINATION "lib"
			COMPONENT Runtime_base
			OPTIONAL )
		# install the include headers
		install( DIRECTORY "${_INSTALL_DIR}/include/"
			DESTINATION "include"
			COMPONENT Runtime_base )
	endif( INSTALL_COMPONENT_RUNTIME )
endfunction()



#
# configure
#
CONFIGURE_WITH( LUA SYSTEM EXTERNAL )
