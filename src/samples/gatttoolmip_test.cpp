/*!
  \file       square.cpp
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
A simple demo for the libmip library using gatttool.
 */
#include "src/gatttoolmip.h"
#include "src/bluetooth_mac2device.h"

int main(int argc, char** argv) {
  Mip mip;
  std::string device_mac = (argc >= 2 ? argv[1] : "00:1A:7D:DA:71:11"),
      mip_mac = (argc >= 3 ? argv[2] : "D0:39:72:B7:AF:66");
  if (!mip.connect(bluetooth_mac2device(device_mac).c_str(), mip_mac.c_str())) {
    printf("Could not connect with device MAC '%s' to MIP with MAC '%s'!\n",
           device_mac.c_str(), mip_mac.c_str());
    return -1;
  }
  printf("volume:%i\n", mip.get_volume());
  printf("version:'%s'\n", mip.get_software_version().c_str());
  printf("game_mode:%i\n", mip.get_game_mode());
  printf("battery:%fV = %i%%\n", mip.get_battery_voltage(), mip.get_battery_percentage());

  int r, g, b , flashing1, flashing2;
  mip.get_chest_LED(r, g, b , flashing1, flashing2);
  printf("chest LED:(%i,%i,%i), flashing:%i,%i\n", r, g, b , flashing1, flashing2);

  while (true) {
    mip.set_chest_LED(rand()%255, rand()%255, rand()%255);
    mip.get_chest_LED(r, g, b , flashing1, flashing2);
    printf("chest LED:(%i,%i,%i), flashing:%i,%i\n", r, g, b , flashing1, flashing2);
    printf("status:%i, weight:%i\n", mip.get_status(), mip.get_weight_update());
  }


  // mip.get_up();
//  for (int sound_idx = 0; sound_idx < 100; ++sound_idx) {
//    mip.play_sound(sound_idx);
//    sleep(1);
//  }
  return 0;
}
