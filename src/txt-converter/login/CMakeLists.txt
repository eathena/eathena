
#
# login-converter
#
if( BUILD_CONVERTERS )
message( STATUS "Creating target login-converter" )
set( COMMON_HEADERS
	${COMMON_BASE_HEADERS}
	)
set( COMMON_SOURCES
	${COMMON_BASE_SOURCES}
	)
set( TXT_HEADERS
	"${TXT_LOGIN_SOURCE_DIR}/account.h"
	)
set( TXT_SOURCES
	"${TXT_LOGIN_SOURCE_DIR}/account_txt.c"
	)
set( SQL_HEADERS
	"${SQL_LOGIN_SOURCE_DIR}/account.h"
	)
set( SQL_SOURCES
	"${SQL_LOGIN_SOURCE_DIR}/account_sql.c"
	)
set( CONVERTER_SOURCES
	"${CONVERTER_SOURCE_DIR}/login-converter.c"
	)
set( DEPENDENCIES ${MYSQL_DEPENDENCIES} )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${MYSQL_DEFINITIONS} -DWITH_TXT -DWITH_SQL" )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} ${TXT_HEADERS} ${TXT_SOURCES} ${SQL_HEADERS} ${SQL_SOURCES} ${CONVERTER_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( txt FILES ${TXT_HEADERS} ${TXT_SOURCES} )
source_group( sql FILES ${SQL_HEADERS} ${SQL_SOURCES} )
source_group( converter FILES ${CONVERTER_SOURCES} )
add_executable( login-converter ${SOURCE_FILES} )
if( DEPENDENCIES )
	add_dependencies( login-converter ${DEPENDENCIES} )
endif()
target_link_libraries( login-converter ${LIBRARIES} )
set_target_properties( login-converter PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
include_directories( ${INCLUDE_DIRS} )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_loginconverter DESCRIPTION "login-converter" DISPLAY_NAME "login-converter" GROUP Runtime )
	install( TARGETS login-converter
		DESTINATION "tools"
		COMPONENT Runtime_loginconverter )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} login-converter  CACHE INTERNAL "" )
message( STATUS "Creating target login-converter - done" )
endif( BUILD_CONVERTERS )
