/*!
  \file        square.cpp
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
drawing a square.
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
  for (int side = 0; side < 4; ++side) {
    mip.distance_drive(.5, 0);
    sleep(2);
    mip.distance_drive(0, M_PI_2);
    sleep(2);
  }
  return 0;
}
