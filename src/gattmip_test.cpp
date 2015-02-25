/*!
  \file
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
 */

#include "bluetooth_mac2device.h"
#include "gattmip.h"

int main(int argc, char **argv) {
  printf("main()\n");
  // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
  Mip mip;
  if (!mip.connect(bluetooth_mac2device("00:1A:7D:DA:71:11").c_str(),
              "D0:39:72:B7:AF:66")) {
    printf("Could not connect to MIP!\n");
    return -1;
  }

  // pump up the callbacks!
  GMainLoop *event_loop;
  event_loop = g_main_loop_new(NULL, FALSE);
  // g_main_loop_run(event_loop);
  // g_main_loop_unref(event_loop);
  GMainContext * context = g_main_loop_get_context(event_loop);
  for (int i = 0; i < 10; ++i) { // iterate a few times to connect well
    g_main_context_iteration(context, false);
    usleep(50E3);
  }
  unsigned int sound_idx = 1;
  while(true) {
    if (rand()%2)
      mip.request_volume();
    else
      mip.request_software_version();
    //    mip.play_sound(sound_idx); ++sound_idx;
    printf("volume:%i\n", mip.request_volume());

    // https://stackoverflow.com/questions/23737750/glib-usage-without-mainloop
    printf("g_main_context_iteration()\n");
    g_main_context_iteration(context, false);
    usleep(50E3);
    //sleep(1);
  }
  return 0;
}
