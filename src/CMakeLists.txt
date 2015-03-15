### third parties
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

# joystick - https://github.com/drewnoakes/joystick
add_library(joystick joystick/joystick.cc joystick/joystick.hh)

### our code
add_executable(gattmip_prompt  gattmip_prompt.cpp mipcommands.h gattmip.h
                               bluetooth_mac2device.h exec_system_get_output.h)
target_link_libraries(gattmip_prompt libgatt ${GLIB_LIBRARIES})

add_subdirectory(samples)
