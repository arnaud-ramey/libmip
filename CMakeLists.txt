cmake_minimum_required(VERSION 2.8)
project( mip )
SET(CMAKE_VERBOSE_MAKEFILE OFF)
# Debug, Release, RelWithDebInfo and MinSizeRe
set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(src)
