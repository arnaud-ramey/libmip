/*!
  \file        gattmip.h
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2015/2/22

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

The MIP library, made for controlling the WowWee MiP robot
and accessing its sensor readings.
Implements most of the commands understood by the WowWee MiP robot.
Cf https://github.com/WowWeeLabs/MiP-BLE-Protocol/blob/master/MiP-Protocol.md

This implementation is based on the Bluetooth Low Energy (BTLE) protocol,
and wraps C GATT commands, such as gatt_connect() and gatt_write_cmd().

A sample application could be:
////////////////////////////////////////////////////////////////////////////////
int main() {
  GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
  Mip mip;
  mip.connect(main_loop, bluetooth_mac2device(YOUR_DEVICE_MAC).c_str(), YOUR_MIP_MAC);
  g_main_loop_run(main_loop);
  g_main_loop_unref(main_loop);
  return 0;
}
////////////////////////////////////////////////////////////////////////////////

Or, for doing stuff in the main loop
(thanks // https://stackoverflow.com/questions/23737750/glib-usage-without-mainloop):
////////////////////////////////////////////////////////////////////////////////
int main() {
  GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
  Mip mip;
  mip.connect(main_loop, bluetooth_mac2device(YOUR_DEVICE_MAC).c_str(), YOUR_MIP_MAC);
  for (int i = 0; i < 10; ++i) { // iterate a few times to connect well
    // ... call Mip functions here
    mip.pump_up_callbacks();
    usleep(50E3);
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
 */
#ifndef Mip_H
#define Mip_H

extern "C" {
#include "libgatt/src/btio.h"
#include "libgatt/src/uuid.h"
#include "libgatt/src/gattrib.h"
#include "libgatt/src/gatt.h"
}
#include <unistd.h>
#include <stdlib.h>
#include <math.h> // fabs
// C++
#include <sstream>
#include <vector>
#include <iomanip>      // std::setfill, std::setw

#include "mipcommands.h"

//#define DEBUG_PRINT(...)   {}
#define DEBUG_PRINT(...)   printf(__VA_ARGS__)
#define RAD2DEG 57.2957795130823208768
#define DEG2RAD 0.01745329251994329577

class Mip {
public:
  //! a minimalistic data structure for chest led info
  struct ChestLed {
    int r, g, b, time_flash_on_sec, time_flash_off_sec;
    inline std::string to_string() const {
      std::ostringstream out;
      out << '(' << r << ',' << g << ',' << b << "), flash:on"
          <<  time_flash_on_sec << "s, off:" << time_flash_off_sec;
      return out.str();
    }
  }; // end struct ChestLed
  //! 0=off, 1=on, 2=blink slow, 3=blink fast
  struct HeadLed {
    int l1, l2, l3, l4;
    static const char* led2str(const int l) {
      switch (l) {
        case 0: return "off";
        case 1: return "on";
        case 2: return "blink_slow";
        case 3: return "blink_fast";
        default:  return "error";
      }
    }
    inline std::string to_string() const {
      std::ostringstream out;
      out << '[' << led2str(l1) << ';' << led2str(l2) << ';'
          << led2str(l3) << ';' << led2str(l4) << ']';
      return out.str();
    }
  }; // end struct HeadLed

  //////////////////////////////////////////////////////////////////////////////

  //! ctor
  Mip() {
    // free possibly busy bluetooth devices
    if (system("rfkill unblock all"))
      printf("Could not free possibly busy bluetooth devices! Keep fingers crossed\n");
    _is_connected = false;
    // default values
    _handle_read = 0x000e;
    _handle_write = 0x13;
    // default values
    _volume = ERROR;
    _game_mode = ERROR;
    _battery_voltage = ERROR;
    _status = ERROR;
    _weight = ERROR;
    _software_version = ERROR;
    _chest_led.r = _chest_led.g = _chest_led.b = ERROR;
    _chest_led.time_flash_on_sec = _chest_led.time_flash_off_sec = ERROR;
    _head_led.l1 = _head_led.l2 = _head_led.l3 = _head_led.l4 = ERROR;
    _odometer_reading_m = ERROR;
    _gesture_detect = ERROR;
    _gesture_or_radar_mode = ERROR;
    _radar_response = ERROR;
    _shake_detected = ERROR;
  }

  //////////////////////////////////////////////////////////////////////////////

  inline void set_main_loop(GMainLoop *main_loop) {
    // connect with GLib - iterate a few times to connect well
    _main_loop = main_loop;
    _context = g_main_loop_get_context(main_loop);
    int ntry = 0;
    while(ntry++ < 100 && !_is_connected) {
      pump_up_callbacks();
      usleep(50E3);
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! Connect with a given Bluetooth Low Energy (BTLE) device
   *  to a MiP robot, identified by its MAC.
   * \example connect("hci0", "D0:39:72:B7:AF:66")
   * \param device_name
   *  The Bluetooth Low Energy (BTLE) device name.
   *  This parameter corresponds to gatttool parameter -i :
   *    "Specify local adapter interface", "hciX"
   *
   *    You can obtain the list of devices by running in a terminal
   *  $ hciconfig -a
   *    The Bluetooth Low Energy (BTLE) devices use Bluetooth 4.0 and
   *    can be identified by the line
   *    "HCI Version: 4.0"
   *
   *  If you want to use a device thanks to its MAC instead of its device name,
   *    use bluetooth_mac2device()
   * \param mic_mac
   *  The MiP mac address.
   *  This parameter corresponds to gatttool parameter -b :
   *    "Specify remote Bluetooth address", "MAC"
   *  You can get the list of devices by running in a terminal
   *  $ sudo hcitool -i hciX lescan
   *  where hciX is your Bluetooth Low Energy (BTLE) device
   *  \return true if the connection was a success
   */
  bool connect(GMainLoop *main_loop, const char* device_name, const char* mip_mac) {
    // -t : "Set LE address type. Default: public", "[public | random]"
    const char *dst_type = "public",
        // -l : "Set security level. Default: low", "[low | medium | high]"
        *sec_level = "low";
    GError* error = NULL;
    GIOChannel* iochannel = gatt_connect(device_name, mip_mac, dst_type, sec_level, 0, 0,
                                         Mip::connect_cb, this, &error);
    if (iochannel == NULL) {
      printf("Error in gatt_connect('%s'->'%s'): '%s'\n",
             device_name, mip_mac, error->message);
      return false;
    }
    set_main_loop(main_loop);
    if (!_is_connected) {
      printf("Error in gatt_connect('%s'->'%s'): connect_cb() not called!\n",
             device_name, mip_mac);
      return false;
    }
    DEBUG_PRINT("gatt_connect('%s'->'%s') succesful\n", device_name, mip_mac);
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////

  inline void pump_up_callbacks() {
    g_main_context_iteration(_context, false);
  }
  inline void pump_up_callbacks(unsigned int ntimes) {
    for (int time = 0; time < ntimes; ++time) {
      pump_up_callbacks();
      usleep(50E3);
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg sound_idx Sound file index (1~106) - Send 105 to stop playing
  inline bool play_sound(uint sound_idx) {
    DEBUG_PRINT("play_sound(%i)\n", sound_idx);
    // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
    return send_order1(CMD_PLAY_SOUND, clamp(sound_idx, 1, 106));
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg distance_m in meters, no speed control
  inline bool distance_drive(double distance_m,
                             double angle_rad) {
    bool backward = (distance_m < 0);
    int angle_deg = rad2deg_norm(angle_rad, -360, 360);
    bool ccw = (angle_deg < 0);
    int distance_cm = clamp(fabs(distance_m*100.), 0, 255);
    DEBUG_PRINT("distance_cm:%i, angle_deg:%i\n", distance_cm, angle_deg);
    // BYTE 1 : Forward: 0X00 or Backward: 0X01
    // BYTE 2 : Distance (cm): 0x00­0xFF
    // BYTE 3 : Turn Clockwise: 0X00 or Anti­-clockwise: 0X01
    // BYTE 4 : Turn Angle(High byte): 0x00~0x01
    // BYTE 5 : Turn Angle(Low byte): 0x00~0xFF
    return send_order5(CMD_DISTANCE_DRIVE, backward, distance_cm,
                       ccw, abs(angle_deg)/256, abs(angle_deg)%256);
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~30 (-30~0 to go backwards)
   *  \arg time_s in seconds, in (0~1.78) */
  inline bool time_drive(double speed, double time_s) {
    int time_7ms = clamp(time_s * 1000/7, 0, 255);
    if (speed > 0)
      return send_order2(CMD_DRIVE_FORWARD_WITH_TIME,
                         clamp(speed, 0, 30), time_7ms);
    return send_order2(CMD_DRIVE_BACKWARD_WITH_TIME,
                       clamp(-speed, 0, 30), time_7ms);
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~24 (-24~0 to go backwards)
   *  \arg angle_rad in radians, > 0 for CCW, < 0 for CW */
  inline bool angle_drive(double speed, double angle_rad) {
    // BYTE 1 : Angle in intervals of 5 degrees (0~255) - Angle = Byte1 Value * 5
    int angle_deg = rad2deg_norm(angle_rad, -1275, 1275);
    if (angle_deg < 0) // CCW
      return send_order2(0x73, clamp(speed, 0, 24), fabs(angle_deg/5));
    return send_order2(0x74, clamp(-speed, 0, 24), fabs(angle_deg/5));
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 1~64 (-64~1 to go backwards)  */
  inline bool continuous_drive_linear(double lin_speed) {
    lin_speed = clamp(lin_speed, -64, 64);
    if (lin_speed > 32) // 33 ~ 64 => crazy Fw:0x81(slow)~­0xA0(fast) = 129 ~ 160
      return send_order1(CMD_CONTINUOUS_DRIVE, 96 + lin_speed);
    if (lin_speed > 0) // 1 ~ 32 => Fw:0x01(slow)~­0x20(fast)
      return send_order1(CMD_CONTINUOUS_DRIVE, lin_speed);
    if (lin_speed > -32) // -1 ~ -32 => Bw:0x21(slow)~0x40(fast) = 33 ~ 64
      return send_order1(CMD_CONTINUOUS_DRIVE, 32 - lin_speed);
    // -33 -> -64 => crazy Bw:0xA1(slow)~0xC0(fast) = 161 ~ 192
    return send_order1(CMD_CONTINUOUS_DRIVE, 128 - lin_speed);
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~64 to turn CCW (-64~0 to turn CW)  */
  inline bool continuous_drive_angular(double ang_speed) {
    ang_speed = clamp(ang_speed, -64, 63);
    if (ang_speed > 32) // 33 ~ 64 => crazy Left spin:0xE1(slow)~0xFF(fast) = 225 - 255
      return send_order1(CMD_CONTINUOUS_DRIVE, 192 + ang_speed);
    if (ang_speed > 0) // 1 ~ 32 => Left spin:0x61(slow)~0x80(fast) = 97 ~ 128
      return send_order1(CMD_CONTINUOUS_DRIVE, 96 + ang_speed);
    if (ang_speed > -32) // -1 ~ -32 => right spin:0x41(slow)~0x60(fast) = 65 ~ 96
      return send_order1(CMD_CONTINUOUS_DRIVE, 64 - ang_speed);
    // -33 -> -64 => crazy right spin:0xC1(slow)~0xE0(fast) = 193 ~ 224
    return send_order1(CMD_CONTINUOUS_DRIVE, 160 - ang_speed);
  }

  /*!
   *  \arg lin_speed in 1~64 (-64~1 to go backwards)
   *  \arg ang_speed in 0~64 to turn CCW (-64~0 to turn CW)
  */
  inline bool continuous_drive(double lin_speed, double ang_speed) {
    printf("continuous_drive(%g, %g)\n", lin_speed, ang_speed);
    int param1, param2;
    lin_speed = (int) clamp(lin_speed, -64, 64);
    if (lin_speed > 32) // 33 ~ 64 => crazy Fw:0x81(slow)~­0xA0(fast) = 129 ~ 160
      param1 = 96 + lin_speed;
    else if (lin_speed >= 0) // 1 ~ 32 => Fw:0x01(slow)~­0x20(fast)
      param1 = lin_speed;
    else if (lin_speed > -32) // -1 ~ -32 => Bw:0x21(slow)~0x40(fast) = 33 ~ 64
      param1 = 32 - lin_speed;
    else  // -33 -> -64 => crazy Bw:0xA1(slow)~0xC0(fast) = 161 ~ 192
      param1 = 128 - lin_speed;

    ang_speed = (int) clamp(ang_speed, -64, 63);
    if (ang_speed > 32) // 33 ~ 64 => crazy Left spin:0xE1(slow)~0xFF(fast) = 225 - 255
      param2 = 192 + ang_speed;
    else if (ang_speed > 0) // 1 ~ 32 => Left spin:0x61(slow)~0x80(fast) = 97 ~ 128
      param2 = 96 + ang_speed;
    else if (ang_speed == 0)
      param2 = 0;
    else if (ang_speed > -32) // -1 ~ -32 => right spin:0x41(slow)~0x60(fast) = 65 ~ 96
      param2 = 64 - ang_speed;
    else  // -33 -> -64 => crazy right spin:0xC1(slow)~0xE0(fast) = 193 ~ 224
      param2 = 160 - ang_speed;
    return send_order2(CMD_CONTINUOUS_DRIVE, param1, param2);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see GameMode enum
  inline bool set_game_mode(const GameMode & mode) {
    return send_order1(0x82, mode);
  }
  //! \return true if the request has been correctly sent to the robot
  inline bool request_game_mode() { return send_order0(0x82); }
  //! \see GameMode enum
  inline GameMode get_game_mode() { return _game_mode; }
  //! \see GameMode enum
  inline const char* get_game_mode2str() {
    return game_mode2str(get_game_mode());
  }

  //////////////////////////////////////////////////////////////////////////////

  //! stop the robot motion
  inline bool stop() { return send_order0(0x77); }

  //////////////////////////////////////////////////////////////////////////////

  //! \return true if the request has been correctly sent to the robot
  inline bool request_battery_voltage() { return send_order0(CMD_MIP_STATUS); }
  //! between 4.0V and 6.4V, or < 0 if error
  inline double get_battery_voltage() { return _battery_voltage; }
  //! in 0-100, or -1 if error
  inline int get_battery_percentage() {
    double voltage = get_battery_voltage();
    return (voltage < 0 ? ERROR : (voltage-4.0) / 2.4 * 100);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \return true if the request has been correctly sent to the robot
  inline bool request_status() { return send_order0(CMD_MIP_STATUS); }
  //! \see Status enum
  inline Status get_status() { return _status; }
  //! \see Status enum
  inline const char* get_status2str() {
    return status2str(get_status());
  }

  //////////////////////////////////////////////////////////////////////////////

  //! this command is not really clear...
  inline bool up() { return send_order1(0x23, 2); }

  //////////////////////////////////////////////////////////////////////////////

  //! \return true if the request has been correctly sent to the robot
  inline bool request_weight_update() { return send_order0(0x81); }
  //! \return angle with vertical, in [-45, 45], or -1 if ERROR.
  inline int get_weight_update() { return _weight; }

  //////////////////////////////////////////////////////////////////////////////

  //! r,g,b in [0, 255]
  inline bool set_chest_LED(const int & r, const int & g, const int & b) {
    return send_order3(CMD_SET_CHEST_LED, r, g, b);
  }
  //! r,g,b in [0, 255],
  inline bool set_chest_LED(const ChestLed & l) {
    int ton = clamp( (int) (l.time_flash_on_sec * 500), 1, 255);
    int toff = clamp( (int) (l.time_flash_off_sec * 500), 1, 255);
    return send_order5(CMD_FLASH_CHEST_LED, l.r, l.g, l.b, ton, toff);
  }
  //! \return true if the request has been correctly sent to the robot
  inline bool request_chest_LED() { return send_order0(0x83); }
  //! \return r,g,b in [0, 255]
  inline ChestLed get_chest_LED() { return _chest_led; }

  //////////////////////////////////////////////////////////////////////////////

  //! \param HeadLed l: for each of the four leds, 0=off,1=on,2=blink_slow,3=blink_fast
  inline bool set_head_LED(const HeadLed & l) {
    return send_order4(CMD_SET_HEAD_LED, l.l1, l.l2, l.l3, l.l4);
  }
  //! \return true if the request has been correctly sent to the robot
  inline bool request_head_LED() { return send_order0(CMD_HEAD_LED); }
  //! \return HeadLed l: for each of the four leds, 0=off,1=on,2=blink_slow,3=blink_fast
  inline HeadLed get_head_LED() { return _head_led; }

  //////////////////////////////////////////////////////////////////////////////

  //! \return true if the request has been correctly sent to the robot
  inline bool request_odometer_reading() { return send_order0(CMD_ODOMETER_READING); }
  //! \return odometry in meters
  inline double get_odometer_reading() { return _odometer_reading_m; }

  //////////////////////////////////////////////////////////////////////////////

  //! \return last gesture detected - \see Gesture enum
  inline Gesture get_gesture_detect() { return _gesture_detect; }
  //! \return last gesture detected - \see Gesture enum
  inline const char* get_gesture_detect2str() {
    return gesture2str(get_gesture_detect());
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see GestureOrRadarMode enum
  inline bool set_gesture_or_radar_mode(GestureOrRadarMode mode) {
    return send_order1(CMD_SET_GESTURE_OR_RADAR_MODE, mode);
  }
  //! \return true if the request has been correctly sent to the robot
  inline bool request_gesture_or_radar_mode() {
    return send_order0(CMD_RADAR_MODE_STATUS);
  }
  //! \see GestureOrRadarMode enum
  inline GestureOrRadarMode get_gesture_or_radar_mode() {
    return _gesture_or_radar_mode;
  }
  //! \see GestureOrRadarMode enum
  inline const char* get_gesture_or_radar_mode2str() {
    return gesture_or_radar_mode2str(get_gesture_or_radar_mode());
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see RadarResponse enum
  inline RadarResponse get_radar_response() { return _radar_response; }
  //! \see GestureOrRadarMode enum
  inline const char* get_radar_response2str() {
    return radar_response2str(get_radar_response());
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \return true if the request has been correctly sent to the robot
  inline bool request_software_version() {
    return send_order0(CMD_MIP_SOFTWARE_VERSION);
  }
  //! \return "YYYY/MM/DD-NN" where NN is the day's number version
  inline std::string get_software_version() { return _software_version; }

  //////////////////////////////////////////////////////////////////////////////

  //! \return true if the request has been correctly sent to the robot
  inline bool request_hardware_version() {
    return send_order0(CMD_MIP_HARDWARE_INFO);
  }
  //! \return "VV-HH", where VV is the voice chip version and HH is the hardware version
  inline std::string get_hardware_version() { return _hardware_version; }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg vol (0~7)
  inline bool set_volume(uint vol) {
    return send_order1(0x06, 247 + clamp(vol, 0, 7) ); // 0xF7­~0xFE for volume
  }
  //! \return true if the request has been correctly sent to the robot
  inline bool request_volume() { return send_order0(CMD_MIP_VOLUME); }
  //! \return volume in 0-7
  inline unsigned int get_volume() { return _volume; }

  //////////////////////////////////////////////////////////////////////////////

  //! extend this function to add behaviours upon reception of a notification
  virtual void notification_post_hook(MipCommand cmd, const std::vector<int> & values) {
  }

protected:

  //////////////////////////////////////////////////////////////////////////////

  //! store notification result in Mip class fields
  inline void store_results(MipCommand cmd, const std::vector<int> & values) {
    unsigned int nvalues = values.size();
    if (cmd == CMD_GET_CURRENT_MIP_GAME_MODE && nvalues == 1)
      _game_mode = values[0];
    else if (cmd == CMD_MIP_STATUS && nvalues == 2) {
      // 0x4D = 77 = 4.0V, 0x7C = 124 = 6.4V
      _battery_voltage = 4.0 + (values[0]-77)*2.4/47;
      _status = values[1];
    }
    else if (cmd == CMD_REQUEST_WEIGHT_UPDATE && nvalues == 1)
      // 0xD3 = 211 = (-­45 degree) ~­ 0x2D = 45 = (+45 degree)
      // 0xD3 = 211 (max) ~ 0xFF = 255 (min) is holding the weight on the front
      // 0x00 = 0 (min) ~ 0x2D = 45 (max) is holding the weight on the back
      // in other words:0 -> 0, 45 -> 45, 255 -> -1, 211 -> -45
      _weight = (values[0] < 100 ? values[0] : 256 - values[0]);
    else if (cmd == CMD_MIP_SOFTWARE_VERSION && nvalues == 5) {
      _chest_led.r = values[0];
      _chest_led.g = values[1];
      _chest_led.b = values[2];
      _chest_led.time_flash_on_sec = values[3];
      _chest_led.time_flash_off_sec = values[4];
    }
    else if (cmd == CMD_HEAD_LED && nvalues == 4) {
      _head_led.l1 = values[0];
      _head_led.l2 = values[1];
      _head_led.l3 = values[2];
      _head_led.l4 = values[3];
    }
    else if (cmd == CMD_ODOMETER_READING && nvalues == 4) {
      // BYTE 1 & 2 & 3 & 4 : Distance, Byte1 is highest byte
      // ((0~4294967296)/48.5) cm
      // 1 cm=48.5 units,
      // 0xFFFFFFFF=4294967295=88556026.7cm
      double value = values[3]
                     + 255 * values[2]
                     + 255 * 255 * values[1]
                     + 255 * 255 * 255 * values[0];
      double dist_cm = value / 48.5;
      _odometer_reading_m = dist_cm * .01;
    }
    else if (cmd == CMD_GESTURE_DETECT && nvalues == 1)
      _gesture_detect = values[0];
    else if (cmd == CMD_RADAR_MODE_STATUS && nvalues == 1)
      _gesture_or_radar_mode = values[0];
    else if (cmd == CMD_RADAR_RESPONSE && nvalues == 1)
      _radar_response = values[0];
    else if (cmd == CMD_SHAKE_DETECTED && nvalues == 1)
      _shake_detected = values[0];
    else if (cmd == CMD_MIP_SOFTWARE_VERSION && nvalues == 4) {
      std::ostringstream vstr;
      vstr << "20" << std::setw(2) << std::setfill('0') << values[0] // year
           << "/"  << std::setw(2) << std::setfill('0') << values[1] // month
           << "/"  << std::setw(2) << std::setfill('0') << values[2] // day
           << "-"  << std::setw(2) << std::setfill('0') << values[3]; // version
      _software_version = vstr.str();
    }
    else if (cmd == CMD_MIP_HARDWARE_INFO && nvalues == 2) {
      std::ostringstream vstr;
      vstr << std::setw(2) << std::setfill('0') << values[0] // voice chip
           << "-"
           << std::setw(2) << std::setfill('0') << values[1];
      _hardware_version = vstr.str();
    }
    else if (cmd == CMD_MIP_VOLUME && nvalues == 1)
      _volume = values[0];
    else // unknown command -> return
      return;
    notification_post_hook(cmd, values);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! low-level GATT order send
  inline bool send_order(uint8_t *value, int vlen) {
    return gatt_write_cmd(_attrib, _handle_write, value, vlen,
                          NULL,
                          // Mip::notify_cb,
                          this);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! send_order() versions for fixed number of parameters
  inline bool send_order0(uint8_t cmd) {
    DEBUG_PRINT("send_order0(0x%02x=%s)\n", cmd, cmd2str(cmd));
    uint8_t value_arr[1] = {cmd};
    return send_order(value_arr, 1);
  }
  inline bool send_order1(uint8_t cmd, uint8_t param1) {
    DEBUG_PRINT("send_order1(0x%02x=%s, params:%i=0x%02x)\n",
                cmd, cmd2str(cmd), param1, param1);
    uint8_t value_arr[2] = {cmd, param1};
    return send_order(value_arr, 2);
  }
  inline bool send_order2(uint8_t cmd, uint8_t param1, uint8_t param2) {
    DEBUG_PRINT("send_order2(0x%02x=%s, params:%i=0x%02x, %i=0x%02x)\n",
                cmd, cmd2str(cmd), param1, param1, param2, param2);
    uint8_t value_arr[3] = {cmd, param1, param2};
    return send_order(value_arr, 3);
  }
  inline bool send_order3(uint8_t cmd, uint8_t param1, uint8_t param2, uint8_t param3) {
    DEBUG_PRINT("send_order3(0x%02x=%s, params:%i=0x%02x, %i=0x%02x, %i=0x%02x)\n",
                cmd, cmd2str(cmd), param1, param1, param2, param2, param3, param3);
    uint8_t value_arr[4] = {cmd, param1, param2, param3};
    return send_order(value_arr, 4);
  }
  inline bool send_order4(uint8_t cmd, uint8_t param1, uint8_t param2,
                          uint8_t param3, uint8_t param4) {
    DEBUG_PRINT("send_order4(0x%02x=%s, params:%i=0x%02x, %i=0x%02x, %i=0x%02x, %i=0x%02x)\n",
                cmd, cmd2str(cmd), param1, param1, param2, param2,
                param3, param3, param4, param4);
    uint8_t value_arr[5] = {cmd, param1, param2, param3, param4};
    return send_order(value_arr, 5);
  }
  inline bool send_order5(uint8_t cmd, uint8_t param1, uint8_t param2,
                          uint8_t param3, uint8_t param4, uint8_t param5) {
    DEBUG_PRINT("send_order5(0x%02x=%s, params:%i=0x%02x, %i=0x%02x, %i=0x%02x, %i=0x%02x, %i=0x%02x)\n",
                cmd, cmd2str(cmd), param1, param1, param2, param2, param3, param3,
                param4, param4, param5, param5);
    uint8_t value_arr[6] = {cmd, param1, param2, param3, param4, param5};
    return send_order(value_arr, 6);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! clamp a value between boundaries
  inline static int clamp(int x, int min, int max) {
    return (x < min ? min : x > max ? max : x);
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! convert a hex value into ints
   https://stackoverflow.com/questions/13490977/convert-hex-stdstring-to-unsigned-char */
  inline static unsigned int hex2int(const std::string & char_pair){
    std::stringstream convertStream;
    convertStream << std::hex << char_pair;
    unsigned int buffer;
    convertStream >> std::hex >> buffer;
    return buffer;
  }

  //////////////////////////////////////////////////////////////////////////////

  //! convert an angle in radians into an angle in degrees, clamping it in given boundaries
  inline static double rad2deg_norm(const double & angle_rad,
                                    double angle_min = 0,
                                    double angle_max = 360) {
    int angle_deg = angle_rad * RAD2DEG;
    while (angle_deg < angle_min)    angle_deg+=360;
    while (angle_deg >= angle_max) angle_deg-=360;
    return angle_deg;
  }

  //////////////////////////////////////////////////////////////////////////////

  //! the events handler callback
  static void events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data) {
    //DEBUG_PRINT("events_handler()\n");
    uint16_t handle, i;
    handle = att_get_u16(&pdu[1]);
    switch (pdu[0]) {
      case ATT_OP_HANDLE_NOTIFY:
        DEBUG_PRINT("Notification handle = 0x%04x: ", handle);
        break;
      case ATT_OP_HANDLE_IND:
        DEBUG_PRINT("Indication   handle = 0x%04x: ", handle);
        break;
      default:
        printf("Invalid opcode %i\n", pdu[0]);
        return;
    }

    std::ostringstream hex_ans_stream;
    //DEBUG_PRINT("values: ");
    for (i = 3; i < len; i++) {
      hex_ans_stream << (char) pdu[i];
      //DEBUG_PRINT("'%c'=%i=0x%02x ", pdu[i], pdu[i], pdu[i]);
    }
    // DEBUG_PRINT("\n");
    std::string hex_ans = hex_ans_stream.str();
    // command - the first 2 chars are the command number
    MipCommand cmd = hex2int(hex_ans.substr(0, 2)); // in decimal base
    // convert each pair of values from hex to int
    unsigned int npairs = (hex_ans.size()-2) / 2;
    std::vector<int> values(npairs);
    for (unsigned int i = 0; i < npairs; ++i)
      values[i] = hex2int(hex_ans.substr(2+i*2, 2));
    std::ostringstream values_str;
    for (unsigned int i = 0; i < npairs; ++i)
      values_str << values[i] << ";";
    DEBUG_PRINT("cmd:0x%02x=%s, values:'%s'\n",
                cmd, cmd2str(cmd), values_str.str().c_str());

    Mip* this_ = (Mip*) user_data;
    this_->store_results(cmd, values);
  } // end events_handler();

  //////////////////////////////////////////////////////////////////////////////

  //! the GATT connect callback
  static void connect_cb(GIOChannel *io, GError *err, gpointer user_data) {
    DEBUG_PRINT("connect_cb()\n");
    if (err) {
      g_printerr("%s\n", err->message);
      return;
    }
    Mip* this_ = (Mip*) user_data;
    this_->_attrib = g_attrib_new(io);
    this_->_is_connected = true;
    // register the callback
    // g_attrib_register(GAttrib *attrib, guint8 opcode, guint16 handle,
    //              GAttribNotifyFunc func, gpointer user_data, GDestroyNotify notify)
    g_attrib_register(this_->_attrib, ATT_OP_HANDLE_NOTIFY, this_->_handle_read,
                      Mip::events_handler, this_,
                      NULL);
    g_attrib_register(this_->_attrib, ATT_OP_HANDLE_IND, this_->_handle_read,
                      Mip::events_handler, this_,
                      NULL);
  } // end connect_cb();

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  //! GLib stuff
  GAttrib *_attrib;
  GMainLoop *_main_loop;
  GMainContext * _context;
  bool _is_connected;
  //! handles for reading and writing parameters from/to the robot
  int _handle_read, _handle_write;
  //! in 0-7
  unsigned int _volume;
  //! \see GameMode enum
  GameMode _game_mode;
  //! between 4.0V and 6.4V, or < 0 if error
  double _battery_voltage;
  //! \see Status enum
  Status _status;
  //! angle with vertical, in [-45, 45], or -1 if ERROR.
  int _weight;
  //! "YYYY/MM/DD-NN" where NN is the day's number version
  std::string _software_version;
  //! "VV-HH", where VV is the voice chip version and HH is the hardware version
  std::string _hardware_version;
  //! \see ChestLed structure
  ChestLed _chest_led;
  //! \see HeadLed structure
  HeadLed _head_led;
  //! meters
  double _odometer_reading_m;
  //! \see Gesture enum
  Gesture _gesture_detect;
  //! \see GestureOrRadarMode enum
  GestureOrRadarMode _gesture_or_radar_mode;
  //! \see RadarResponse enum
  RadarResponse _radar_response;
  //! 1 when shaken
  int _shake_detected;
}; // end class Mip
#endif // Mip_H
