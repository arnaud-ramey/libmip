cmake_minimum_required(VERSION 2.8)
project( mip )
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})
# sudo apt-get install libbluetooth-dev
find_package( Bluez REQUIRED )

add_executable( inquiry inquiry.cpp )
target_link_libraries( inquiry   ${BLUEZ_LIBRARIES} )
add_executable( inquiry_raw inquiry_raw.cpp )
target_link_libraries( inquiry_raw   ${BLUEZ_LIBRARIES} )
#~ add_executable( scanner2 scanner2.cpp )
#~ target_link_libraries( scanner2   ${BLUEZ_LIBRARIES} )

add_executable( drdobbs drdobbs.cpp )
target_link_libraries( drdobbs   ${BLUEZ_LIBRARIES} )
add_executable( drdobbs2 drdobbs2.c )
target_link_libraries( drdobbs2   ${BLUEZ_LIBRARIES} )

add_executable( client client.cpp )
target_link_libraries( client   ${BLUEZ_LIBRARIES} )

include_directories("/usr/include/glib-2.0/"
    "/usr/lib/i386-linux-gnu/glib-2.0/include/"
    "bluez-5.27/"
    "bluez-5.27/attrib/")
find_package( GLIB REQUIRED )

# bluez-5.27/attrib/utils.o
add_executable( gatttool gatttool.cpp
    bluez-5.27/attrib/utils.o
    bluez-5.27/attrib/gattrib.o
    bluez-5.27/btio/btio.o)
target_link_libraries( gatttool   ${BLUEZ_LIBRARIES} ${GLIB_LIBRARIES}
#${PROJECT_SOURCE_DIR}/bluez-5.27/src/shared/queue.lo
    ${PROJECT_SOURCE_DIR}/bluez-5.27/lib/.libs/libbluetooth.so)
