#include <lin_stack.h>
#include "STM32_CAN.h"
#include "can.h"
#include "config.h"
#include "color-map.h"

HardwareSerial Serial1(PA10, PA9);
//HardwareSerial Serial2(PA3, PA2);
//lin_stack LIN2(2); // Creating LIN Stack objects, 2 - second channel
lin_stack LIN2(2); // Creating LIN Stack objects, 1 - first channel

STM32_CAN can1(CAN1, DEF);

int currentColor = 0;
uint8_t currentBrt = 0;

void setup() {
  Serial.begin(115200);

  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);

  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(blueLedPin, LOW);

  can1.begin();
  //can1.setBaudRate(250000);  //250KBPS
  can1.setBaudRate(500000);  //500KBPS
  //can1.setBaudRate(1000000);  //1000KBPS

  can1.setFilter(0, HSCAN_PID_APIM_LIGHTING_REQUEST, 0x1FFFFFFF);
  can1.setFilter(0, HSCAN_PID_BCM_LIGHTING_RESPONSE, 0x1FFFFFFF);
}

int onMessageReceived(CAN_message_t canMsg) {
  switch (canMsg.id) {
    case HSCAN_PID_APIM_LIGHTING_REQUEST: {
      int color = (int)canMsg.buf[reqMsgColorIdx];
      uint8_t intensity = canMsg.buf[reqMsgBrtIdx];

      currentColor = color;
      currentBrt = intensity;

      setLightState(color, intensity);

      #if !BCM_SUPPORTS_AMBIENT_LIGHTING
      sendResponseToApim(currentColor, currentBrt);
      #endif

      break;
    }
    #if BCM_SUPPORTS_AMBIENT_LIGHTING
    case HSCAN_PID_BCM_LIGHTING_RESPONSE: {
      int recColor = (int)canMsg.buf[respMsgColorIdx];
      uint8_t recIntensity = canMsg.buf[respMsgBrtIdx];

      Serial.print("Current state: ");
      if ((currentColor = recColor) && (currentBrt = recIntensity)) {
        Serial.print("ON Color: ");
        Serial.print(colorNames[recColor]);
        Serial.print("Intensity: ");
        Serial.println(recIntensity);
      } else {
        Serial.println("APIM Request/BCM Response Mismatch");
      }
      break;
    }
    #endif
    default: {
      return 0;
    }
  }
  return 1;
}

uint8_t* createLinMessage(int colorIndex, byte intensity) {
  static uint8_t linMsg[AMBIENT_LIN_DATA_LEN];

  uint8_t colorByte = 0xF0;
  
  if (colorIndex > 7) {
    colorIndex = 7;
  }

  if (colorIndex < 1) {
    colorIndex = 1;
  }

  colorByte = (colorByte & 0xF0) | ((colorIndex - 1) & 0x0F);

  // Assign control byte
  linMsg[0] = colorByte;
  linMsg[1] = 0x14;
  linMsg[2] = 0x88;
  linMsg[3] = 0x00;
  linMsg[4] = intensity;
  if (intensity > 0) {
    linMsg[5] = 0x57;
  } else {
    linMsg[5] = 0x49;
  }
  linMsg[6] = 0x00;
  linMsg[7] = 0x00;
  return linMsg;
}

int mapColorValue(uint8_t colorValue) {
  return map((int)colorValue, 0, 255, 0, 4095);
}

int sendResponseToApim(int colorIndex, uint8_t intensity) {
  CAN_message_t responseMsg;

  responseMsg.id = HSCAN_PID_BCM_LIGHTING_RESPONSE;
  responseMsg.len = 8;
  responseMsg.buf[respMsgColorIdx] = colorIndex;
  responseMsg.buf[respMsgBrtIdx] = intensity;
  for (int i=2; i<8; i++) {
    responseMsg.buf[i] = 0x00;
  }

  return can1.write(responseMsg);
}

int setLightState(int colorIndex, uint8_t intensity) {
  if (linBusEnabled) {
    uint8_t identifier = AMBIENT_LIN_PID_SET_COLOR;
    uint8_t* data = createLinMessage(colorIndex, intensity);
    return LIN2.write(identifier, data, sizeof(data));
  } else {
    analogWrite(redLedPin, ((intensity / 100) * mapColorValue(rgbColorMap[colorIndex][0])));
    analogWrite(greenLedPin, ((intensity / 100) * mapColorValue(rgbColorMap[colorIndex][1])));
    analogWrite(blueLedPin, ((intensity / 100) * mapColorValue(rgbColorMap[colorIndex][2])));

    return 1;
  }
}

void loop() {
  CAN_message_t canMsg;
  can1.read(canMsg);

  onMessageReceived(canMsg);
}