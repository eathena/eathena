
#
# mapcache
#
if( WITH_ZLIB )
	option( BUILD_MAPCACHE "build mapcache executable" ON )
else()
	message( STATUS "Disabled mapcache target (required ZLIB)" )
endif()
if( BUILD_MAPCACHE )
message( STATUS "Creating target mapcache" )
set( MAPCACHE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/mapcache.c"
	)
set( DEPENDENCIES ${ZLIB_DEPENDENCIES} common_base )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} common_base )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${ZLIB_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${MAPCACHE_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} )
source_group( mapcache FILES ${MAPCACHE_SOURCES} )
add_executable( mapcache ${SOURCE_FILES} )
if( DEPENDENCIES )
	add_dependencies( mapcache ${DEPENDENCIES} )
endif()
target_link_libraries( mapcache ${LIBRARIES} )
set_target_properties( mapcache PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
include_directories( ${INCLUDE_DIRS} )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapcache DESCRIPTION "mapcache generator" DISPLAY_NAME "mapcache" GROUP Runtime )
	install( TARGETS mapcache
		DESTINATION "."
		COMPONENT Runtime_mapcache )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} mapcache  CACHE INTERNAL "" )
message( STATUS "Creating target mapcache - done" )
endif( BUILD_MAPCACHE )
