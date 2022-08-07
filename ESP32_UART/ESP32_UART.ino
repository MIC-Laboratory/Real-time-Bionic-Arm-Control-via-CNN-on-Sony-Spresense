/*
  Pins needed:
  - TX2 -> Spresense RX
  - GND -> Spresense GND
*/

// Library for Myo Armband connection (https://www.arduino.cc/reference/en/libraries/sparthan-myo/)
#include <myo.h>
armband myo;  // Myo Armband BLE

// Dependencies needed to transmit sEMG signals
#include <HardwareSerial.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

HardwareSerial SerialPort(2); // use UART2 Serial Communication


void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  
  String sequence;
  // length should be 16
  for (int i = 0; i < length; i++) {
    String temp_semg = String(pData[i]) + "<";
    sequence = sequence + temp_semg;
  }
  SerialPort.print(sequence);
  Serial.println(sequence);
  SerialPort.print("?");

  // NOTE: returns 16 semg samples
  // first semg array of 1 sample from 8 channeles
  // second semg array of 1 sample from 8 channeles
  // [channel1, channel2, channel3, channel4, channel5, channel6, channel7, channel8, channel1, channel2, channel3, channel4, channel5, channel6, channel7, channel8]
}

const int baud_rate = 250000;
void setup()
{
  // Connect with Myo Armband
  myo.connect();
  delay(30);
  myo.set_myo_mode(0x02,     // EMG mode
                   0x00,     // IMU mode
                   0x00);    // Classifier mode
  myo.emg_notification(TURN_ON)->registerForNotify(emgCallback);

  // Just to send error debugs
  Serial.begin(baud_rate);

  SerialPort.begin(baud_rate, SERIAL_8N1, 16, 17);

}

void loop()
{
  if (!myo.connected) {
    Serial.println ("Device disconnected: reconnecting...");
    myo.connect();
    Serial.println (" - Connected");
    // [1, 3, 2, 0, 0]
    myo.set_myo_mode(0x02,     // EMG mode
                     0x00,     // IMU mode
                     0x00);    // Classifier mode

    myo.emg_notification(TURN_ON)->registerForNotify(emgCallback);
  }
}
