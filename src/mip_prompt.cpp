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
    printf("'sou': play_sound                    sound_idx(1~106)\n");
    printf("'dis': distance_drive                distance_m       angle_rad\n");
    printf("'tim': time_drive                    speed(-30~30)    time_s\n");
    printf("'ang': angle_drive                   speed(-24~24)    angle_rad\n");
    printf("'cli': continuous_drive_linear       speed(-64~64)\n");
    printf("'can': continuous_drive_angular      speed(-64~64)\n");
    printf("'mod': get_game_mode\n");
    printf("'bat': get_battery_voltage\n");
    printf("'ver': get_version\n");
    printf("'vol': get_volume\n");
    printf("'vol': set_volume                    VOL\n");
    return -1;
  }
  Mip mip;
  //mip.set_bluetooth_device_by_name("hci0");
  mip.set_bluetooth_device_by_mac("00:1A:7D:DA:71:11");
  mip.connect("D0:39:72:B7:AF:66");
  std::string choice = argv[1];
  double param1 = (argc >= 3 ? atof(argv[2]) : -1);
  double param2 = (argc >= 4 ? atof(argv[3]) : -1);
  if (choice == "sou")
    printf("retval:%i\n", mip.play_sound(param1));
  else if (choice == "dis")
    printf("retval:%i\n", mip.distance_drive(param1, param2));
  else if (choice == "tim")
    printf("retval:%i\n", mip.time_drive(param1, param2));
  else if (choice == "ang")
    printf("retval:%i\n", mip.angle_drive(param1, param2));
  else if (choice == "cli")
    printf("retval:%i\n", mip.continuous_drive_linear(param1));
  else if (choice == "can")
    printf("retval:%i\n", mip.continuous_drive_angular(param1));
  else if (choice == "mod")
    printf("game_mode:%i = '%s'\n", mip.get_game_mode(), mip.get_game_mode2str());
  else if (choice == "bat")
    printf("battery:%fV = %i%%\n", mip.get_battery_voltage(), mip.get_battery_percentage());
  else if (choice == "ver")
    printf("version:'%s'\n", mip.get_version().c_str());
  else if (choice == "vol" && argc == 2)
    printf("volume:%i\n", mip.get_volume());
  else if (choice == "vol")
    printf("retval:%i\n", mip.set_volume(param1));
  return 0;
}

