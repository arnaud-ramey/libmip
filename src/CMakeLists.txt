add_executable(system_client system_client.cpp exec_system_get_output.h mip.h)

add_executable(mip_prompt mip_prompt.cpp exec_system_get_output.h mip.h)

# compile libgatt, from https://github.com/jacklund/libgatt
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})
find_package( GLIB REQUIRED )
include_directories(${GLIB_INCLUDE_DIRS})
add_library(libgatt
  libgatt/src/att.c
  libgatt/src/gatt.c
  libgatt/src/gattrib.c
  libgatt/src/btio.c
  libgatt/src/uuid.c
  libgatt/src/hci.c
  libgatt/src/sdp.c
  libgatt/src/bluetooth.c
  #libgatt/src/utils.c
)

add_executable(libgatttest libgatt/src/test.c)
target_link_libraries(libgatttest libgatt ${GLIB_LIBRARIES})

add_executable(gattmip gattmip.cpp)
target_link_libraries(gattmip libgatt ${GLIB_LIBRARIES})
