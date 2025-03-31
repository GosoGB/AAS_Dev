#ifndef MELESC_CONSTANTS_H
#define MELESC_CONSTANTS_H

#include <Arduino.h>

enum MitsuPLCSeries {
  QL_SERIES = 0,
  iQR_SERIES = 1
};

enum MitsuPLCRemoteControl {
  RUN = 0,
  STOP,
  PAUSE,
  CLEAR,
  RESET,
  CPU_MODEL
};

enum MitsuRemoteControlMode {
  DONT_EXECUTE_FORCIBLY = 0x01,
  EXECUTE_FORCIBLY = 0x03
};

enum MitsuRemoteControlClearMode {
  DO_NOT_CLEAR = 0x01,
  CLEAR_ONLY_OUTSIDE_LATCH_RANGE = 0x02,
  ALL_CLEAR = 0x03
};

enum MitsuDeviceType {
  SM,
  SD,
  X,
  Y,
  M,
  L,
  F,
  V,
  B,
  D,
  W,
  TS,
  TC,
  TN,
  LTS,
  LTC,
  LTN,
  STS,
  STC,
  STN,
  LSTS,
  LSTC,
  LSTN,
  CS,
  CC,
  CN,
  LCS,
  LCC,
  LCN,
  SB,
  SW,
  DX,
  DY,
  Z,
  LZ
};

const char DeviceTypeQL[40][3] = {"SM", "SD", "X*", "Y*", "M*", "L*",
                                  "F*", "V*", "B*", "D*", "W*",
                                  "TS", "  ", "  ", "  ", "  ",
                                  "  ", "SS", "SC", "SN", "  ",
                                  "  ", "  ", "CS", "CC", "CN",
                                  "  ", "  ", "  ", "SB", "SW",
                                  "DX", "DY", "Z*", "  ", "R*",
                                  "ZR"};

const char DeviceTypeiQR[40][5] = {"SM**", "SD**", "X***", "Y***", "M***", "L***",
                                   "F***", "V***", "B***", "D***", "W***",
                                   "TS**", "TC**", "TN**", "LTS*", "LTC*",
                                   "LTN*", "STS*", "STC*", "STN*", "LSTS",
                                   "LSTC", "LSTN", "CS**", "CC**", "CN**",
                                   "LCS*", "LCC*", "LCN*", "SB**", "SW**",
                                   "DX**", "DY**", "Z***", "LZ**", "R***",
                                   "ZR**"};

#endif // MELESC_CONSTANTS_H
