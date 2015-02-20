/*!
  file
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2015/2/19

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

The MIP library.
 */
#include "mip.h"
#include <iostream>

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Synopsis: %s CMD PARAM\n", argv[0]);
    printf("1: play_sound                    sound_idx(1~106)\n");
    printf("2: distance_drive                distance_m       angle_rad\n");
    printf("3: time_drive                    speed(-30~30)    time_s\n");
    printf("4: angle_drive                   speed(-24~24)    angle_rad\n");
    printf("5: continuous_drive_linear       speed(-64~64)\n");
    printf("6: continuous_drive_angular      speed(-64~64)\n");
    printf("20: set_volume       VOL\n");
    printf("21: get_volume\n");
    printf("10: get_version\n");
    printf("11: get_game_mode\n");
    printf("12: get_battery_voltage\n");
    return -1;
  }
  Mip mip;
  //mip.set_device_by_name("hci0");
  mip.set_device_by_bd_address("00:1A:7D:DA:71:11");
  mip.connect("D0:39:72:B7:AF:66");
  int choice = atoi(argv[1]);
  double param1 = (argc >= 3 ? atof(argv[2]) : -1);
  double param2 = (argc >= 4 ? atof(argv[3]) : -1);
  if (choice == 1)
    printf("retval:%i\n", mip.play_sound(param1));
  else if (choice == 2)
    printf("retval:%i\n", mip.distance_drive(param1, param2));
  else if (choice == 3)
    printf("retval:%i\n", mip.time_drive(param1, param2));
  else if (choice == 4)
    printf("retval:%i\n", mip.angle_drive(param1, param2));
  else if (choice == 5)
    printf("retval:%i\n", mip.continuous_drive_linear(param1));
  else if (choice == 6)
    printf("retval:%i\n", mip.continuous_drive_angular(param1));
  else if (choice == 10)
    printf("version:'%s'\n", mip.get_version().c_str());
  else if (choice == 11)
    printf("game_mode:%i = '%s'\n", mip.get_game_mode(), mip.get_game_mode2str());
  else if (choice == 12)
  printf("battery:%fV = %i%%\n", mip.get_battery_voltage(), mip.get_battery_percentage());
  else if (choice == 20)
    printf("retval:%i\n", mip.set_volume(param1));
  else if (choice == 21)
    printf("volume:%i\n", mip.get_volume());
  return 0;
}

