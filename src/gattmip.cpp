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
#include "libgatt/src/utils.h"
#include "libgatt/src/uuid.h"
#include "libgatt/src/gattrib.h"
#include "libgatt/src/gatt.h"
}
#include <unistd.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data) {
  printf("connect_cb()\n");
  if (err) {
    g_printerr("%s\n", err->message);
    return;
  }
  GAttrib *attrib;
  attrib = g_attrib_new(io);
  // GAttrib *attrib, uint16_t handle, uint8_t *value, int vlen,
  // GDestroyNotify notify, gpointer user_data
  int handle = 13;
  uint8_t value[2] = {6, 2};
  size_t len = 2;
  gatt_write_cmd(attrib, handle, value, len);
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  printf("main()\n");
  // free possibly busy bluetooth devices
  system("rfkill unblock all");

  // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602

  // gatt_connect(const char *src,  const char *dst,
  // const char *dst_type, const char *sec_level,
  // int psm, int mtu, BtIOConnect connect_cb,
  //GError **gerr)
  // -i : "Specify local adapter interface", "hciX"
  const char* src = "hci0", // "00:1A:7D:DA:71:11" - hciconfig
      // -b : "Specify remote Bluetooth address", "MAC"
      *dst = "D0:39:72:B7:AF:66",
      // -t : "Set LE address type. Default: public", "[public | random]"
      *dst_type = "public",
      // -l : "Set security level. Default: low", "[low | medium | high]"
      *sec_level = "low";
  GError* error = NULL;
  GIOChannel* iochannel = gatt_connect(src, dst, dst_type, sec_level, 0, 0,
                           connect_cb, &error);
  if (iochannel == NULL) {
    printf("Error in gatt_connect('%s'->'%s'): '%s'\n",
           src, dst, error->message);
    return -1;
  }
  printf("gatt_connect('%s'->'%s') succesful\n", src, dst);
  sleep(1);
  return 0;
}

