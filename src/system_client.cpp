#include "exec_system_get_output.h"

// http://www.humbug.in/2014/using-gatttool-manualnon-interactive-mode-read-ble-devices/
class Mip {
public:
  Mip() {
  }

  bool connect(const std::string & address) {
    _address = address;
  }

  bool play_sound() {
     // sudo gatttool ­i hci1 ­b D0:39:72:B7:AF:66
  }

protected:
  std::string _address;
}; // end class Mip

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  Mip mip;
  return 0;
}
