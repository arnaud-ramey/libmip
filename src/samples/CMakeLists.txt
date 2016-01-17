add_executable(joystick_control        joystick_control.cpp)
target_link_libraries(joystick_control libgatt ${GLIB_LIBRARIES} joystick)

add_executable(play_all_sounds         play_all_sounds.cpp)
target_link_libraries(play_all_sounds  libgatt ${GLIB_LIBRARIES})

add_executable(random_walk             random_walk.cpp)
target_link_libraries(random_walk      libgatt ${GLIB_LIBRARIES})

add_executable(speed_calibration       speed_calibration.cpp)
target_link_libraries(speed_calibration libgatt ${GLIB_LIBRARIES} curses)

add_executable(square                  square.cpp)
target_link_libraries(square           libgatt ${GLIB_LIBRARIES})
