/*!
  file
  author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  date        2015/2/22

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

  odo Description of the file

section Parameters
  - b "foo"
        [string] (default: "bar")
        Description of the parameter.

section Subscriptions
  - b "/foo"
        [xxx]
        Descrption of the subscription

section Publications
  - b "~foo"
        [xxx]
        Descrption of the publication

 */

#ifndef BLUETOOTH_MAC2DEVICE_H
#define BLUETOOTH_MAC2DEVICE_H

#include "find_and_replace.h"
#include "exec_system_get_output.h"
#include "string_split.h"
#include <algorithm>

std::string bluetooth_mac2device(const std::string & address) {
  printf("bluetooth_mac2device('%s')\n", address.c_str());
  std::string result = exec_system_get_output("hciconfig");
  StringUtils::find_and_replace(result, "\n", " ");
  StringUtils::find_and_replace(result, "\t", " ");
  while (StringUtils::find_and_replace(result, "  ", " ")) {}
  //printf("result:'%s\n'\n", result.c_str());
  std::vector<std::string> words;
  StringUtils::StringSplit(result, " ", &words);
  std::vector<std::string>::const_iterator it =
      std::find(words.begin(), words.end(), address)  ;
  if (it == words.end()) {
    printf("No device with address'%s'\n", address.c_str());
    return "";
  }
  while (it >= words.begin()) {
    if (it->size() > 3 && it->substr(0, 3) == "hci")
      return it->substr(0, 4);
    --it;
  }
  printf("Device with address'%s' has no 'hci' name!\n", address.c_str());
  return "";
} // end bluetooth_mac2device()

#endif // BLUETOOTH_MAC2DEVICE_H

