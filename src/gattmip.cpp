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
#define RAD2DEG 0.01745329251994329577
#define DEG2RAD 57.2957795130823208768

class GATTMip {
public:
  GATTMip() {
    // free possibly busy bluetooth devices
    system("rfkill unblock all");
    _handle_read = 0x000e;
    _handle_write = 0x13;
  }

  //////////////////////////////////////////////////////////////////////////////

  /*!
   * \brief connect
   * \param src
   *  corresponds to gatttool parameter -i :
   *    "Specify local adapter interface", "hciX"
   * \param dst
   *  corresponds to gatttool parameter -b :
   *    "Specify remote Bluetooth address", "MAC"
   */
  bool connect(const char* src, const char* dst) {
    // -t : "Set LE address type. Default: public", "[public | random]"
    const char *dst_type = "public",
        // -l : "Set security level. Default: low", "[low | medium | high]"
        *sec_level = "low";
    GError* error = NULL;
    GIOChannel* iochannel = gatt_connect(src, dst, dst_type, sec_level, 0, 0,
                                         GATTMip::connect_cb, this, &error);
    if (iochannel == NULL) {
      printf("Error in gatt_connect('%s'->'%s'): '%s'\n",
             src, dst, error->message);
      return false;
    }
    printf("gatt_connect('%s'->'%s') succesful\n", src, dst);
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \arg sound_idx Sound file index (1~106) - Send 105 to stop playing
  inline bool play_sound(uint sound_idx) {
    printf("play_sound(%i)\n", sound_idx);
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
    printf("distance_cm:%i\n", distance_cm);
    // BYTE 1 : Forward: 0X00 or Backward: 0X01
    // BYTE 2 : Distance (cm): 0x00­0xFF
    // BYTE 3 : Turn Clockwise: 0X00 or Anti­-clockwise: 0X01
    // BYTE 4 : Turn Angle(High byte): 0x00~0x01
    // BYTE 5 : Turn Angle(Low byte): 0x00~0xFF
    return send_order5(CMD_DISTANCE_DRIVE, backward, distance_cm,
                       ccw, angle_deg/256, angle_deg%256);
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~30 (-30~0 to go backwards)
   *  \arg time_s in seconds */
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
  inline bool continuous_drive_linear(double speed) {
    speed = clamp(speed, -64, 64);
    if (speed > 32) // 33 ~ 64 => crazy Fw:0x81(slow)~­0xA0(fast) = 129 ~ 160
      return send_order1(0x78, 96 + speed);
    if (speed > 0) // 1 ~ 32 => Fw:0x01(slow)~­0x20(fast)
      return send_order1(0x78, speed);
    if (speed > -32) // -1 ~ -32 => Bw:0x21(slow)~0x40(fast) = 33 ~ 64
      return send_order1(0x78, 32 - speed);
    // -33 -> -64 => crazy Bw:0xA1(slow)~0xC0(fast) = 161 ~ 192
    return send_order1(0x78, 128 - speed);
  }

  //////////////////////////////////////////////////////////////////////////////

  /*! \arg speed in 0~64 to turn CCW (-64~0 to turn CW)  */
  inline bool continuous_drive_angular(double speed) {
    speed = clamp(speed, -64, 63);
    if (speed > 32) // 33 ~ 64 => crazy Left spin:0xE1(slow)~0xFF(fast) = 225 - 255
      return send_order1(0x78, 192 + speed);
    if (speed > 0) // 1 ~ 32 => Left spin:0x61(slow)~0x80(fast) = 97 ~ 128
      return send_order1(0x78, 96 + speed);
    if (speed > -32) // -1 ~ -32 => right spin:0x41(slow)~0x60(fast) = 65 ~ 96
      return send_order1(0x78, 64 - speed);
    // -33 -> -64 => crazy right spin:0xC1(slow)~0xE0(fast) = 193 ~ 224
    return send_order1(0x78, 160 - speed);
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool stop() {
    return send_order0(0x77);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see GameMode enum
  inline bool set_game_mode(const GameMode & mode) {
    return send_order1(0x82, mode);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see GameMode enum
  inline GameMode request_game_mode() {
    return send_order0(0x82);
  }

  //! \see GameMode enum
  inline const char* request_game_mode2str() {
    return game_mode2str(request_game_mode());
  }

  //////////////////////////////////////////////////////////////////////////////

  //! between 4.0V and 6.4V, or < 0 if error
  inline double request_battery_voltage() {
    int voltage_int, status;
    if (!send_order2(0x79, voltage_int, status))
      return ERROR;
    // 0x4D = 77 = 4.0V, 0x7C = 124 = 6.4V
    return 4.0 + (voltage_int-77)*2.4/47;
  }

  //! in 0-100, or -1 if error
  inline int request_battery_percentage() {
    double voltage = request_battery_voltage();
    return (voltage < 0 ? ERROR : (voltage-4.0) / 2.4 * 100);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \see Status enum
  inline Status request_status() {
    int voltage_int, status;
    return (send_order2(0x79, voltage_int, status) ? status : ERROR);
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool request_up() {
    return send_order1(0x23, 2);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! \return angle with vertical, in [-45, 45], or -1 if ERROR.
  inline int request_weight_update() {
    int weight_int = send_order0(0x81);
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
  inline bool request_chest_LED(int & r, int & g, int & b, int & flashing1, int & flashing2) {
    if (!send_order5(0x83, r, g, b, flashing1, flashing2))
      return false;
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////

  //! r,g,b in [0, 255]
  inline bool set_chest_LED(const int & r, const int & g, const int & b) {
    return send_order3(0x84, r, g, b);
  }

  //////////////////////////////////////////////////////////////////////////////

  //! r,g,b in [0, 255],
  inline bool set_chest_LED(const int & r, const int & g, const int & b,
                            const double & time_flash_on_sec, const double & time_flash_off_sec) {
    int ton = clamp( (int) (time_flash_on_sec * 500), 1, 255);
    int toff = clamp( (int) (time_flash_off_sec * 500), 1, 255);
    return send_order5(0x89, r, g, b, ton, toff);
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool request_software_version() { return send_order0(CMD_MIP_SOFTWARE_VERSION); }

  //! "YYYY/MM/DD-NN" where NN is the day's number version
  inline std::string request_version() {
    int year, month, day, number;
    if (!send_order4(0x14, year, month, day, number))
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
    return send_order1(0x06, 247 + clamp(vol, 0, 7) ); // 0xF7­~0xFE for volume
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool     request_volume() { return send_order0(CMD_MIP_VOLUME); }
  //! in 0-7
  inline unsigned int get_volume() { return _volume; }

protected:

  unsigned int _volume;
  GameMode _game_mode;

  //////////////////////////////////////////////////////////////////////////////

  //! store notification result in GATTMip class fields
  inline void store_results(unsigned int cmd, const std::vector<int> & values) {
    unsigned int nvalues = values.size();
    if (cmd == CMD_MIP_VOLUME && nvalues == 1)
      _volume = values[0];
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool send_order(uint8_t *value, int vlen) {
    return gatt_write_cmd(_attrib, _handle_write, value, vlen, GATTMip::notify_cb, this);
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool send_order0(uint8_t cmd) {
    printf("send_order0(0x%02x)\n", cmd);
    uint8_t value_arr[1] = {cmd};
    return send_order(value_arr, 1);
  }
  inline bool send_order1(uint8_t cmd, uint8_t param1) {
    printf("send_order1(0x%02x, params:0x%02x)\n", cmd, param1);
    uint8_t value_arr[2] = {cmd, param1};
    return send_order(value_arr, 2);
  }
  inline bool send_order2(uint8_t cmd, uint8_t param1, uint8_t param2) {
    printf("send_order2(0x%02x, params:0x%02x, 0x%02x)\n", cmd, param1, param2);
    uint8_t value_arr[3] = {cmd, param1, param2};
    return send_order(value_arr, 3);
  }
  inline bool send_order3(uint8_t cmd, uint8_t param1, uint8_t param2, uint8_t param3) {
    printf("send_order3(0x%02x, params:0x%02x, 0x%02x, 0x%02x)\n", cmd,
           param1, param2, param3);
    uint8_t value_arr[4] = {cmd, param1, param2, param3};
    return send_order(value_arr, 4);
  }
  inline bool send_order4(uint8_t cmd, uint8_t param1, uint8_t param2,
                          uint8_t param3, uint8_t param4) {
    printf("send_order4(0x%02x, params:0x%02x, 0x%02x, 0x%02x, 0x%02x)\n", cmd,
           param1, param2, param3, param4);
    uint8_t value_arr[5] = {cmd, param1, param2, param3};
    return send_order(value_arr, 5);
  }
  inline bool send_order5(uint8_t cmd, uint8_t param1, uint8_t param2,
                          uint8_t param3, uint8_t param4, uint8_t param5) {
    printf("send_order5(0x%02x, params:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x)\n", cmd,
           param1, param2, param3, param4, param5);
    uint8_t value_arr[6] = {cmd, param1, param2, param3};
    return send_order(value_arr, 6);
  }

  //////////////////////////////////////////////////////////////////////////////

  static const inline uint clamp(uint x, uint min, uint max) {
    return (x < min ? min : x > max ? max : x);
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

  //////////////////////////////////////////////////////////////////////////////

  static const inline std::string int_to_hex(uint i1, uint ndigits = 2) {
    std::ostringstream ans;
    ans << std::setw(ndigits) << std::setfill('0') << std::hex << i1;
    return ans.str();
  }

  //////////////////////////////////////////////////////////////////////////////

  static const inline uint rad2deg_norm(const double & angle_rad,
                                        const double angle_min = 0,
                                        const double angle_max = 360) {
    int angle_deg = angle_rad * RAD2DEG;
    while (angle_deg < angle_min)    angle_deg+=360;
    while (angle_deg >= angle_max) angle_deg-=360;
    return angle_deg;
  }

  //////////////////////////////////////////////////////////////////////////////

  static void notify_cb(gpointer user_data) {
    printf("notify_cb()\n");
  }

  //////////////////////////////////////////////////////////////////////////////

  static void events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
  {
    printf("events_handler()\n");
    uint16_t handle, i;
    handle = att_get_u16(&pdu[1]);
    switch (pdu[0]) {
      case ATT_OP_HANDLE_NOTIFY:
        printf("Notification handle = 0x%04x: ", handle);
        break;
      case ATT_OP_HANDLE_IND:
        printf("Indication   handle = 0x%04x: ", handle);
        break;
      default:
        printf("Invalid opcode\n");
        return;
    }

    std::ostringstream hex_ans_stream;
    //printf("values: ");
    for (i = 3; i < len; i++) {
      hex_ans_stream << (char) pdu[i];
      //printf("'%c'=%i=0x%02x ", pdu[i], pdu[i], pdu[i]);
    }
    // printf("\n");
    std::string hex_ans = hex_ans_stream.str();
    // command - the first 2 chars are the command number
    unsigned int cmd = hex2int(hex_ans.substr(0, 2)); // in decimal base
    // convert each pair of values from hex to int
    unsigned int npairs = (hex_ans.size()-2) / 2;
    std::vector<int> values(npairs);
    for (int i = 0; i < npairs; ++i)
      values[i] = hex2int(hex_ans.substr(2+i*2, 2));
    std::ostringstream values_str;
    for (int i = 0; i < npairs; ++i)
      values_str << values[i] << ";";
    printf("cmd:0x%02x, values:'%s'\n", cmd, values_str.str().c_str());

    GATTMip* this_ = (GATTMip*) user_data;
    this_->store_results(cmd, values);
  } // end events_handler();

  //////////////////////////////////////////////////////////////////////////////

  static void connect_cb(GIOChannel *io, GError *err, gpointer user_data) {
    printf("connect_cb()\n");
    if (err) {
      g_printerr("%s\n", err->message);
      return;
    }
    GATTMip* this_ = (GATTMip*) user_data;

    this_->_attrib = g_attrib_new(io);
    // register the callback
    // g_attrib_register(GAttrib *attrib, guint8 opcode, guint16 handle,
    //              GAttribNotifyFunc func, gpointer user_data, GDestroyNotify notify)
    g_attrib_register(this_->_attrib, ATT_OP_HANDLE_NOTIFY,
                      this_->_handle_read,
                      GATTMip::events_handler, this_, GATTMip::notify_cb);
    g_attrib_register(this_->_attrib, ATT_OP_HANDLE_IND,
                      this_->_handle_read,
                      GATTMip::events_handler, this_, GATTMip::notify_cb);
  } // end connect_cb();

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  GAttrib *_attrib;
  int _handle_read, _handle_write;
}; // end class GATTMip

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  printf("main()\n");
  // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
  GATTMip mip;
  // hci MAC "00:1A:7D:DA:71:11, check- with hciconfig
  mip.connect(
        //"hci0",
        "hci1",
        "D0:39:72:B7:AF:66");

  // pump up the callbacks!
  GMainLoop *event_loop;
  event_loop = g_main_loop_new(NULL, FALSE);
#if 0
  g_main_loop_run(event_loop);
  g_main_loop_unref(event_loop);
#else
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
#endif
  return 0;
}

