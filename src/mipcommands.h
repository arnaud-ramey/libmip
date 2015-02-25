/*!
  \file        mipcommands.h
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
This file lists all the commands understood by the WowWee MiP robot.
Cf https://github.com/WowWeeLabs/MiP-BLE-Protocol/blob/master/MiP-Protocol.md
 */
#ifndef MIPCOMMANDS_H
#define MIPCOMMANDS_H

static const int ERROR = -100;

////////////////////////////////////////////////////////////////////////////////
typedef int MipCommand;
static const MipCommand CMD_PLAY_SOUND=0x06;
static const MipCommand CMD_SET_MIP_POSITION=0x08;
static const MipCommand CMD_DISTANCE_DRIVE=0x70;
static const MipCommand CMD_DRIVE_FORWARD_WITH_TIME=0x71;
static const MipCommand CMD_DRIVE_BACKWARD_WITH_TIME=0x72;
static const MipCommand CMD_TURN_LEFT_BY_ANGLE=0x73;
static const MipCommand CMD_TURN_RIGHT_BY_ANGLE=0x74;
static const MipCommand CMD_CONTINUOUS_DRIVE=0x78;
static const MipCommand CMD_SET_GAME_MODE=0x76;
static const MipCommand CMD_GET_CURRENT_MIP_GAME_MODE=0x82;
static const MipCommand CMD_CURRENT_MIP_GAME_MODE=0x82;
static const MipCommand CMD_STOP=0x77;
static const MipCommand CMD_REQUEST_MIP_STATUS=0x79;
static const MipCommand CMD_MIP_STATUS=0x79;
static const MipCommand CMD_MIP_GET_UP=0x23;
static const MipCommand CMD_REQUEST_WEIGHT_UPDATE=0x81;
static const MipCommand CMD_WEIGHT_UPDATE=0x81;
static const MipCommand CMD_REQUEST_CHEST_LED=0x83;
static const MipCommand CMD_CHEST_LED=0x83;
static const MipCommand CMD_SET_CHEST_LED=0x84;
static const MipCommand CMD_FLASH_CHEST_LED=0x89;
static const MipCommand CMD_SET_HEAD_LED=0x8A;
static const MipCommand CMD_REQUEST_HEAD_LED=0x8B;
static const MipCommand CMD_HEAD_LED=0x8B;
static const MipCommand CMD_READ_ODOMETER=0x85;
static const MipCommand CMD_ODOMETER_READING=0x85;
static const MipCommand CMD_REST_ODOMETER=0x86;
static const MipCommand CMD_GESTURE_DETECT=0x0A;
static const MipCommand CMD_SET_GESTURE_OR_RADAR_MODE=0x0C;
static const MipCommand CMD_GET_RADAR_MODE=0x0D;
static const MipCommand CMD_RADAR_MODE_STATUS=0x0D;
static const MipCommand CMD_RADAR_RESPONSE=0x0C;
static const MipCommand CMD_MIP_DETECTION_MODE=0x0E;
static const MipCommand CMD_REQUEST_MIP_DETECTION_MODE=0x0F;
static const MipCommand CMD_MIP_DETECTION_STATUS=0x0F;
static const MipCommand CMD_MIP_DETECTED=0x04;
static const MipCommand CMD_SHAKE_DETECTED=0x1A;
static const MipCommand CMD_IR_REMOTE_CONTROL_ENABLED=0x10;
static const MipCommand CMD_REQUEST_IR_CONTROL_ENABLED=0x11;
static const MipCommand CMD_IR_CONTROL_STATUS=0x11;
static const MipCommand CMD_SLEEP=0xFA;
static const MipCommand CMD_DISCONNECT_APP=0xFE;
static const MipCommand CMD_FORCE_BLE_DISCONNECT=0xFC;
static const MipCommand CMD_SET_USER_DATA=0x12;
static const MipCommand CMD_GET_USER_OR_OTHER_EEPROM_DATA=0x13;
static const MipCommand CMD_MIP_USER_OR_OTHER_EEPROM_DATA=0x13;
static const MipCommand CMD_GET_MIP_SOFTWARE_VERSION=0x14;
static const MipCommand CMD_MIP_SOFTWARE_VERSION=0x14;
static const MipCommand CMD_GET_MIP_HARDWARE_INFO=0x19;
static const MipCommand CMD_MIP_HARDWARE_INFO=0x19;
static const MipCommand CMD_SET_MIP_VOLUME=0x15;
static const MipCommand CMD_GET_MIP_VOLUME=0x16;
static const MipCommand CMD_MIP_VOLUME=0x16;
static const MipCommand CMD_SEND_IR_DONGLE_CODE=0x8C;
static const MipCommand CMD_RECEIVE_IR_DONGLE_CODE=0x03;
static const MipCommand CMD_CLAP_TIMES=0x1D;
static const MipCommand CMD_CLAP_ENABLED=0x1E;
static const MipCommand CMD_REQUEST_CLAP_ENABLED=0x1F;
static const MipCommand CMD_CLAP_STATUS=0x1F;
static const MipCommand CMD_DELAY_TIME_BETWEEN_TWO_CLAPS=0x20;

////////////////////////////////////////////////////////////////////////////////
typedef int GameMode;
static const GameMode GAME_MODE_APP = 1;
static const GameMode GAME_MODE_CAGE = 2;
static const GameMode GAME_MODE_TRACKING = 3;
static const GameMode GAME_MODE_DANCE = 4;
static const GameMode GAME_MODE_DEFAULT_MIP_MODE = 5;
static const GameMode GAME_MODE_STACK = 6;
static const GameMode GAME_MODE_TRICK_PROGRAMMING_AND_PLAYBACK = 7;
static const GameMode GAME_MODE_ROAM_MODE = 8;

inline static const char* game_mode2str(const GameMode mode) {
  switch (mode) {
    case GAME_MODE_APP:  return "APP";
    case GAME_MODE_CAGE:  return "CAGE";
    case GAME_MODE_TRACKING:  return "TRACKING";
    case GAME_MODE_DANCE:  return "DANCE";
    case GAME_MODE_DEFAULT_MIP_MODE:  return "DEFAULT_MIP_MODE";
    case GAME_MODE_STACK:  return "STACK";
    case GAME_MODE_TRICK_PROGRAMMING_AND_PLAYBACK:  return "TRICK_PROGRAMMING_AND_PLAYBACK";
    case GAME_MODE_ROAM_MODE:  return "ROAM_MODE";
    case ERROR:
    default:
      return "ERROR";
  }
} // end game_mode2str()

////////////////////////////////////////////////////////////////////////////////
typedef int Status;
static const Status STATUS_ON_BACK = 0;
static const Status STATUS_FACE_DOWN = 1;
static const Status STATUS_UPRIGHT = 2;
static const Status STATUS_PICKED_UP = 3; // Note:it will be sent after(connecting,falldown,pickup….)
static const Status STATUS_HAND_STAND = 4;
static const Status STATUS_FACE_DOWN_ON_TRAY = 5;
static const Status STATUS_ON_BACK_WITH_KICKSTAND = 6;

inline static const char* status2str(const Status mode) {
  switch (mode) {
    case STATUS_ON_BACK:  return "ON_BACK";
    case STATUS_FACE_DOWN:  return "FACE_DOWN";
    case STATUS_UPRIGHT:  return "UPRIGHT";
    case STATUS_PICKED_UP:  return "PICKED_UP";
    case STATUS_HAND_STAND:  return "HAND_STAND";
    case STATUS_FACE_DOWN_ON_TRAY: return "FACE_DOWN_ON_TRAY";
    case STATUS_ON_BACK_WITH_KICKSTAND: return "ON_BACK_WITH_KICKSTAND";
    case ERROR:
    default:
      return "ERROR";
  }
} // end status2str()

////////////////////////////////////////////////////////////////////////////////
typedef int Gesture;
static const Gesture GESTURE_LEFT = 0x0A;
static const Gesture GESTURE_RIGHT = 0x0B;
static const Gesture GESTURE_CENTER_SWEEP_LEFT=0X0C;
static const Gesture GESTURE_CENTER_SWEEP_RIGHT=0X0D;
static const Gesture GESTURE_CENTER_HOLD=0X0E;
static const Gesture GESTURE_FORWARD=0X0F;
static const Gesture GESTURE_BACK=0X10;

inline static const char* gesture2str(const Gesture gest) {
  switch (gest) {
    case GESTURE_LEFT: return "LEFT";
    case GESTURE_RIGHT: return "RIGHT";
    case GESTURE_CENTER_SWEEP_LEFT: return "CENTER_SWEEP_LEFT";
    case GESTURE_CENTER_SWEEP_RIGHT: return "CENTER_SWEEP_RIGHT";
    case GESTURE_CENTER_HOLD: return "CENTER_HOLD";
    case GESTURE_FORWARD: return "FORWARD";
    case GESTURE_BACK: return "BACK";
    case ERROR:
    default:
      return "ERROR";
  }
} // end gesture2str()

////////////////////////////////////////////////////////////////////////////////
typedef int GestureOrRadarMode;
static const GestureOrRadarMode GESTUREOFF_RADAROFF = 0x00;
static const GestureOrRadarMode GESTUREON_RADAROFF = 0x02;
static const GestureOrRadarMode GESTUREOFF_RADARON = 0x04;

inline static const char* gesture_or_radar_mode2str(const GestureOrRadarMode m) {
  switch (m) {
    case GESTUREOFF_RADAROFF: return "geture OFF, radar OFF";
    case GESTUREON_RADAROFF: return "geture ON, radar OFF";
    case GESTUREOFF_RADARON: return "geture OFF, radar ON";
    case ERROR:
    default:
      return "ERROR";
  }
} // end gesture2str()

////////////////////////////////////////////////////////////////////////////////
typedef int RadarResponse;
static const RadarResponse RADAR_NO_OBJECT = 1;
static const RadarResponse RADAR_OBJECT_10TO30CM = 2;
static const RadarResponse RADAR_OBJECT_0TO10CM = 3;

inline static const char* radar_response2str(const RadarResponse r) {
  switch (r) {
    case RADAR_NO_OBJECT: return "NO_OBJECT";
    case RADAR_OBJECT_10TO30CM: return "OBJECT_10TO30CM";
    case RADAR_OBJECT_0TO10CM: return "OBJECT_0TO10CM";
    case ERROR:
    default:
      return "ERROR";
  }
} // end radar_response2str()

#endif // MIPCOMMANDS_H
