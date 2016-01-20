/*!
  \file        joystick_control.cpp
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2015/2/21

________________________________________________________________________________

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
________________________________________________________________________________
A simple demo for the libmip library:
driving the robot with the joystick.
 */
#include "src/bluetooth_mac2device.h"
#include "src/gattmip.h"
#include "src/joystick/joystick.hh"

int main(int argc, char** argv) {
  GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
  Mip mip;
  std::string device_mac = (argc >= 2 ? argv[1] : "00:1A:7D:DA:71:11"),
      mip_mac = (argc >= 3 ? argv[2] : "D0:39:72:B7:AF:66"),
      joystick_device = "/dev/input/js1";
  if (!mip.connect(main_loop, bluetooth_mac2device(device_mac).c_str(), mip_mac.c_str())) {
    printf("Could not connect with device MAC '%s' to MIP with MAC '%s'!\n",
           device_mac.c_str(), mip_mac.c_str());
    return -1;
  }
  // Create an instance of Joystick
  Joystick joystick(joystick_device);
  // Ensure that it was found and that we can use it
  if (!joystick.isFound()) {
    printf("Connecting to joystick '%s' failed.\n", joystick_device.c_str());
    return -1;
  }

  double speed_lin = 0, speed_ang = 0;
  static const double MAX_AXIS = 32767, MAX_SPEED_LIN = 60, MAX_SPEED_ANG = 600;
  while (true) {
    // Restrict rate
    usleep(25E3);
    mip.continuous_drive(MAX_SPEED_LIN * speed_lin, MAX_SPEED_ANG * speed_ang);
    mip.pump_up_callbacks();
    // Attempt to sample an event from the joystick
    JoystickEvent event;
    if (!joystick.sample(&event))
      continue;
    if (!event.isAxis())
      continue;
    printf("Axis %u is at position %d\n", event.number, event.value);
    if (event.number == 1 || event.number == 5) { // up=-32767, down=32767
      speed_lin = -1. * event.value / MAX_AXIS;
    }
    if (event.number == 2 || event.number == 4) { // left=-32767, right=32767
      speed_ang = -1. * event.value / MAX_AXIS;
    }
    printf("speed(v:%f, w:%f)\n", speed_lin, speed_ang);
    //mip.angle_drive(10. * speed_ang, 24. * speed_lin);
    //mip.distance_drive(10. * speed_lin, 10. * speed_ang);
  } // end while()
  return 0;
} // end main()
