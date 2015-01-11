#include <string>

// http://www.humbug.in/2014/using-gatttool-manualnon-interactive-mode-read-ble-devices/
class Mip {
public:
  Mip() {
  }

  bool connect(const std::string & address) {
    _address = address;
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
