
# macro to configure the use of several types of a package
# Uses for each type:
#	HAVE_${name}_${type} - is ${type} version available?
#	${name}_DEPENDENCIES_${type} - dependencies of the ${type} version
#	${name}_LIBRARIES_${type} - libraries of the ${type} version
#	${name}_INCLUDE_DIRS_${type} - include directories of the ${type} version
#	${name}_DEFINITIONS_${type} - definitions of the ${type} version
# Generates:
#	WITH_${name}_${type} - use the ${type} version of the package (only when ${type} is available)
#	WITH_${name} - use this package
#	${name}_DEPENDENCIES - dependencies
#	${name}_LIBRARIES - libraries
#	${name}_INCLUDE_DIRS - include directories
#	${name}_DEFINITIONS - definitions
macro( CONFIGURE_WITH name )
	unset( WITH_${name} CACHE )
	set( _count 0 )
	set( _selected )
	set( _default )
	foreach( _type IN ITEMS ${ARGN} )
		math( EXPR _count "${_count} + 1" )
		if( HAVE_${name}_${_type} )
			if( NOT _selected AND WITH_${name}_${_type} )
				set( _selected ${_type} )
			endif()
			set( WITH_${name}_${_type} ON
				CACHE BOOL "use ${_type} version of ${name} (alternative ${_count})" )
			if( NOT _default )
				set( _default ${_type} )
			endif()
		else()
			unset( WITH_${name}_${_type} CACHE )
		endif()
	endforeach()
	if( NOT _selected AND _default )
		set( _selected ${_default} )
	endif()
	foreach( _type IN ITEMS ${ARGN} )
		if( HAVE_${name}_${_type} AND NOT ("${_selected}" EQUAL "${_type}") )
			set_property( CACHE WITH_${name}_${_type} PROPERTY VALUE OFF )
		endif()
	endforeach()
	if( _selected )
		set_property( CACHE WITH_${name}_${_selected} PROPERTY VALUE ON )
		set( WITH_${name} ON
			CACHE BOOL "use ${name} (generated from all the WITH_MYSQL_<type>)" )
		unset( ${name}_DEPENDENCIES CACHE )
		unset( ${name}_DEPENDENCIES CACHE )
		unset( ${name}_LIBRARIES CACHE )
		unset( ${name}_INCLUDE_DIRS CACHE )
		unset( ${name}_DEFINITIONS CACHE )
		message( STATUS "Configuring for ${_selected} ${name}" )
		set( _file "${CMAKE_CURRENT_BINARY_DIR}/CONFIGURE_WITH_${name}_${_selected}.cmake" )
		file( WRITE "${_file}"
			"if( COMMAND CONFIGURE_WITH_${name}_${_selected} )\n"
			"	CONFIGURE_WITH_${name}_${_selected}()\n"
			"endif()\n"
		)
		include( ${_file} )
		unset( _file )
		set( ${name}_DEPENDENCIES ${${name}_DEPENDENCIES_${_selected}}
			CACHE PATH "${name} dependencies" )
		set( ${name}_LIBRARIES ${${name}_LIBRARIES_${_selected}}
			CACHE PATH "${name} libraries" )
		set( ${name}_INCLUDE_DIRS ${${name}_INCLUDE_DIRS_${_selected}}
			CACHE PATH "${name} include directories" )
		set( ${name}_DEFINITIONS ${${name}_DEFINITIONS_${_selected}}
			CACHE PATH "${name} definitions" )
		mark_as_advanced( ${name}_DEPENDENCIES )
		mark_as_advanced( ${name}_LIBRARIES )
		mark_as_advanced( ${name}_INCLUDE_DIRS )
		mark_as_advanced( ${name}_DEFINITIONS )
		message( STATUS "Configuring for ${_selected} ${name} - done" )
	endif()
	unset( _count )
	unset( _selected )
	unset( _default )
endmacro( CONFIGURE_WITH )

#add_subdirectory( msinttypes )# included in the root to setup global variables
add_subdirectory( mt19937ar )
add_subdirectory( mysql )
add_subdirectory( pcre )
add_subdirectory( zlib )
