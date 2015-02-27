/*!
  \file        bluetooth_mac2device.h
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
Convert a bluetooth device MAC address, for instance "00:11:22:33:44:55",
 *  into a device name, for instance "hci0".
*/
#ifndef BLUETOOTH_MAC2DEVICE_H
#define BLUETOOTH_MAC2DEVICE_H

#include "find_and_replace.h"
#include "exec_system_get_output.h"
#include "string_split.h"
#include <algorithm>

/*! Convert a bluetooth device MAC address, for instance "00:11:22:33:44:55",
 *  into a device name, for instance "hci0".
 * \brief bluetooth_mac2device
 * \param mac_address
 *    the MAC address of the device we want to use.
 *
 *    You can obtain the list of devices by running in a terminal
 *  $ hciconfig -a
 *    The Bluetooth Low Energy (BTLE) devices use Bluetooth 4.0 and
 *    can be identified by the line
 *    "HCI Version: 4.0"
 * \return the device name of the device with the given MAC,
 *    or "" if the device does not exist.
 */
std::string bluetooth_mac2device(const std::string & mac_address) {
  printf("bluetooth_mac2device('%s')\n", mac_address.c_str());
  std::string result = exec_system_get_output("hciconfig");
  StringUtils::find_and_replace(result, "\n", " ");
  StringUtils::find_and_replace(result, "\t", " ");
  while (StringUtils::find_and_replace(result, "  ", " ")) {}
  //printf("result:'%s\n'\n", result.c_str());
  std::vector<std::string> words;
  StringUtils::StringSplit(result, " ", &words);
  std::vector<std::string>::const_iterator it =
      std::find(words.begin(), words.end(), mac_address)  ;
  if (it == words.end()) {
    printf("bluetooth_mac2device: no device with address'%s'\n", mac_address.c_str());
    return "";
  }
  while (it >= words.begin()) {
    if (it->size() > 3 && it->substr(0, 3) == "hci")
      return it->substr(0, 4);
    --it;
  }
  printf("bluetooth_mac2device: device with address'%s' has no 'hci' name!\n", mac_address.c_str());
  return "";
} // end bluetooth_mac2device()

#endif // BLUETOOTH_MAC2DEVICE_H

