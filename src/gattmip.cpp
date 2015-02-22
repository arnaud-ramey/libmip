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
// C++
#include <sstream>
#include <vector>
#include <iomanip>      // std::setfill, std::setw

class GATTMip {
public:
  enum CommandCode {
    CMD_SOUND = 0x06,
    CMD_VERSION = 0x14,
    CMD_VOLUME = 0x16
  };

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
    return send_order1(CMD_SOUND, clamp(sound_idx, 1, 106));
  }

  //////////////////////////////////////////////////////////////////////////////

  inline bool     request_volume() { return send_order0(CMD_VOLUME); }
  inline unsigned int get_volume() { return _volume; }

  //////////////////////////////////////////////////////////////////////////////

  inline bool request_version() { return send_order0(CMD_VERSION); }

protected:

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

  //////////////////////////////////////////////////////////////////////////////

  inline bool send_order1(uint8_t cmd, uint8_t param1) {
    printf("send_order1(0x%02x, params:0x%02x)\n", cmd, param1);
    uint8_t value_arr[2] = {cmd, param1};
    return send_order(value_arr, 2);
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

  //! store notification result in GATTMip class fields
  inline void store_results(unsigned int cmd, const std::vector<int> & values) {
    unsigned int nvalues = values.size();
    if (cmd == CMD_VOLUME && nvalues == 1)
      _volume = values[0];
  }

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

    // get volume
    uint8_t value1[1];
    value1[0] = 0x16;
    // GAttrib *attrib, uint16_t handle, uint8_t *value, int vlen,
    // GDestroyNotify notify, gpointer user_data
    gatt_write_cmd(this_->_attrib, this_->_handle_write, value1, 1,
                   GATTMip::notify_cb, this_);

    // get version
    value1[0]= 0x14;
    gatt_write_cmd(this_->_attrib, this_->_handle_write, value1, 1,
                   GATTMip::notify_cb, this_);

    // play some sound
    uint8_t value2[2] = {0x6, 0x10};
    gatt_write_cmd(this_->_attrib, this_->_handle_write, value2, 2,
                   GATTMip::notify_cb, this_);
    printf("end of connect_cb()\n");
  } // end connect_cb();

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  GAttrib *_attrib;
  int _handle_read, _handle_write;
  unsigned int _volume;
}; // end class GATTMip

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  printf("main()\n");
  // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
  GATTMip mip;
  // hci MAC "00:1A:7D:DA:71:11, check- with hciconfig
  mip.connect("hci0", "D0:39:72:B7:AF:66");

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
      mip.request_version();
    //    mip.play_sound(sound_idx); ++sound_idx;
    printf("volume:%i\n", mip.get_volume());

    // https://stackoverflow.com/questions/23737750/glib-usage-without-mainloop
    printf("g_main_context_iteration()\n");
    g_main_context_iteration(context, false);
    usleep(50E3);
    //sleep(1);
  }
#endif
  return 0;
}

