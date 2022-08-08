# Sony-Competition

Module Setup Guide:

    1. ESP32 Devkit v1:
        - arduino-esp32: https://github.com/espressif/arduino-esp32
        - sparthan-myo: https://github.com/project-sparthan/sparthan-myo
    
    2. Sony Spresense (Main + Extension): 
        - spresense-arduino-tensorflow: https://github.com/YoshinoTaro/spresense-arduino-tensorflow

1. Folder ESP32 contains:
    - ESP32.ino:
        Baud Rate: 250000

        1. Connects to Myo Armband via. BLE and retrieve sEMG signals from it.
        2. Communicates retrieved sEMG signals to Sony Spresense via. UART Serial.

2. Folder Spresense contains:
    