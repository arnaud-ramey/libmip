#include "exec_system_get_output.h"
#include "string_split.h"
#include "find_and_replace.h"
#include <algorithm>
#include <iomanip>      // std::setfill, std::setw
#include <unistd.h>
#include <math.h>

#define RAD2DEG 0.01745329251994329577
#define DEG2RAD 57.2957795130823208768

////////////////////////////////////////////////////////////////////////////////

// http://www.humbug.in/2014/using-gatttool-manualnon-interactive-mode-read-ble-devices/
class Mip {
public:
  Mip() {
    // free possibly busy bluetooth devices
    system("rfkill unblock all");
  }

  //////////////////////////////////////////////////////////////////////////////

  bool set_device_by_name(const std::string & device) {
    printf("set_device_by_name(device:'%s')\n", device.c_str());
    _hci_device = device;
    // sudo hciconfig hci1 up
    std::ostringstream order;
    order << "hciconfig " << _hci_device << " up";
    exec_system_get_output(order.str().c_str());
  }

  //////////////////////////////////////////////////////////////////////////////

  bool set_device_by_bd_address(const std::string & address) {
    std::string result = exec_system_get_output("hciconfig");
    StringUtils::find_and_replace(result, "\n", " ");
    StringUtils::find_and_replace(result, "\t", " ");
    while (StringUtils::find_and_replace(result, "  ", " ")) {}
    printf("result:'%s\n'\n", result.c_str());
    std::vector<std::string> words;
    StringUtils::StringSplit(result, " ", &words);
    std::vector<std::string>::const_iterator it =
        std::find(words.begin(), words.end(), address)  ;
    if (it == words.end()) {
      printf("No device with address'%s'\n", address.c_str());
      return false;
    }
    while (it != words.begin()) {
      if (it->size() > 3 && it->substr(0, 3) == "hci")
        return set_device_by_name(it->substr(0, 4));
      --it;
    }
    printf("Device with address'%s' has no 'hci' name!\n", address.c_str());
    return false;
  } // end set_device_by_bd_address()

  //////////////////////////////////////////////////////////////////////////////

  inline bool connect(const std::string & address) {
    _address = address;
    return true; // TODO check device exists?
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg sound_idx Sound file index (1~106) - Send 105 to stop playing
  inline bool play_sound(uint sound_idx) {
    // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
    return send_order("06", int_to_hex(clamp(sound_idx, 1, 106)));
  }

  //! \arg vol 0~9
  inline bool set_volume(uint vol) {
    return send_order("06", int_to_hex(247 + clamp(vol, 0, 7) )); // 0xF7­~0xFE for volume
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg distance_m in meters, no speed control
  inline bool distance_drive(double distance_m,
                             double angle_rad) {
    bool backward = (distance_m < 0);
    int angle_deg = rad2deg_norm(angle_rad, -360, 360);
    bool ccw = (angle_deg < 0);
    // BYTE 1 : Forward: 0X00 or Backward: 0X01
    // BYTE 2 : Distance (cm): 0x00­0xFF
    std::string order = int2_to_hex(backward, clamp(fabs(distance_m)*100, 0, 255) )
        // BYTE 3 : Turn Clockwise: 0X00 or Anti­-clockwise: 0X01
        // BYTE 4 : Turn Angle(High byte): 0x00~0x01
        // BYTE 5 : Turn Angle(Low byte): 0x00~0xFF
        + int3_to_hex(ccw, angle_deg/256, angle_deg%256);
    return send_order("70", order);
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~30 (-30~0 to go backwards)
   *  \arg time_s in seconds */
  inline bool time_drive(double speed, double time_s) {
    int time_7ms = clamp(time_s * 1000/7, 0, 255);
    if (speed > 0)
      return send_order("71", int2_to_hex(clamp(speed, 0, 30), time_7ms));
    return send_order("72", int2_to_hex(clamp(-speed, 0, 30), time_7ms));
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~24 (-24~0 to go backwards)
   *  \arg angle_rad in radians, > 0 for CCW, < 0 for CW */
  inline bool angle_drive(double speed, double angle_rad) {
    // BYTE 1 : Angle in intervals of 5 degrees (0~255) - Angle = Byte1 Value * 5
    int angle_deg = rad2deg_norm(angle_rad, -1275, 1275);
    if (angle_deg < 0) // CCW
      return send_order("73", int2_to_hex(clamp(speed, 0, 24), fabs(angle_deg/5)));
    return send_order("74", int2_to_hex(clamp(-speed, 0, 24), fabs(angle_deg/5)));
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 1~64 (-64~1 to go backwards)  */
  inline bool continuous_drive_linear(double speed) {
    speed = clamp(speed, -64, 64);
    if (speed > 32) // 33 ~ 64 => Carzy Fw:0x81(slow)~­0xA0(fast) = 129 ~ 160
      return send_order("78", int_to_hex(96 + speed));
    if (speed > 0) // 1 ~ 32 => Fw:0x01(slow)~­0x20(fast)
      return send_order("78", int_to_hex(speed));
    if (speed > -32) // -1 ~ -32 => Bw:0x21(slow)~0x40(fast) = 33 ~ 64
      return send_order("78", int_to_hex(32 - speed));
    // -33 -> -64 => Carzy Bw:0xA1(slow)~0xC0(fast) = 161 ~ 192
    return send_order("78", int_to_hex(128 - speed));
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~64 to turn CCW (-64~0 to turn CW)  */
  inline bool continuous_drive_angular(double speed) {
    speed = clamp(speed, -64, 63);
    if (speed > 32) // 33 ~ 64 => Carzy Left spin:0xE1(slow)~0xFF(fast) = 225 - 255
      return send_order("78", int_to_hex(192 + speed));
    if (speed > 0) // 1 ~ 32 => Left spin:0x61(slow)~0x80(fast) = 97 ~ 128
      return send_order("78", int_to_hex(96 + speed));
    if (speed > -32) // -1 ~ -32 => right spin:0x41(slow)~0x60(fast) = 65 ~ 96
      return send_order("78", int_to_hex(64 - speed));
    // -33 -> -64 => Carzy right spin:0xC1(slow)~0xE0(fast) = 193 ~ 224
    return send_order("78", int_to_hex(160 - speed));
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool stop() {
    return send_order("77");
  }

  //////////////////////////////////////////////////////////////////////////////

  inline uint get_game_mode() {
    return atoi(get_value("0a"));
  }

  //////////////////////////////////////////////////////////////////////////////

  inline uint get_volume() {
    return atoi(get_value("0a"));
  }

  //////////////////////////////////////////////////////////////////////////////

protected:
  static const inline uint rad2deg_norm(const double & angle_rad,
                                        const double angle_min = 0,
                                        const double angle_max = 360) {
    int angle_deg = angle_rad * RAD2DEG;
    while (angle_deg < angle_min)    angle_deg+=360;
    while (angle_deg >= angle_max) angle_deg-=360;
    return angle_deg;
  }

  static const inline uint clamp(uint x, uint min, uint max) {
    return (x < min ? min : x > max ? max : x);
  }

  static const inline std::string int_to_hex(uint i1, uint ndigits = 2) {
    std::ostringstream ans;
    ans << std::setw(ndigits) << std::setfill('0') << std::hex << i1;
    return ans.str();
  }
  static const inline std::string int2_to_hex
  (uint i1, uint i2, uint ndigits = 2) {
    std::ostringstream ans;
    ans << std::setw(ndigits) << std::setfill('0') << std::hex << i1 << i2;
    return ans.str();
  }
  static const inline std::string int3_to_hex
  (uint i1, uint i2, uint i3, uint ndigits = 2) {
    std::ostringstream ans;
    ans << std::setw(ndigits) << std::setfill('0') << std::hex << i1 << i2 << i3;
    return ans.str();
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool send_order(const std::string & handle, const std::string & param = "") {
    // sudo gatttool -i hci1 -b D0:39:72:B7:AF:66 --char-write -a 0x0013 -n 0602
    std::ostringstream order;
    order << "gatttool -i " << _hci_device << " -b " << _address;
    order << " --char-write -a 0x0013 -n " << handle << param;
    printf("order:'%s'\n", order.str().c_str());
    return system(order.str().c_str());
  }

  //////////////////////////////////////////////////////////////////////////////

  inline std::string get_value(const std::string & handle) {
    // sudo gatttool -i hci1 -b D0:39:72:B7:AF:66 --char-read -a 0x0a
    // handle: 0x0016, char properties: 0x0a, char value handle: 0x0017, uuid: 0000fff1-0000-1000-8000-00805f9b34fb
    std::ostringstream order;
    order << "gatttool -i " << _hci_device << " -b " << _address;
    order << " --char-read -a 0x" << handle;
    printf("order:'%s'\n", order.str().c_str());
    return exec_system_get_output(order.str().c_str());
  }

  std::string _address, _hci_device;
}; // end class Mip

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  Mip mip;
  //mip.set_device_by_name("hci0");
  mip.set_device_by_bd_address("00:1A:7D:DA:71:11");
  mip.connect("D0:39:72:B7:AF:66");
  for (int sound_idx = 0; sound_idx < 100; ++sound_idx) {
    mip.play_sound(sound_idx);
    sleep(1);
  }
  return 0;
}
