/*
    Description:
      Robotic Arm Control Utility Function
    Date: Summer 2022

    1. D06 = PWM Thumb 
    2. D05 = PWM Index
    3. D03 = PWM Pinky (& Middle & Ring)

    - Parallex Servo (Index) = 4 ~ 6 V 15 ~ 200 mAh
    - J- DEAL Micro Servo 9g SG90 = 4.8 ~ 6 V 100mA ~ 250mA

*/

// Import dependency for Bionic Arm control
#include <Servo.h>

// Setup angles for Bionic Arm's finger control
#define flexing_thumb 150
#define flexing_index 170
#define flexing_pinky 180

#define clenching_thumb 50
#define clenching_index 50
#define clenching_pinky 0

#define half_clenching_thumb 120
#define half_clenching_index 90
#define half_clenching_pinky 90


// up to twelve servo objects can be created on most boards; only 4 for Spresense EXT board
Servo myservo_thumb;  // create servo object to control a thumb servo
Servo myservo_index;  // create servo object to control a index servo
Servo myservo_pinky;  // create servo object to control a pinky servo

int pred = 0;  // variable to store the outputted prediction

char pos_thumb = 0;  // variable to store the thumb servo position
char pos_index = 0;  // variable to store the index servo position
char pos_pinky = 0;  // variable to store the pinky servo position

// functions for different gestures

void rest() {
  /*
    Purpose:
      Perform Rest gesture on Bionic Arm
  */
  myservo_thumb.write(flexing_thumb);
  myservo_index.write(flexing_index);
  myservo_pinky.write(flexing_pinky);
}

void fist() {
  /*
    Purpose:
      Perform Fist gesture on Bionic Arm
  */
  myservo_thumb.write(clenching_thumb);
  myservo_index.write(clenching_index);
  myservo_pinky.write(clenching_pinky);
}

void thumb_up() {
  /*
    Purpose:
      Perform Thumb Up gesture on Bionic Arm
  */
  myservo_thumb.write(flexing_thumb);
  myservo_index.write(clenching_index);
  myservo_pinky.write(clenching_pinky);
}

void ok_sign() {
  /*
    Purpose:
      Perform Ok Sign gesture on Bionic Arm
  */
  myservo_thumb.write(clenching_thumb);
  myservo_index.write(clenching_index);
  myservo_pinky.write(flexing_pinky);
}