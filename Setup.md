# 1. Setup for ESP32 devkit v1
    - Install arduino-esp32:

        1. Start Arduino IDE.
        2. On top left hand side, select "Preferences" under "File". (OR: run command CTRL + COMMA)
        3. Enter link below into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
            Link: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
        4. Click "OK" and exit.
        5. On top left hand side, select "Board" under "Tools".
        6. Select "Board Manager" located to the right hand side of "Board".
        7. Find "ESP32 Arduino" and select "ESP32 Dev Module".
    
    - Install sparthan-myo:

        1. Start Arduino IDE.
        2. On top left hand side, select "Manage Libraries" under "Tools". (OR: run command CTRL + SHIFT + I)
        3. Type "Sparthan" in the search box and select "Sparthan Myo"
        4. Click "Install", wait upon installation to complete and exit.