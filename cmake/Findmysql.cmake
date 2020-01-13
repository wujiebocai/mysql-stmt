
if( WIN32 )
	find_path( MYSQL_INCLUDE_DIR
		NAMES "mysql.h"
		PATHS "$ENV{SYSTEMDRIVE}/MySQL/*/include" )
	
	find_library( MYSQL_LIBRARY
		NAMES "mysqlclient" "mysqlclient_r"
		PATHS "$ENV{SYSTEMDRIVE}/MySQL/*/lib" )
else()
	find_path( MYSQL_INCLUDE_DIR
		NAMES "mysql.h"
		PATHS "/usr/app/mysql/include" )
	
	find_library( MYSQL_LIBRARY
		NAMES "mysqlclient" "mysqlclient_r"
		PATHS "/usr/app/mysql/lib" )
endif()


if( MYSQL_INCLUDE_DIR AND EXISTS "${MYSQL_INCLUDE_DIR}/mysql_version.h" )
	file( STRINGS "${MYSQL_INCLUDE_DIR}/mysql_version.h"
		MYSQL_VERSION_H REGEX "^#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE
		"^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_VERSION_STRING
		"${MYSQL_VERSION_H}" )
endif()


MARK_AS_ADVANCED(
  MYSQL_INCLUDE_DIR
  )
  
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( MYSQL REQUIRED_VARS MYSQL_LIBRARY MYSQL_INCLUDE_DIR
	VERSION_VAR	MYSQL_VERSION_STRING )

if(MYSQL_FOUND)
  set( MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR} )
  set( MYSQL_LIBRARIES ${MYSQL_LIBRARY} )
  
  MESSAGE(STATUS "Found MYSQL INCLUDE: ${MYSQL_INCLUDE_DIRS} (found version \"${MYSQL_VERSION_STRING}\")")
endif()

