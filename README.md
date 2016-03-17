# `libmip`

`libmip` is a library for using
[WoWWee MiP](http://wowwee.com/mip) robots in C++ programs.
It has been designed as a lightweight and easy-to-use library.
It gives you the possibility to both:

  * send orders to the robot, for instance, move forward or play sounds;
  * read sensors, for instance the proximity sensor or the state of the LED lights.

Supported hardware
==================

The library is developed for the original WowWee MiP.

Supported functionalities
=========================

The library wraps all the commands released in
[the WowWee documentation](https://github.com/WowWeeLabs/MiP-BLE-Protocol/blob/master/MiP-Protocol.md).
The detailed list is copied below.

    Chest LED
    Clap Enabled
    Clap Status
    Clap times
    Continuous Drive
    Current MIP Game Mode
    Delay time between two claps
    Disconnect App
    Distance Drive
    Drive backward with Time
    Drive forward with Time
    Flash Chest LED
    Force BLE disconnect
    Gesture Detect
    Get current MIP Game Mode
    Get Mip Hardware Info
    Get Mip Software Version
    Get Mip Volume
    Get Radar Mode
    Get User Or Other Eeprom Data
    Head LED
    IR Control Status
    IR Remote Control Enabled
    Mip Detected
    MIP Detection Mode
    Mip Detection Status
    Mip Get Up
    Mip Hardware Info
    Mip Software Version
    MIP status
    MIP User Or Other Eeprom Data
    Mip Volume
    Odometer reading
    Play Sound
    Radar Mode Status
    Radar Response
    Read Odometer
    Receive IR Dongle code
    Request Chest LED
    Request Clap Enabled
    Request Head LED
    Request IR Control Enabled
    Request MIP Detection Mode
    Request MIP status
    Request weight update
    Reset Odometer
    Send IR Dongle code
    Set Chest LED
    Set Game Mode
    Set Gesture Or Radar Mode
    Set Head LED
    Set Mip Position
    Set Mip Volume
    Set User Data
    Shake Detected
    Sleep
    Stop
    Turn left by Angle
    Turn right by Angle
    Weight update

Licence
=======

LGPL v3 (GNU Lesser General Public License version 3).
See LICENCE.


Authors
=======

See the AUTHORS file.


How to build the program
=========================

See the INSTALL file.


How to use the program
======================

Check out the sample executables in the "samples" folder.

A minimalistic sample using can be:

```
#include "src/bluetooth_mac2device.h"
#include "src/gattmip.h"
int main(int argc, char** argv) {
  GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
  Mip mip;
  std::string device_mac = (argc >= 2 ? argv[1] : "00:1A:7D:DA:71:11"),
      mip_mac = (argc >= 3 ? argv[2] : "D0:39:72:B7:AF:66");
  if (!mip.connect(main_loop, bluetooth_mac2device(device_mac).c_str(), mip_mac.c_str())) {
    printf("Could not connect with device MAC '%s' to MIP with MAC '%s'!\n",
           device_mac.c_str(), mip_mac.c_str());
    return -1;
  }
  // now the real stuff
  mip.distance_drive(.5, 0);
  mip.pump_up_callbacks();
  return 0;
}
```

Or, for doing stuff in the main loop,
thanks [this link](https://stackoverflow.com/questions/23737750/glib-usage-without-mainloop):

```
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
```

Finding the MAC of your BLE device and of your MiP
==================================================

First determine the MAC address of your BLE device by running:
You can obtain the list of devices by running in a terminal

```
$ hciconfig -a
```

The Bluetooth Low Energy (BTLE) devices use Bluetooth 4.0 and
can be identified by the line
"HCI Version: 4.0"
Also note the name of your interface, for instance `hci1`.

Then obtain the MAC of your robot.
Start with resetting Bluetooth (from [ubuntu-fr.org](http://doc.ubuntu-fr.org/bluetooth#problemes_connus)) ,
then perform a LE scan

```
$ sudo rfkill unblock all
$ sudo hciconfig hci1 up
$ sudo hcitool -i hci1 lescan
D0 :39:72: B7 : AF :66 ( unknown )
D0 :39:72: B7 : AF :66 Bubi
```

Implementation
==============

`libmip` uses the Bluetooth Low Energy (BTLE, part of Bluetooth 4.0)
protocol for communication with the robot.
Your computer might not support it natively, in which case you need to
use a BTLE dongle.
Underneath, the Bluetooth commands and orders are structured as
[GATT operations]( https://en.wikipedia.org/wiki/Bluetooth_low_energy#GATT_Operations).

The commands are based on the MiP BLE protocol, released by WoWWee.
See [this link](https://github.com/WowWeeLabs/MiP-BLE-Protocol).
This implementation wraps C GATT commands, such as
`gatt_connect()` and `gatt_write_cmd()`.
The GATT library is
[libgatt](https://github.com/jacklund/libgatt),
it is simply the [BlueZ 5.7](http://www.bluez.org/) GATT/LE
code extracted into a library, embedded into the project.

The joystick sample is powered thanks to the
[joystick project of drewnoakes](https://github.com/drewnoakes/joystick).
It is a minimal C++ object-oriented API onto joystick devices under Linux.


