/*!
  \file        speed_calibration.cpp
  \author      Arnaud Ramey <arnaud.a.ramey@gmail.com>
                -- Robotics Lab, University Carlos III of Madrid
  \date        2016/1/10

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
A simple demo for the libmip library:
random walking while avoiding obstacles.
 */
#include "src/bluetooth_mac2device.h"
#include "src/gattmip.h"
#include <curses.h>
#include <sys/time.h>

class Timer {
public:
  typedef float Time;
  const static Time NOTIME = -1;
  Timer() { reset(); }
  virtual inline void reset() {
    gettimeofday(&start, NULL);
  }
  //! get the time since ctor or last reset (milliseconds)
  virtual inline Time getTimeMilliseconds() const {
    struct timeval end;
    gettimeofday(&end, NULL);
    return (Time) (// seconds
                   (end.tv_sec - start.tv_sec)
                   * 1000 +
                   // useconds
                   (end.tv_usec - start.tv_usec)
                   / 1000.f);
  }
private:
  struct timeval start;
};

inline double get_odometry_safe(Mip & mip) {
  mip.request_odometer_reading();
  double odom = mip.get_odometer_reading();
  if (odom == ERROR) {
    printf("Could not get odometry!\n");
    exit(-1);
  }
  return odom;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
  if (argc < 4) {
    printf("Synopsis: %s  LIN_SPEED ANG_SPEED NLOOPS [DEVICE_MAC] [MIP_MAC]\n", argv[0]);
    printf("  LIN_SPEED: integer in (-64~64)\n");
    printf("  ANG_SPEED: integer in (-64~64)\n");
    printf("  NLOOPS :   integer > 0\n");
    return 0;
  }
  int lin_speedi = atoi(argv[1]), ang_speedi = atoi(argv[2]), nloops = atoi(argv[3]);
  GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
  Mip mip;
  std::string device_mac = (argc >= 5 ? argv[4] : "00:1A:7D:DA:71:11"),
      mip_mac = (argc >= 6 ? argv[5] : "D0:39:72:B7:AF:66");
  if (!mip.connect(main_loop, bluetooth_mac2device(device_mac).c_str(), mip_mac.c_str())) {
    printf("Could not connect with device MAC '%s' to MIP with MAC '%s'!\n",
           device_mac.c_str(), mip_mac.c_str());
    return -1;
  }
  // now the real stuff
  double odom_begin = get_odometry_safe(mip);
  Timer timer;

  // non-blocking wait for key
  // http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
  initscr();     //in ncurses
  printw("press any key when the %i loops are over...", nloops);
  timeout(0);
  cbreak();
  while(getch() <= 0) {
    mip.continuous_drive(lin_speedi, ang_speedi);
    usleep(50 * 1000);
  }
  endwin();

  double time = timer.getTimeMilliseconds() * 1E-3;
  double odom_end = get_odometry_safe(mip), dist = odom_end - odom_begin;
  double lin_speed = dist /time, ang_speed = 2 * M_PI * nloops / time;
  printf("%g m in %g sec\n", dist, time);
  printf(" --> lin_speed: %g m/s, ang_speed: %g rad/s\n", lin_speed, ang_speed);
  return 0;
}
