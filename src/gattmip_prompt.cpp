/*!
  \file        gattmip_prompt.cpp
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2015/2/24

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

A standalone prompt to test the MIP library.
 */

#include "gattmip.h"
#include "bluetooth_mac2device.h"
#include <iostream>

void print_help(int argc, char** argv) {
  printf("Synopsis: %s CMD PARAM\n", argv[0]);
  printf("'sou':  play_sound                    sound_idx(1~106)\n");
  printf("'dis':  distance_drive                distance_m       angle_rad\n");
  printf("'tim':  time_drive                    speed(-30~30)    time_s(0~1.78)\n");
  printf("'ang':  angle_drive                   speed(-24~24)    angle_rad\n");
  printf("'con':  continuous_drive              lin_speed(-64~64) ang_speed(-64~64) [time ms]\n");
  printf("'mod':  get_game_mode\n");
  printf("'sto':  stop\n");
  printf("'sta':  get_status\n");
  printf("'up':   up\n");
  printf("'wei':  get_weight\n");
  printf("'cled': get_chest led\n");
  printf("'cled': set_chest led                 r(0~255) g(0~255) b(0~255)\n");
  printf("'cled': set_chest led                 r(0~255) g(0~255) b(0~255) time_flash_on(s) time_flash_off(s)\n");
  printf("'hled': get_head led\n");
  printf("'hled': set_head led                  l1(0=off,1=on,2=blink_slow,3=blink_fast) l2 l3 l4\n");
  printf("'odo':  get_odometer_reading\n");
  printf("'ges':  get_gesture_detect\n");
  printf("'gmod': get_gesture_or_radar_mode\n");
  printf("'gmod': set_gesture_or_radar_mode     mod(0=GESTUREOFF_RADAROFF,2=GESTUREON_RADAROFF,4=GESTUREOFF_RADARON)\n");
  printf("'rad':  get_radar_response\n");
  printf("'bat':  get_battery_voltage\n");
  printf("'sve':  get_software_version\n");
  printf("'hve':  get_hardware_version\n");
  printf("'vol':  get_volume\n");
  printf("'vol':  set_volume                    vol(0~7)\n");
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
  if (argc < 2) {
    print_help(argc, argv);
    return -1;
  }
  GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
  Mip mip;
  std::string device_mac = (argc >= 20 ? argv[1] : "00:1A:7D:DA:71:11"),
      mip_mac = (argc >= 30 ? argv[2] : "D0:39:72:B7:AF:66");
  if (!mip.connect(main_loop, bluetooth_mac2device(device_mac).c_str(), mip_mac.c_str())) {
    printf("Could not connect with device MAC '%s' to MIP with MAC '%s'!\n",
           device_mac.c_str(), mip_mac.c_str());
    return -1;
  }

  std::string choice = argv[1];
  unsigned int nparams = argc - 2;
  double param1 = (argc >= 3 ? atof(argv[2]) : -1);
  double param2 = (argc >= 4 ? atof(argv[3]) : -1);
  double param3 = (argc >= 5 ? atof(argv[4]) : -1);
  double param4 = (argc >= 6 ? atof(argv[5]) : -1);
  double param5 = (argc >= 7 ? atof(argv[6]) : -1);
  if (choice == "sou" && nparams == 1)
    printf("retval:%i\n", mip.play_sound(param1));
  else if (choice == "dis" && nparams == 2)
    printf("retval:%i\n", mip.distance_drive(param1, param2));
  else if (choice == "tim" && nparams == 2)
    printf("retval:%i\n", mip.time_drive(param1, param2));
  else if (choice == "ang" && nparams == 2)
    printf("retval:%i\n", mip.angle_drive(param1, param2));
  else if (choice == "con" && nparams == 2)
    printf("retval:%i\n", mip.continuous_drive(param1, param2));
  else if (choice == "con" && nparams == 3) {
    unsigned int ntimes = param3 / 50.; // a command each 50 ms
    printf("ntimes:%i\n", ntimes);
    for (int i = 0; i < ntimes; ++i) {
      mip.continuous_drive(param1, param2);
      mip.pump_up_callbacks();
      usleep(50 * 1000);
    }
   }
  else if (choice == "mod" && nparams == 0) {
    mip.request_game_mode();
    mip.pump_up_callbacks(10);
    printf("game_mode:%i = '%s'\n", mip.get_game_mode(), mip.get_game_mode2str());
  }
  else if (choice == "sto" && nparams == 0)
    printf("retval:%i\n", mip.stop());
  else if (choice == "sta" && nparams == 0) {
    mip.request_status();
    mip.pump_up_callbacks(10);
    printf("status:%i = '%s'\n", mip.get_status(), mip.get_status2str());
  }
  else if (choice == "up" && nparams == 0)
    mip.up();
  else if (choice == "wei" && nparams == 0) {
    mip.request_weight_update();
    mip.pump_up_callbacks(10);
    printf("weight:%i\n", mip.get_weight_update());
  }
  else if (choice == "cled" && nparams == 0) {
    mip.request_chest_LED();
    mip.pump_up_callbacks(10);
    printf("chest_led:%s\n", mip.get_chest_LED().to_string().c_str());
  }
  else if (choice == "cled" && nparams == 3) {
    printf("retval:%i\n", mip.set_chest_LED(param1, param2, param3));
  }
  else if (choice == "cled" && nparams == 5) {
    Mip::ChestLed led;
    led.r = param1;
    led.g = param2;
    led.b = param3;
    led.time_flash_on_sec = param4;
    led.time_flash_off_sec = param5;
    printf("retval:%i\n", mip.set_chest_LED(led));
  }
  else if (choice == "hled" && nparams == 0) {
    mip.request_head_LED();
    mip.pump_up_callbacks(10);
    printf("head_led:%s\n", mip.get_head_LED().to_string().c_str());
  }
  else if (choice == "hled" && nparams == 4) {
    Mip::HeadLed led;
    led.l1 = param1;
    led.l2 = param2;
    led.l3 = param3;
    led.l4 = param4;
    printf("retval:%i\n", mip.set_head_LED(led));
  }
  else if (choice == "odo" && nparams == 0) {
    mip.request_odometer_reading();
    mip.pump_up_callbacks(10);
    printf("odometer:%f\n", mip.get_odometer_reading());
  }
  else if (choice == "ges" && nparams == 0)
    printf("gesture:%i = '%s'\n", mip.get_gesture_detect(), mip.get_gesture_detect2str());
  else if (choice == "gmod" && nparams == 0) {
    mip.request_gesture_or_radar_mode();
    mip.pump_up_callbacks(10);
    printf("gesture_or_radar_mode:%i = '%s'\n",
           mip.get_gesture_or_radar_mode(),
           mip.get_gesture_or_radar_mode2str());
  }
  else if (choice == "gmod" && nparams == 1)
    mip.set_gesture_or_radar_mode(param1);
  else if (choice == "rad" && nparams == 0)
    printf("radar_response:%i = '%s'\n",
           mip.get_radar_response(), mip.get_radar_response2str());
  else if (choice == "bat" && nparams == 0) {
    mip.request_battery_voltage();
    mip.pump_up_callbacks(10);
    printf("battery:%fV = %i%%\n", mip.get_battery_voltage(), mip.get_battery_percentage());
  }
  else if (choice == "sve" && nparams == 0) {
    mip.request_software_version();
    mip.pump_up_callbacks(10);
    printf("software version:'%s'\n", mip.get_software_version().c_str());
  }
  else if (choice == "hve" && nparams == 0) {
    mip.request_hardware_version();
    mip.pump_up_callbacks(10);
    printf("hardware version:'%s'\n", mip.get_hardware_version().c_str());
  }
  else if (choice == "vol" && nparams == 0) {
    mip.request_volume();
    mip.pump_up_callbacks(10);
    printf("volume:%i\n", mip.get_volume());
  }
  else if (choice == "vol" && nparams == 1)
    printf("retval:%i\n", mip.set_volume(param1));
  else // nothing done
    print_help(argc, argv);

  // ensure order was sent
  mip.pump_up_callbacks();
  return 0;
}

