cmake_minimum_required(VERSION 2.8)
project(mysqllib)

# version
execute_process(
    OUTPUT_VARIABLE GIT_BRANCH
    COMMAND git symbolic-ref --short -q HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(TIMESTAMP TODAY "%Y%m%d%H%I")
set(BUILD_VERSION "${TODAY}")
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/version/version_config.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/version/version_config.h"
)

# option
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
set(LIBPROTOBUF_PLATFORM "ubuntu")

# def
add_definitions(-DASIO_STANDALONE)
add_definitions(-DASIO_NO_DEPRECATED)
add_definitions(-DUSE_OPENSSL)
add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)

# platform
if(UNIX)
    include(cmake/linux.cmake)
else()
    include(cmake/windows.cmake)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# include
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/mysql")      									


include_directories("${MYSQL_INCLUDE_DIR}")                              

# sub
add_subdirectory(demo)
