# Module Setup Guide:

    1. ESP32 Devkit v1:
        - arduino-esp32: https://github.com/espressif/arduino-esp32
        - sparthan-myo: https://github.com/project-sparthan/sparthan-myo
    
    2. Sony Spresense (Main + Extension): 
        - spresense-arduino-tensorflow: https://github.com/YoshinoTaro/spresense-arduino-tensorflow

# Files Overview:
1. Folder ESP32 contains:
    - ESP32.ino:
        Baud Rate: 250000

        1. Connects to Myo Armband via. BLE and retrieve sEMG signals from it.
        2. Communicates retrieved sEMG signals to Sony Spresense via. UART Serial.

2. Folder Spresense contains:
    - Spresense.ino:
        Baud Rate: 250000

        1. Decode sEMG signals sent from ESP32 Devkit v1 via. UART Serial
        2. Perform sEMG preprocessing and run model inference z
        3. Control Bionic Arm

    - arm_control.h:

        1. Robotic Arm Control Utility Function
    
    - model.h:
        
        1. Finetuned model weights for tensorflow lite model inference
    
    - std_and_mean.h:
        
        1. Mean and Standard Deviation for each Myo channel, calculated from 7 gestues' sEMG in NinaPro DB5


![Alt text](system.jpg?raw=true "Title")