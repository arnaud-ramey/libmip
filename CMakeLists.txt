cmake_minimum_required(VERSION 2.8)
project( mip )
SET(CMAKE_VERBOSE_MAKEFILE OFF)
# Debug, Release, RelWithDebInfo and MinSizeRe
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(src)
