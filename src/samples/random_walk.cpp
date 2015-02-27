/*!
  \file        random_walk.cpp
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2015/2/27

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
random walking while avoiding obstacles.
 */
#include "src/bluetooth_mac2device.h"
#include "src/gattmip.h"

int main(int argc, char** argv) {
  GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
  Mip mip;
  std::string device_mac = (argc >= 2 ? argv[1] : "00:1A:7D:DA:71:11"),
      mip_mac = (argc >= 3 ? argv[2] : "D0:39:72:B7:AF:66");
  if (!mip.connect(main_loop, bluetooth_mac2device(device_mac).c_str(), mip_mac.c_str())) {
    printf("Could not connect with device MAC '%s' to MIP with MAC '%s'!\n",
           device_mac.c_str(), mip_mac.c_str());
    return -1;
  }
  // now the real stuff
  mip.set_gesture_or_radar_mode(GESTUREOFF_RADARON);
  mip.pump_up_callbacks(10);
  mip.request_gesture_or_radar_mode();
  mip.pump_up_callbacks(10);
  printf("gesture_or_radar_mode:%i = '%s'\n",
         mip.get_gesture_or_radar_mode(),
         mip.get_gesture_or_radar_mode2str());

  int rotate_in_place_counter = INFINITY;

  while(true) {
    if (rotate_in_place_counter < 20) // do nothing
      ++rotate_in_place_counter;
    else if (mip.get_radar_response() == RADAR_OBJECT_0TO10CM
             || mip.get_radar_response() == RADAR_OBJECT_10TO30CM) { // turn on place
      rotate_in_place_counter = 0;
      double angle_rad = (rand()%2 ? 1. : -1.) * (M_PI_2 + drand48());
      //mip.distance_drive(0, angle_rad); - no radar update
      mip.angle_drive(15, angle_rad);
    }
    else if (rand()%20 == 0) { // randomly change direction
      //mip.distance_drive(drand48(), angle_rad); - no radar update
      mip.time_drive(10 + rand()%10, 2);
    }
    mip.pump_up_callbacks();
    usleep(50E3);
  }
  return 0;
}
