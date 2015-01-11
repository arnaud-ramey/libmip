/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010  Nokia Corporation
 *  Copyright (C) 2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "lib/uuid.h"
extern "C" {
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "btio/btio.h"
#include "gatttool.h"
}

static GMainLoop *event_loop;

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data) {
  GAttrib *attrib;
  uint16_t mtu;
  uint16_t cid;
  GError *gerr = NULL;

  if (err) {
    g_printerr("%s\n", err->message);
    //got_error = TRUE;
    g_main_loop_quit(event_loop);
  }

  bt_io_get(io, &gerr, BT_IO_OPT_IMTU, &mtu,
            BT_IO_OPT_CID, &cid, BT_IO_OPT_INVALID);

  if (gerr) {
    g_printerr("Can't detect MTU, using default: %s",
               gerr->message);
    g_error_free(gerr);
    mtu = ATT_DEFAULT_LE_MTU;
  }

  if (cid == ATT_CID)
    mtu = ATT_DEFAULT_LE_MTU;

  attrib = g_attrib_new(io, mtu);

//  if (opt_listen)
//    g_idle_add(listen_start, attrib);

//  operation(attrib);
}

int main() {
  const char* src = "hci0";
  const char* dst = "D0:39:72:B7:AF:66";
  const char* dst_type = "public";
  const char* sec_level = "low";
  int mtu = 0;
  int psm = 0;
  GError *gerr = NULL;
  //gatt_connect();
  // GIOChannel *gatt_connect(const char *src, const char *dst,
  //                            const char *dst_type, const char *sec_level,
  //                            int psm, int mtu, BtIOConnect connect_cb,
  //                            GError **gerr);
  GIOChannel *chan = gatt_connect(src, dst, dst_type, sec_level,
                                  psm, mtu, connect_cb, &gerr);
  if (chan == NULL) {
    g_printerr("%s\n", gerr->message);
    //got_error = TRUE;
    g_clear_error(&gerr);
  }
}
