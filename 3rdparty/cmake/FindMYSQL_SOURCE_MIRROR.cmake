# - Find MYSQL_SOURCE_MIRROR
# Find the url to the source code of MySQL
#
#  MYSQL_SOURCE_MIRROR_VERSION  - version of the source code (optional)
#  MYSQL_SOURCE_MIRROR_FILESIZE - filesize of the source code archive (optional)
#  MYSQL_SOURCE_MIRROR_FILENAME - filename of the source code archive (optional)
#  MYSQL_SOURCE_MIRROR_MD5      - md5 hash of the source code archive
#  MYSQL_SOURCE_MIRROR_URL      - url of the source code archive
#  MYSQL_SOURCE_MIRROR_FOUND    - True if a mirror url is found.
#

if( NOT MYSQL_SOURCE_MIRROR_URL OR NOT MYSQL_SOURCE_MIRROR_MD5 )

macro( GET_NEXT_ELEMENT elements value result )
	list( FIND ${elements} ${value} _idx )
	if( _idx EQUAL -1 )
		unset( ${result} )
	else()
		math( EXPR _idx "${_idx} + 1" )
		list( GET ${elements} ${_idx} ${result} )
	endif()
endmacro()

# source code list
set( _file "${CMAKE_CURRENT_BINARY_DIR}/downloads.html" )
set( _url "http://www.mysql.com/downloads/mysql/?current_os=src&version=5.5" )
if( NOT MYSQL_SOURCE_MIRROR_FIND_QUIETLY )
	message( STATUS "Getting source code list from ${_url}" )
endif()
file( DOWNLOAD "${_url}" "${_file}" STATUS _status )
list( GET _status 0 _error )
if( _error EQUAL 0 )
	file( READ "${_file}" _html )
	string( REGEX MATCH "Generic Linux.*</tr>" _html "${_html}" ) # get Generic Linux
	string( REGEX MATCHALL "<?[^<>]+>?" _elements "${_html}" ) # break down html elements
	GET_NEXT_ELEMENT( _elements "<td class=\"col3\">" MYSQL_SOURCE_MIRROR_VERSION )
	GET_NEXT_ELEMENT( _elements "<td class=\"col4\">" MYSQL_SOURCE_MIRROR_FILESIZE )
	string( REGEX MATCH "<a href=\"([^\"]*)" _url "${_html}" )
	set( _url "${CMAKE_MATCH_1}" )
	if( _url MATCHES "^/downloads/.*$" )
		set( _url "http://www.mysql.com${_url}" ) 
	endif()
	GET_NEXT_ELEMENT( _elements "<td class=\"sub-text\">" MYSQL_SOURCE_MIRROR_FILENAME )
	string( REGEX MATCH "[^\\(][^\\)]*" MYSQL_SOURCE_MIRROR_FILENAME "${MYSQL_SOURCE_MIRROR_FILENAME}" )
	GET_NEXT_ELEMENT( _elements "<code class=\"md5\">" MYSQL_SOURCE_MIRROR_MD5 )
	
	# mirror list
	set( _file "${CMAKE_CURRENT_BINARY_DIR}/mirrors.html" )
	if( _url STREQUAL "" )
		set( _status -1 "URL of mirror list not found" )
	else()
		if( NOT MYSQL_SOURCE_MIRROR_FIND_QUIETLY )
			message( STATUS "Getting mirror list from ${_url}" )
		endif()
		file( DOWNLOAD "${_url}" "${_file}" STATUS _status SHOW_PROGRESS )
	endif()
	list( GET _error 0 _status )
	if( _error EQUAL 0 )
		file( READ "${_file}" _html )
		string( REGEX MATCH "<a href=\"([^\"]*/get/[^\"]*/[^\"]*)\">" MYSQL_SOURCE_MIRROR_URL "${_html}" ) # get download url
		set( MYSQL_SOURCE_MIRROR_URL "${CMAKE_MATCH_1}" )
		string( REGEX REPLACE ".*/get/(.*)/from/(.*)" "\\2\\1" MYSQL_SOURCE_MIRROR_URL "${MYSQL_SOURCE_MIRROR_URL}" ) # get mirror url
		if( MYSQL_SOURCE_MIRROR_URL STREQUAL "" )
			set( _status -1 "URL of source code not found" )
		endif()
	endif()
endif()
list( GET _status 0 _error )
if( _error EQUAL 0 )
	if( NOT MYSQL_SOURCE_MIRROR_FIND_QUIETLY )
		message( STATUS "Found version: ${MYSQL_SOURCE_MIRROR_VERSION}" )
		message( STATUS "Found filesize: ${MYSQL_SOURCE_MIRROR_FILESIZE}" )
		message( STATUS "Found filename: ${MYSQL_SOURCE_MIRROR_FILENAME}")
		message( STATUS "Found md5: ${MYSQL_SOURCE_MIRROR_MD5}" )
		message( STATUS "Found url: ${MYSQL_SOURCE_MIRROR_URL}" )
	endif()
	set( MYSQL_SOURCE_MIRROR_VERSION "${MYSQL_SOURCE_MIRROR_VERSION}" CACHE STRING "version of the source code (optional)" )
	set( MYSQL_SOURCE_MIRROR_FILESIZE "${MYSQL_SOURCE_MIRROR_FILESIZE}" CACHE STRING "filesize of the source code archive (optional)" )
	set( MYSQL_SOURCE_MIRROR_FILENAME "${MYSQL_SOURCE_MIRROR_FILENAME}" CACHE STRING "filename of the source code archive (optional)" )
	set( MYSQL_SOURCE_MIRROR_MD5 "${MYSQL_SOURCE_MIRROR_MD5}" CACHE STRING "md5 hash of the source code archive" )
	set( MYSQL_SOURCE_MIRROR_URL "${MYSQL_SOURCE_MIRROR_URL}" CACHE STRING "url of the source code archive" )
	mark_as_advanced( MYSQL_SOURCE_MIRROR_VERSION )
	mark_as_advanced( MYSQL_SOURCE_MIRROR_FILESIZE )
	mark_as_advanced( MYSQL_SOURCE_MIRROR_FILENAME )
	mark_as_advanced( MYSQL_SOURCE_MIRROR_MD5 )
	mark_as_advanced( MYSQL_SOURCE_MIRROR_URL )
else()
	if( NOT MYSQL_SOURCE_MIRROR_FIND_QUIETLY )
		message( STATUS "Failed: ${_status}" )
	endif()
	set( MYSQL_SOURCE_MIRROR_URL MYSQL_SOURCE_MIRROR_URL-NOTFOUND )
endif()
unset( _file )
unset( _url )
unset( _html )
unset( _elements )
unset( _status )
unset( _error )

endif( NOT MYSQL_SOURCE_MIRROR_URL OR NOT MYSQL_SOURCE_MIRROR_MD5 )

# handle the QUIETLY and REQUIRED arguments and set MYSQL_SOURCE_MIRROR_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS( MYSQL_SOURCE_MIRROR
	REQUIRED_VARS MYSQL_SOURCE_MIRROR_URL MYSQL_SOURCE_MIRROR_MD5
	VERSION_VAR MYSQL_SOURCE_MIRROR_VERSION
)
