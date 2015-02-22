/*!
  \file
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
This file lists all the commands understood by the WoWWee MiP robot.
Cf https://github.com/WowWeeLabs/MiP-BLE-Protocol/blob/master/MiP-Protocol.md
 */
#ifndef MIPCOMMANDS_H
#define MIPCOMMANDS_H

enum MipCommand {
    CMD_PLAY_SOUND=0x06,
    CMD_SET_MIP_POSITION=0x08,
    CMD_DISTANCE_DRIVE=0x70,
    CMD_DRIVE_FORWARD_WITH_TIME=0x71,
    CMD_DRIVE_BACKWARD_WITH_TIME=0x72,
    CMD_TURN_LEFT_BY_ANGLE=0x73,
    CMD_TURN_RIGHT_BY_ANGLE=0x74,
    CMD_CONTINUOUS_DRIVE=0x78,
    CMD_SET_GAME_MODE=0x76,
    CMD_GET_CURRENT_MIP_GAME_MODE=0x82,
    CMD_CURRENT_MIP_GAME_MODE=0x82,
    CMD_STOP=0x77,
    CMD_REQUEST_MIP_STATUS=0x79,
    CMD_MIP_STATUS=0x79,
    CMD_MIP_GET_UP=0x23,
    CMD_REQUEST_WEIGHT_UPDATE=0x81,
    CMD_WEIGHT_UPDATE=0x81,
    CMD_REQUEST_CHEST_LED=0x83,
    CMD_CHEST_LED=0x83,
    CMD_SET_CHEST_LED=0x84,
    CMD_FLASH_CHEST_LED=0x89,
    CMD_SET_HEAD_LED=0x8A,
    CMD_REQUEST_HEAD_LED=0x8B,
    CMD_HEAD_LED=0x8B,
    CMD_READ_ODOMETER=0x85,
    CMD_ODOMETER_READING=0x85,
    CMD_REST_ODOMETER=0x86,
    CMD_GESTURE_DETECT=0x0A,
    CMD_SET_GESTURE_OR_RADAR_MODE=0x0C,
    CMD_GET_RADAR_MODE=0x0D,
    CMD_RADAR_MODE_STATUS=0x0D,
    CMD_RADAR_RESPONSE=0x0C,
    CMD_MIP_DETECTION_MODE=0x0E,
    CMD_REQUEST_MIP_DETECTION_MODE=0x0F,
    CMD_MIP_DETECTION_STATUS=0x0F,
    CMD_MIP_DETECTED=0x04,
    CMD_SHAKE_DETECTED=0x1A,
    CMD_IR_REMOTE_CONTROL_ENABLED=0x10,
    CMD_REQUEST_IR_CONTROL_ENABLED=0x11,
    CMD_IR_CONTROL_STATUS=0x11,
    CMD_SLEEP=0xFA,
    CMD_DISCONNECT_APP=0xFE,
    CMD_FORCE_BLE_DISCONNECT=0xFC,
    CMD_SET_USER_DATA=0x12,
    CMD_GET_USER_OR_OTHER_EEPROM_DATA=0x13,
    CMD_MIP_USER_OR_OTHER_EEPROM_DATA=0x13,
    CMD_GET_MIP_SOFTWARE_VERSION=0x14,
    CMD_MIP_SOFTWARE_VERSION=0x14,
    CMD_GET_MIP_HARDWARE_INFO=0x19,
    CMD_MIP_HARDWARE_INFO=0x19,
    CMD_SET_MIP_VOLUME=0x15,
    CMD_GET_MIP_VOLUME=0x16,
    CMD_MIP_VOLUME=0x16,
    CMD_SEND_IR_DONGLE_CODE=0x8C,
    CMD_RECEIVE_IR_DONGLE_CODE=0x03,
    CMD_CLAP_TIMES=0x1D,
    CMD_CLAP_ENABLED=0x1E,
    CMD_REQUEST_CLAP_ENABLED=0x1F,
    CMD_CLAP_STATUS=0x1F,
    CMD_DELAY_TIME_BETWEEN_TWO_CLAPS=0x20,
};

static const int ERROR = -100;

typedef int GameMode;
static const GameMode UNKNOWN = -1;
static const GameMode APP = 1;
static const GameMode CAGE = 2;
static const GameMode TRACKING = 3;
static const GameMode DANCE = 4;
static const GameMode DEFAULT_MIP_MODE = 5;
static const GameMode STACK = 6;
static const GameMode TRICK_PROGRAMMING_AND_PLAYBACK = 7;
static const GameMode ROAM_MODE = 8;

inline static const char* game_mode2str(const GameMode mode) {
  switch (mode) {
    case APP:  return "APP";
    case CAGE:  return "CAGE";
    case TRACKING:  return "TRACKING";
    case DANCE:  return "DANCE";
    case DEFAULT_MIP_MODE:  return "DEFAULT_MIP_MODE";
    case STACK:  return "STACK";
    case TRICK_PROGRAMMING_AND_PLAYBACK:  return "TRICK_PROGRAMMING_AND_PLAYBACK";
    case ROAM_MODE:  return "ROAM_MODE";
    case UNKNOWN:
    default:
      return "UNKNOWN";
  }
} // end game_mode2str()


typedef int Status;
static const Status ON_BACK = 0;
static const Status FACE_DOWN = 1;
static const Status UPRIGHT = 2;
static const Status PICKED_UP = 3; // Note:it will be sent after(connecting,falldown,pickup….)
static const Status HAND_STAND = 4;
static const Status FACE_DOWN_ON_TRAY = 5;
static const Status ON_BACK_WITH_KICKSTAND = 6;

#endif // MIPCOMMANDS_H
