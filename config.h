#ifndef __CONFIG_H__
#define __CONFIG_H__

#define ARCH_CGEA13 true
#define BCM_SUPPORTS_AMBIENT_LIGHTING false
bool linBusEnabled = false;

// LIN
const int AMBIENT_LIN_DATA_LEN = 8;
const uint8_t AMBIENT_LIN_PID_SET_COLOR = 0x37;

// CAN
#if ARCH_CGEA13
const uint16_t HSCAN_PID_APIM_LIGHTING_REQUEST = 0x3DA;
const uint16_t HSCAN_PID_BCM_LIGHTING_RESPONSE = 0x3E3;

const int reqMsgColorIdx = 0;
const int reqMsgBrtIdx = 1;

const int respMsgColorIdx = 5;
const int respMsgBrtIdx = 6;
#endif

const int redLedPin = 10;
const int greenLedPin = 11;
const int blueLedPin = 12;

#endif