cmake_minimum_required(VERSION 2.8)
project( mip )
# Debug, Release, RelWithDebInfo and MinSizeRe
set(CMAKE_BUILD_TYPE Debug)

add_executable(system_client system_client.cpp)
