/*
  Device: ESP32 Devkit v1

  Pins needed:
  - TX2 -> Spresense RX
  - GND -> Spresense GND
*/

// Library for Myo Armband connection
// Setup Guide / Github Repo: https://www.arduino.cc/reference/en/libraries/sparthan-myo/
#include <myo.h>
armband myo;  // Myo Armband BLE

// Dependencies needed to transmit sEMG signals
#include <HardwareSerial.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

// use UART2 Serial Communication
HardwareSerial SerialPort(2);

// Set default baud rate for UART Serial transmission.
const int baud_rate = 250000;


void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  /*
    Purpose:
      1. EMG signals handler for Myo Armband's 4 myo characteristics.
      2. Communicate received sEMG signals to Sony Spresense via. UART Serial.
  */

  // Initialize String variable sequence to encode retrieved sEMG signal before UART Serial transmission.
  String sequence;

  // length should be 16 since 2 sEMG arrays are transmitted
  for (int i = 0; i < length; i++) {
    String temp_semg = String(pData[i]) + "<";
    sequence = sequence + temp_semg;
  }

  /*
    Transmits 16 semg samples via UART Serial in String type since Serial transmits values in Ascii format.
    Sample sequence: "0<134<26<32<49<123<34<34<90<39<34<34<4<9<6<10<"   ("<" seperates each sEMG sample)
    - First sEMG array: 1st to 8th sEMG sample comes from myo sensor/channel 1, 2 ... 8
    - Second sEMG array: 9th to 16th sEMG samples comes from myo sensor/channel 1, 2 ... 8
  */
  SerialPort.print(sequence);

  // Monitor if Myo Armband retrieve sEMG signals.
  Serial.println(sequence);

  // Indicates the end of 1 sEMG handler transmission to Sony Spresense.
  SerialPort.print("?");

}

// Arduino Setup Function
void setup()
{
  // Connect with Myo Armband
  myo.connect();
  delay(30);
  myo.set_myo_mode(0x02,     // EMG mode
                   0x00,     // IMU mode
                   0x00);    // Classifier mode
  myo.emg_notification(TURN_ON)->registerForNotify(emgCallback);

  // Just to send error debugs in local Serial Monitor.
  Serial.begin(baud_rate);

  // Initialize Serial communication between Sony Spresense board.
  SerialPort.begin(baud_rate, SERIAL_8N1, 16, 17);

}


// Arduino Loop Function
void loop()
{ 
  // Always ensure Myo Armband is connected.
  if (!myo.connected) {
    Serial.println ("Device disconnected: reconnecting...");
    myo.connect();
    Serial.println (" - Connected");

    // [1, 3, 2, 0, 0]
    myo.set_myo_mode(0x02,     // EMG mode
                     0x00,     // IMU mode
                     0x00);    // Classifier mode

    // Register emgCallback function for the 4 Myo Characteristics to retrieve and send sEMG samples to Sony Spresense.
    myo.emg_notification(TURN_ON)->registerForNotify(emgCallback);
  }
}
