/*!
  \file        mip.h
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
#ifndef MIP_H
#define MIP_H

#include "exec_system_get_output.h"
#include "string_split.h"
#include "find_and_replace.h"
#include <algorithm>
#include <iomanip>      // std::setfill, std::setw
#include <unistd.h>
#include <math.h> // fabs
#include <assert.h>

//#define DEBUG_PRINT(...)   {}
#define DEBUG_PRINT(...)   printf(__VA_ARGS__)
#define RAD2DEG 0.01745329251994329577
#define DEG2RAD 57.2957795130823208768

////////////////////////////////////////////////////////////////////////////////

static const int ERROR = -100;

// http://www.humbug.in/2014/using-gatttool-manualnon-interactive-mode-read-ble-devices/
class Mip {
public:
  typedef int GameMode;
  static const GameMode UNKNOWN = -1;
  static const GameMode APP = 1;
  static const GameMode CAGE = 2;
  static const GameMode TRACKING = 3;
  static const GameMode DANCE = 4;
  static const GameMode DEFAULT_MIP_MODE = 5;
  static const GameMode STACK = 6;
  static const GameMode TRICK_PROGRAMMING_AND_PLAYBACK = 7;
  static const GameMode ROAM_MODE = 8;
  typedef int Status;
  static const Status ON_BACK = 0;
  static const Status FACE_DOWN = 1;
  static const Status UPRIGHT = 2;
  static const Status PICKED_UP = 3; // Note:it will be sent after(connecting,falldown,pickup….)
  static const Status HAND_STAND = 4;
  static const Status FACE_DOWN_ON_TRAY = 5;
  static const Status ON_BACK_WITH_KICKSTAND = 6;


  Mip() {
    // free possibly busy bluetooth devices
    system("rfkill unblock all");
  }

  //////////////////////////////////////////////////////////////////////////////

  bool set_bluetooth_device_by_name(const std::string & device) {
    printf("Mip::set_bluetooth_device_by_name(device:'%s')\n", device.c_str());
    _hci_device = device;
    // sudo hciconfig hci1 up
    std::ostringstream order;
    order << "hciconfig " << _hci_device << " up";
    exec_system_get_output(order.str().c_str());
  }

  //////////////////////////////////////////////////////////////////////////////

  bool set_bluetooth_device_by_mac(const std::string & address) {
    printf("Mip::set_bluetooth_device_by_mac('%s')\n", address.c_str());
    std::string result = exec_system_get_output("hciconfig");
    StringUtils::find_and_replace(result, "\n", " ");
    StringUtils::find_and_replace(result, "\t", " ");
    while (StringUtils::find_and_replace(result, "  ", " ")) {}
    DEBUG_PRINT("result:'%s\n'\n", result.c_str());
    std::vector<std::string> words;
    StringUtils::StringSplit(result, " ", &words);
    std::vector<std::string>::const_iterator it =
        std::find(words.begin(), words.end(), address)  ;
    if (it == words.end()) {
      printf("Mip: No device with address'%s'\n", address.c_str());
      return false;
    }
    while (it >= words.begin()) {
      if (it->size() > 3 && it->substr(0, 3) == "hci")
        return set_bluetooth_device_by_name(it->substr(0, 4));
      --it;
    }
    printf("Mip: Device with address'%s' has no 'hci' name!\n", address.c_str());
    return false;
  } // end set_bluetooth_device_by_mac()

  //////////////////////////////////////////////////////////////////////////////

  inline bool connect(const std::string & address) {
    _address = address;
    return true; // TODO check device exists?
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  //! \arg sound_idx Sound file index (1~106) - Send 105 to stop playing
  inline bool play_sound(uint sound_idx) {
    // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
    return send_order("06", int_to_hex(clamp(sound_idx, 1, 106)));
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg distance_m in meters, no speed control
  inline bool distance_drive(double distance_m,
                             double angle_rad) {
    bool backward = (distance_m < 0);
    int angle_deg = rad2deg_norm(angle_rad, -360, 360);
    bool ccw = (angle_deg < 0);
    int distance_cm = clamp(fabs(distance_m*100.), 0, 255);
    printf("distance_cm:%i\n", distance_cm);
    std::ostringstream order;
    // BYTE 1 : Forward: 0X00 or Backward: 0X01
    // BYTE 2 : Distance (cm): 0x00­0xFF
    order << int2_to_hex(backward, distance_cm);
    // BYTE 3 : Turn Clockwise: 0X00 or Anti­-clockwise: 0X01
    // BYTE 4 : Turn Angle(High byte): 0x00~0x01
    // BYTE 5 : Turn Angle(Low byte): 0x00~0xFF
    order << int3_to_hex(ccw, angle_deg/256, angle_deg%256);
    return send_order("70", order.str());
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
    if (speed > 32) // 33 ~ 64 => crazy Fw:0x81(slow)~­0xA0(fast) = 129 ~ 160
      return send_order("78", int_to_hex(96 + speed));
    if (speed > 0) // 1 ~ 32 => Fw:0x01(slow)~­0x20(fast)
      return send_order("78", int_to_hex(speed));
    if (speed > -32) // -1 ~ -32 => Bw:0x21(slow)~0x40(fast) = 33 ~ 64
      return send_order("78", int_to_hex(32 - speed));
    // -33 -> -64 => crazy Bw:0xA1(slow)~0xC0(fast) = 161 ~ 192
    return send_order("78", int_to_hex(128 - speed));
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~64 to turn CCW (-64~0 to turn CW)  */
  inline bool continuous_drive_angular(double speed) {
    speed = clamp(speed, -64, 63);
    if (speed > 32) // 33 ~ 64 => crazy Left spin:0xE1(slow)~0xFF(fast) = 225 - 255
      return send_order("78", int_to_hex(192 + speed));
    if (speed > 0) // 1 ~ 32 => Left spin:0x61(slow)~0x80(fast) = 97 ~ 128
      return send_order("78", int_to_hex(96 + speed));
    if (speed > -32) // -1 ~ -32 => right spin:0x41(slow)~0x60(fast) = 65 ~ 96
      return send_order("78", int_to_hex(64 - speed));
    // -33 -> -64 => crazy right spin:0xC1(slow)~0xE0(fast) = 193 ~ 224
    return send_order("78", int_to_hex(160 - speed));
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool stop() {
    return send_order("77");
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see GameMode enum
  inline bool set_game_mode(const GameMode & mode) {
    return send_order("82", int_to_hex(mode));
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see GameMode enum
  inline GameMode get_game_mode() {
    return get_value_int1("82");
  }

  //! \see GameMode enum
  inline const char* get_game_mode2str() {
    switch (get_game_mode()) {
      case APP:  return "APP";
      case CAGE:  return "CAGE";
      case TRACKING:  return "TRACKING";
      case DANCE:  return "DANCE";
      case DEFAULT_MIP_MODE:  return "DEFAULT_MIP_MODE";
      case STACK:  return "STACK";
      case TRICK_PROGRAMMING_AND_PLAYBACK:  return "TRICK_PROGRAMMING_AND_PLAYBACK";
      case ROAM_MODE:  return "ROAM_MODE";
      case UNKNOWN:
      default:
        return "UNKNOWN";
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  //! between 4.0V and 6.4V, or < 0 if error
  inline double get_battery_voltage() {
    int voltage_int, status;
    if (!get_value_int2("79", voltage_int, status))
      return ERROR;
    // 0x4D = 77 = 4.0V, 0x7C = 124 = 6.4V
    return 4.0 + (voltage_int-77)*2.4/47;
  }

  //! in 0-100, or -1 if error
  inline int get_battery_percentage() {
    double voltage = get_battery_voltage();
    return (voltage < 0 ? ERROR : (voltage-4.0) / 2.4 * 100);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see Status enum
  inline Status get_status() {
    int voltage_int, status;
    return (get_value_int2("79", voltage_int, status) ? status : ERROR);
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool get_up() {
    return send_order("23", int_to_hex(2));
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \return angle with vertical, in [-45, 45], or -1 if ERROR.
  inline int get_weight_update() {
    int weight_int = get_value_int1("81");
    if (weight_int == ERROR)
      return ERROR;
    // 0xD3 = 211 = (-­45 degree) ~­ 0x2D = 45 = (+45 degree)
    // 0xD3 = 211 (max) ~ 0xFF = 255 (min) is holding the weight on the front
    // 0x00 = 0 (min) ~ 0x2D = 45 (max) is holding the weight on the back
    // in other words:0 -> 0, 45 -> 45, 255 -> -1, 211 -> -45
    return (weight_int < 100 ? weight_int : 256 - weight_int);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \return r,g,b in [0, 255]
  inline bool get_chest_LED(int & r, int & g, int & b, int & flashing1, int & flashing2) {
    if (!get_value_int5("83", r, g, b, flashing1, flashing2))
      return false;
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////

  //! r,g,b in [0, 255]
  inline bool set_chest_LED(const int & r, const int & g, const int & b) {
    return send_order("84", int3_to_hex(r, g, b));
  }

  //////////////////////////////////////////////////////////////////////////////

  //! r,g,b in [0, 255],
  inline bool set_chest_LED(const int & r, const int & g, const int & b,
                            const double & time_flash_on_sec, const double & time_flash_off_sec) {
    int ton = clamp( (int) (time_flash_on_sec * 500), 1, 255);
    int toff = clamp( (int) (time_flash_off_sec * 500), 1, 255);
    return send_order("89", int5_to_hex(r, g, b, ton, toff));
  }

  //////////////////////////////////////////////////////////////////////////////

  //! "YYYY/MM/DD-NN" where NN is the day's number version
  inline std::string get_version() {
    int year, month, day, number;
    if (!get_value_int4("14", year, month, day, number))
      return "";
    std::ostringstream ans;
    ans << "20" << std::setw(2) << std::setfill('0') << year
        << "/"  << std::setw(2) << std::setfill('0') << month
        << "/"  << std::setw(2) << std::setfill('0') << day
        << "-"  << std::setw(2) << std::setfill('0') << number;
    return ans.str();
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg vol 0~9
  inline bool set_volume(uint vol) {
    return send_order("06", int_to_hex(247 + clamp(vol, 0, 7) )); // 0xF7­~0xFE for volume
  }

  //////////////////////////////////////////////////////////////////////////////

  //! in 0-7
  inline uint get_volume() {
    return get_value_int1("16");
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
  static const inline std::string int5_to_hex
  (uint i1, uint i2, uint i3, uint i4, uint i5, uint ndigits = 2) {
    std::ostringstream ans;
    ans << std::setw(ndigits) << std::setfill('0') << std::hex
        << i1 << i2 << i3 << i4 << i5;
    return ans.str();
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool send_order(const std::string & handle, const std::string & param = "") {
    // sudo gatttool -i hci1 -b D0:39:72:B7:AF:66 --char-write -a 0x0013 -n 0602
    std::ostringstream order;
    order << "gatttool -i " << _hci_device << " -b " << _address;
    order << " --char-write -a 0x0013 -n " << handle << param;
    DEBUG_PRINT("order:'%s'\n", order.str().c_str());
    return system(order.str().c_str());
  }

  //////////////////////////////////////////////////////////////////////////////

  // https://stackoverflow.com/questions/13490977/convert-hex-stdstring-to-unsigned-char
  static const inline unsigned int hex2int(const std::string & char_pair){
    std::stringstream convertStream;
    convertStream << std::hex << char_pair;
    unsigned int buffer;
    convertStream >> std::hex >> buffer;
    return buffer;
  }

  static const inline char hex2ascii(const std::string & char_pair){
    return static_cast<unsigned char>(hex2int(char_pair));
  }

  inline std::string get_value(const std::string & handle) {
    // timeout .5 gatttool -i hci1 -b D0:39:72:B7:AF:66 --char-write-req -a 0x0013 -n 14 --listen | grep -m 1 "value:"
    // determine what we should wait for
    assert(handle.size() == 2);
    int handle_char1 = handle[0];
    int handle_char2 = handle[1];
    std::string keyword = "value: " + int_to_hex(handle_char1) + ' ' + int_to_hex(handle_char2) + ' ';
    // build order
    std::ostringstream order;
    order << "timeout .5 gatttool -i " << _hci_device << " -b " << _address;
    order << " --char-write-req -a 0x0013 -n " << handle << " --listen";
    // https://superuser.com/questions/402979/kill-program-after-it-outputs-a-given-line-from-a-shell-script
    order << "| grep -m 1 \"" << keyword << "\"";
    DEBUG_PRINT("order:'%s'\n", order.str().c_str());
    std::string cmd_output = exec_system_get_output(order.str().c_str());
    DEBUG_PRINT("output:'%s'\n", cmd_output.c_str());
    // typical output:'Notification handle = 0x000e value: 31 34 30 45 30 33 31 36 30 32
    //'
    // parse line
    StringUtils::find_and_replace(cmd_output, "\n", "");
    if (cmd_output.empty()) {
      printf("Mip::get_value('%s'): got less than two lines\n",
             handle.c_str());
      return "";
    }
    std::string::size_type dots_pos = cmd_output.find(keyword);
    if (dots_pos == std::string::npos) {
      printf("Mip::get_value('%s'): could not find keyword 'value:'\n",
             handle.c_str());
      return "";
    }
    std::string hex = cmd_output.substr(dots_pos + keyword.size());
    // convert each pair of hex bytes
    std::ostringstream ans;
    unsigned int npairs = hex.size() / 3;
    for (int i = 0; i < npairs; ++i)
      ans << hex2ascii(hex.substr(3*i, 2));
    DEBUG_PRINT("Mip::get_value('%s'): hex:'%s' -> ans:'%s'\n",
                handle.c_str(), hex.c_str(), ans.str().c_str());
    return ans.str();
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool get_value_intn(const std::string & handle, unsigned int n, std::vector<int> & ans) {
    ans = std::vector<int>(n, ERROR);
    std::string value_str = get_value(handle);
    if (value_str.length() != n*2) {
      printf("Mip::get_value_int('%s', n=%i): ans '%s' of incorrect size\n",
             handle.c_str(), n, value_str.c_str());
      return false;
    }
    for (int i = 0; i < n; ++i)
      ans[i] = hex2int(value_str.substr(i*2, 2));
    return true;
  } // end get_value_int2()

  inline int get_value_int1(const std::string & handle) {
    std::vector<int> ans;
    get_value_intn(handle, 1, ans);
    return ans[0];
  } // end get_value_int2()

  inline bool get_value_int2(const std::string & handle, int & ans1, int & ans2) {
    std::vector<int> ans;
    bool ok = get_value_intn(handle, 2, ans);
    ans1 = ans[0]; ans2 = ans[1];
    return ok;
  } // end get_value_int2()

  inline bool get_value_int3(const std::string & handle, int & ans1, int & ans2, int & ans3) {
    std::vector<int> ans;
    bool ok = get_value_intn(handle, 3, ans);
    ans1 = ans[0]; ans2 = ans[1]; ans3 = ans[2];
    return ok;
  } // end get_value_int3()

  inline bool get_value_int4(const std::string & handle,
                             int & ans1, int & ans2, int & ans3, int & ans4) {
    std::vector<int> ans;
    bool ok = get_value_intn(handle, 4, ans);
    ans1 = ans[0]; ans2 = ans[1]; ans3 = ans[2]; ans4 = ans[3];
    return ok;
  } // end get_value_int4()

  inline bool get_value_int5(const std::string & handle,
                             int & ans1, int & ans2, int & ans3, int & ans4, int & ans5) {
    std::vector<int> ans;
    bool ok = get_value_intn(handle, 5, ans);
    ans1 = ans[0]; ans2 = ans[1]; ans3 = ans[2]; ans4 = ans[3]; ans5 = ans[4];
    return ok;
  } // end get_value_int4()

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  std::string _address, _hci_device;
}; // end class Mip


#endif // MIP_H

