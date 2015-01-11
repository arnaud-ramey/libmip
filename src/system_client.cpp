#include "exec_system_get_output.h"
#include "string_split.h"
#include "find_and_replace.h"
#include <algorithm>
#include <iomanip>      // std::setfill, std::setw
#include <unistd.h>

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

  // between 0 and 106
  inline bool play_sound(unsigned int sound_idx) {
    // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
    return send_order("06", int2hex(sound_idx));
  }

  //////////////////////////////////////////////////////////////////////////////

protected:
  static const inline std::string int2hex(unsigned int i, unsigned int ndigits = 2) {
    std::ostringstream ans;
    ans << std::setw(ndigits) << std::setfill('0') << std::hex << i;
    return ans.str();
  }

  // between 0 and 106
  inline bool send_order(const std::string & handle, const std::string & param) {
    // sudo gatttool -­i hci1 ­-b D0:39:72:B7:AF:66 --char­-write­ -a 0x0013 -n 0602
    std::ostringstream order;
    order << "gatttool -i " << _hci_device << " -b " << _address;
    order << " --char-write -a 0x0013 -n " << handle << param;
    printf("order:'%s'\n", order.str().c_str());
    return system(order.str().c_str());
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
