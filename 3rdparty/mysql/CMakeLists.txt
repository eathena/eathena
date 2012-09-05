
macro( MYSQL_TRY_COMPILE _libs_var _includes_var _result_Var )
	unset( ${_result_Var} )
	if( ${_libs_var} AND ${_includes_var} )
		set( CMAKE_REQUIRED_LIBRARIES ${GLOBAL_LIBRARIES} ${${_libs_var}} )
		set( CMAKE_REQUIRED_INCLUDES ${GLOBAL_INCLUDE_DIRS} ${${_includes_var}} )
		set( CMAKE_REQUIRED_DEFINITIONS ${GLOBAL_DEFINITIONS} )
		include( CheckIncludeFile )
		CHECK_INCLUDE_FILE( windows.h HAVE_WINDOWS_H )
		if( HAVE_WINDOWS_H )
			list( APPEND CMAKE_REQUIRED_DEFINITIONS "-DHAVE_WINDOWS_H" )
		endif()
		include( CheckCSourceCompiles )
		file( READ "${CMAKE_SOURCE_DIR}/3rdparty/cmake/tests/MYSQL_TRY_RUN.c" _SOURCE )
		CHECK_C_SOURCE_COMPILES( "${_SOURCE}" ${_result_Var} )
		unset( CMAKE_REQUIRED_LIBRARIES )
		unset( CMAKE_REQUIRED_INCLUDES )
		unset( CMAKE_REQUIRED_DEFINITIONS )
	endif()
endmacro()


#
# system
#
message( STATUS "Detecting system MYSQL" )
unset( MYSQL_LIBRARIES CACHE )
unset( MYSQL_INCLUDE_DIRS CACHE )
find_package( MYSQL )
MYSQL_TRY_COMPILE( MYSQL_LIBRARIES  MYSQL_INCLUDE_DIRS  MYSQL_TRY_COMPILE_SYSTEM )
set( MYSQL_LIBRARIES_SYSTEM "${MYSQL_LIBRARIES}"
	CACHE PATH "system mysql libraries" )
set( MYSQL_INCLUDE_DIRS_SYSTEM "${MYSQL_INCLUDE_DIRS}"
	CACHE PATH "system mysql include directories" )
mark_as_advanced( MYSQL_LIBRARIES_SYSTEM )
mark_as_advanced( MYSQL_INCLUDE_DIRS_SYSTEM )
mark_as_advanced( MYSQL_TRY_COMPILE_SYSTEM )

if( MYSQL_TRY_COMPILE_SYSTEM )
	set( HAVE_MYSQL_SYSTEM ON
		CACHE BOOL "mysql client is available on the system" )
	mark_as_advanced( HAVE_MYSQL_SYSTEM )
else()
	unset( HAVE_MYSQL_SYSTEM CACHE )
endif()
message( STATUS "Detecting system MYSQL - done" )

function( CONFIGURE_WITH_MYSQL_SYSTEM )
	if( INSTALL_COMPONENT_RUNTIME )
		string( REPLACE ".lib" ".dll" _dll "${MYSQL_LIBRARIES_SYSTEM}" )
		if( EXISTS "${_dll}" AND "${_dll}" MATCHES "^.*dll$" )
			install( FILES "${_dll}"
				DESTINATION "."
				COMPONENT Runtime_base )
		endif()
	endif( INSTALL_COMPONENT_RUNTIME )
endfunction()



#
# local
#
message( STATUS "Detecting local MYSQL" )
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set( MYSQL_LOCAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/winx64" CACHE INTERNAL "" )
elseif( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	set( MYSQL_LOCAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/win32" CACHE INTERNAL "" )
endif()
set( _LIBRARY "${MYSQL_LOCAL_PATH}/lib/libmysql.dll" )
set( _LIBRARY_IMPLIB "${MYSQL_LOCAL_PATH}/lib/libmysql.lib" )
set( _INCLUDE_DIR "${MYSQL_LOCAL_PATH}/include" )
message( STATUS "Using LIBRARY: ${_LIBRARY_IMPLIB}" )
message( STATUS "Using INCLUDE_DIR: ${_INCLUDE_DIR}" )
if( EXISTS "${_INCLUDE_DIR}/mysql_version.h" )
	file( STRINGS "${_INCLUDE_DIR}/mysql_version.h" _version REGEX "^#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE "^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" _version "${_version}" )
	message( STATUS "Found version: ${_version}" )
endif()
add_library( local_libmysql SHARED IMPORTED GLOBAL )
set_property( TARGET local_libmysql PROPERTY IMPORTED_LOCATION "${_LIBRARY}" )
set_property( TARGET local_libmysql PROPERTY IMPORTED_IMPLIB "${_LIBRARY_IMPLIB}" )
MYSQL_TRY_COMPILE( _LIBRARY_IMPLIB  _INCLUDE_DIR  MYSQL_TRY_COMPILE_LOCAL )
set( MYSQL_LIBRARIES_LOCAL local_libmysql
	CACHE PATH "local mysql libraries" )
set( MYSQL_INCLUDE_DIRS_LOCAL "${_INCLUDE_DIR}"
	CACHE PATH "local mysql include directories" )
mark_as_advanced( MYSQL_LIBRARIES_LOCAL )
mark_as_advanced( MYSQL_INCLUDE_DIRS_LOCAL )
mark_as_advanced( MYSQL_TRY_COMPILE_LOCAL )

if( MYSQL_TRY_COMPILE_LOCAL )
	set( HAVE_MYSQL_LOCAL ON
		CACHE BOOL "mysql client is available as a local library" )
	mark_as_advanced( HAVE_MYSQL_LOCAL )
else()
	unset( HAVE_MYSQL_LOCAL CACHE )
endif()
message( STATUS "Detecting local MYSQL - done" )

function( CONFIGURE_WITH_MYSQL_LOCAL )
	if( INSTALL_COMPONENT_RUNTIME )
		get_property( _dll TARGET local_libmysql PROPERTY IMPORTED_LOCATION )
		install( FILES "${_dll}"
			DESTINATION "."
			COMPONENT Runtime_base )
	endif( INSTALL_COMPONENT_RUNTIME )
endfunction()



#
# external_mysql
#
set( HAVE_MYSQL_EXTERNAL ON
	CACHE BOOL "mysql client is available as an external project" )
mark_as_advanced( HAVE_MYSQL_EXTERNAL )

function( CONFIGURE_WITH_MYSQL_EXTERNAL )
	# external_mysql
	message( STATUS "Detecting external MYSQL" )
	find_package( MYSQL_SOURCE_MIRROR REQUIRED )
	message( STATUS "Detecting external MYSQL - done" )
	message( STATUS "Creating target external_mysql" )
	set( _URL "${MYSQL_SOURCE_MIRROR_URL}" )
	set( _URL_MD5 "${MYSQL_SOURCE_MIRROR_MD5}" )
	set( _INSTALL_DIR "${CMAKE_BINARY_DIR}/external/mysql" )
	set( _CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR}"
		"-DDISABLE_SHARED=ON"
		"-DWITHOUT_SERVER=ON"
		"-DWITH_UNIT_TESTS=OFF"
	)
	set( _LIBRARY "${_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mysqlclient${CMAKE_STATIC_LIBRARY_SUFFIX}" )
	set( _INCLUDE_DIR "${_INSTALL_DIR}/include" )
	
	message( STATUS "Using URL: ${_URL}" )
	message( STATUS "Using LIBRARY: ${_LIBRARY}" )
	message( STATUS "Using INCLUDE_DIR: ${_INCLUDE_DIR}" )
	include(ExternalProject)
	ExternalProject_Add( external_mysql
		URL "${_URL}"
		URL_MD5 "${_URL_MD5}"
		CMAKE_ARGS "${_CMAKE_ARGS}"
	)
	set_target_properties( external_mysql PROPERTIES EXCLUDE_FROM_ALL "TRUE" )
	set( TARGET_LIST ${TARGET_LIST} external_mysql  CACHE INTERNAL "" )
	set( MYSQL_DEPENDENCIES_EXTERNAL external_mysql
		CACHE STRING "external mysql dependencies" )
	set( MYSQL_LIBRARIES_EXTERNAL "${_LIBRARY}"
		CACHE PATH "external mysql libraries" )
	set( MYSQL_INCLUDE_DIRS_EXTERNAL "${_INSTALL_DIR}/include"
		CACHE PATH "external mysql include directories" )
	set( MYSQL_DEFINITIONS_EXTERNAL
		CACHE STRING "external mysql definitions" )
	mark_as_advanced( MYSQL_DEPENDENCIES_EXTERNAL )
	mark_as_advanced( MYSQL_LIBRARIES_EXTERNAL )
	mark_as_advanced( MYSQL_INCLUDE_DIRS_EXTERNAL )
	mark_as_advanced( MYSQL_DEFINITIONS_EXTERNAL )
	message( STATUS "Creating target external_mysql - done" )
endfunction()



#
# configure
#
CONFIGURE_WITH( MYSQL SYSTEM LOCAL EXTERNAL )
