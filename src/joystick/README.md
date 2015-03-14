# joystick++

![stable](http://badges.github.io/stability-badges/dist/stable.svg)

A minimal C++ object-oriented API onto joystick devices under Linux.

# usage

Create an instance of `Joystick`:

    Joystick joystick();

Ensure that it was found and that we can use it:

    if (!joystick.isFound())
    {
      printf("open failed.\n");
      // hmm
    }

Sample events from the `Joystick`:

    JoystickEvent event;
    if (joystick.sample(&event))
    {
      // use 'event'
    }

# example

You might run this in a loop:

    while (true)
    {
      // Restrict rate
      usleep(1000);

      // Attempt to sample an event from the joystick
      JoystickEvent event;
      if (joystick.sample(&event))
      {
        if (event.isButton())
        {
          printf("Button %u is %s\n", event.number, event.value == 0 ? "up" : "down");
        }
        else if (event.isAxis())
        {
          printf("Axis %u is at position %d\n", event.number, event.value);
        }
      }
    }

This produces something similar to:

    Button 1 is up
    Button 2 is down
    Axis 0 is at position 122
    Axis 1 is at position -11
    Axis 2 is at position 9796
    Axis 3 is at position -13850

# options

You can specify the particular joystick by id:

    Joystick js0(0);
    Joystick js1(1);
    
Or provide a specific device name:

    Joystick js0("/dev/input/js0");

# license

Released under [LGPL v3](http://www.gnu.org/copyleft/lesser.html).

Copyright [Drew Noakes](http://drewnoakes.com) 2013-2014.
