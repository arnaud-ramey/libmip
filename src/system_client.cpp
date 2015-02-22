#include "mip.h"

int main(int argc, char **argv) {
  Mip mip;
  //mip.set_bluetooth_device_by_name("hci0");
  mip.set_bluetooth_device_by_mac("00:1A:7D:DA:71:11");
  mip.connect("D0:39:72:B7:AF:66");
  printf("volume:%i\n", mip.get_volume());
  printf("version:'%s'\n", mip.get_version().c_str());
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
