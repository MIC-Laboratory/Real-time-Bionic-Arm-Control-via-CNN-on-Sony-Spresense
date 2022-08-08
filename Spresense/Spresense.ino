// NOTE: Robotic Arm Control Setup
//D06 = PWM Thumb
//D05 = PWM Index
//D03 = PWM Pinky (& Middle & Ring)

// Parallex Servo (Index) = 4 ~ 6 V 15 ~ 200 mAh

//J- DEAL Micro Servo 9g SG90 = 4.8 ~ 6 V 100mA ~ 250mA

#include <Servo.h>

#define flexing_thumb 150
#define flexing_index 170
#define flexing_pinky 180

#define clenching_thumb 50
#define clenching_index 50
#define clenching_pinky 0

#define half_clenching_thumb 120
#define half_clenching_index 90
#define half_clenching_pinky 90

#define delay_var 2000

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
  myservo_thumb.write(flexing_thumb);
  myservo_index.write(flexing_index);
  myservo_pinky.write(flexing_pinky);
}

void fist() {
  myservo_thumb.write(clenching_thumb);
  myservo_index.write(clenching_index);
  myservo_pinky.write(clenching_pinky);
}

void thumb_up() {
  myservo_thumb.write(flexing_thumb);
  myservo_index.write(clenching_index);
  myservo_pinky.write(clenching_pinky);
}

void ok() {
  myservo_thumb.write(clenching_thumb);
  myservo_index.write(clenching_index);
  myservo_pinky.write(flexing_pinky);
}


/*
  Pins needed:
  - RX -> ESP32 TX2
  - GND -> ESP32 GND
*/
// Import tensorflow
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Number of gestures to classify
int num_class = 4;
// Get quantized finetuned model
#include "model.h"


// Init tensorflow utilites
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
// Allocate model ram
constexpr int kTensorArenaSize = 47 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// Get STD and MEAN for each myo channel
#include "std_and_mean.h";

// Define the current window (8x32)
float current_window[8][32];
// Tracker for current sample in window
int curr_idx = 0;


// Increase baud_rate if ARM/nuttx compiler failed

/*
  - The current window changes
*/
void semg_handler(float semg_arr[]) {
  if (curr_idx < 32) {
    for (int i = 0; i < 8; i++) {
      current_window[i][curr_idx] = semg_arr[i];
    }
    curr_idx = curr_idx + 1;
    //    Serial.print(curr_idx);
    //    Serial.print(" ");

    for (int i = 0; i < 8; i++) {
      current_window[i][curr_idx] = semg_arr[i + 8];
    }
    curr_idx = curr_idx + 1;
    //    Serial.println(curr_idx);
  }

  if (curr_idx >= 32) {
    // Run Prediction Here

    prediction();


    // Do the step 16  here.
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 16; j++) {
        current_window[i][j] = current_window[i][j + 16];
      }
    }

    curr_idx = 16;
    //    curr_idx = 0; // set 16? to achieve step 16 maybe 15 if not
  }
}

// Majority Voting Of 4
int majority_counter = 4;

/*
  0: Relax Gesture
  1: Fist Gesture
  2: Ok Sign Gesture
  3: Thumbs Up Gesture
*/
int counters[4] = {0, 0, 0, 0};

void prediction() {
  input = interpreter->input(0);
  output = interpreter->output(0);

  // semg: 8 x 32
  int myo_channels = 8;
  int window_size = 32;
  long int t1 = millis();
  // feed semg to input pointer
  for (int i = 0; i < myo_channels; i++) {
    for (int j = 0; j < window_size; j++) {
      input->data.f[j + (i * 32)] = current_window[i][j];
    }
  }

  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed");
    return;
  }

  float preds[4];
  for (int i = 0; i < num_class; i++) {
    float value = output->data.f[i];
    preds[i] = value;
  }

  // find max index
  int p_max_idx = 0;
  for (int i = 0; i < 3; i++) {
    if (preds[p_max_idx] < preds[i + 1]) {
      p_max_idx = i + 1;
    }
  }

  // Print Results

  // 0 = Relax Gesture
  if (p_max_idx == 0) {
    counters[0] = counters[0] + 1;
    //    rest();

    // 1 = Fist Gesture
  }
  else if (p_max_idx == 1) {
    counters[1] = counters[1] + 1;

    // 2 = Ok Sign Gesture
  }
  else if (p_max_idx == 2) {
    counters[2] = counters[2] + 1;

    // 3 = Thumbs Up Gesture
  }
  else {
    counters[3] = counters[3] + 1;
  }

  if (counters[0] + counters[1] + counters[2] + counters[3] == majority_counter) {
    int max_idx = 0;
    for (int i = 0; i < 3; i++) {
      if (counters[max_idx] < counters[i + 1]) {
        max_idx = i + 1;
      }
    }

    // Control Arm Here

    // 0 = Relax Gesture
    if (max_idx == 0) {
      rest();
      Serial.println("Rest");

      // 1 = Fist Gesture
    }
    else if (max_idx == 1) {
      fist();
      Serial.println("Fist");

      // 2 = Ok Sign Gesture
    }
    else if (max_idx == 2) {
      ok();
      Serial.println("Ok Sign");

    // 3 = Thumbs Up Gesture
    }
    else {
      thumb_up();
      Serial.println("Thumb Up");
    }

    // Rewrite Counter
    for (int i=0; i<4; i++){
      counters[i] = 0;  
    }
  }

}

float *extract_semg(String string_semg) {
  // semg_size should 16
  // Processing takes 1ms: "0<1<2<3<4<5<6<255<0<1<2<3<4<5<6<255<"
  static float semg[16];

  String temp_s = "";
  int t_idx = 0;
  for (int i = 0; i < string_semg.length(); i++) {
    String current_s = String(string_semg[i]);
    if (current_s.equals("<")) {
      int semg_sample = temp_s.toInt();

      // Apply 2's Complement
      if (semg_sample > 127) {
        semg_sample = semg_sample - 256;
      }

      // Apply Standarization
      int curr_myo_channel = t_idx % 8;
      float scaled_semg = (semg_sample - s_params[curr_myo_channel][0]) / s_params[curr_myo_channel][1];

      semg[t_idx] = scaled_semg;

      temp_s = "";
      t_idx = t_idx + 1;
    }
    else {
      temp_s = temp_s + current_s;
    }
  }

  return semg;
  /*TESTING
    String s = "0<1<2<3<4<5<6<255<0<1<2<3<4<5<6<255<";

    int* semg = NULL;
    semg = extract_semg(s);

    for (int i=0; i<16; i++){
      Serial.println(*(semg+i));
    }
  */
}

// MAYBE< need to reallocate tensor , rerun all code form tturoaial in loop

void setup() {
  myservo_thumb.attach(6);  // red thumb
  myservo_index.attach(5);  // green index
  myservo_pinky.attach(3);  // blue middle, ring, pinkie

  rest();

  int baud_rate = 250000;
  Serial2.begin(baud_rate);
  Serial.begin(baud_rate);

  tflite::InitializeTarget();
  memset(tensor_arena, 0, kTensorArenaSize * sizeof(uint8_t));

  // Set up logging.
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure..
  model = tflite::GetModel(model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model provided is schema version "
                   + String(model->version()) + " not equal "
                   + "to supported version "
                   + String(TFLITE_SCHEMA_VERSION));
    return;
  } else {
    Serial.println("Model version: " + String(model->version()));
  }

  // This pulls in all the operation implementations we need.
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  } else {
    Serial.println("AllocateTensor() Success");
  }

  size_t used_size = interpreter->arena_used_bytes();
  Serial.println("Area used bytes: " + String(used_size));
  // Setup input and outpu tensors
  input = interpreter->input(0);
  output = interpreter->output(0);

  /* check input */
  if (input->type != kTfLiteFloat32) {
    Serial.println("input type mismatch. expected input type is float32");
    return;
  } else {
    Serial.println("input type is float32");
  }
}

String str_semg = "";
void loop() {
  // Receive in Ascii
  // -> Important funcs
  // Serial.readString()
  // Serial.toInt()
  // Serial2.read() is 1ms, Serial2.readString() is 1000ms
  // use String(Serial2.read() - '0'); instead
  // 17 ms to process: String sequence = "255<255<255<255<255<255<255<255<255<255<255<255<255<255<255<255<";
  if (Serial2.available()) {

    if (curr_idx < 32) { // 32, 33 to test print

      String temp_semg = String(Serial2.read() - '0');

      if (temp_semg == "15") {
        float* semg = NULL;
        semg = extract_semg(str_semg);

        semg_handler(semg);

        //        for (int i = 0; i < 16; i++) {
        //          Serial.print(*(semg + i), 7); //  specify how many digits after .
        //          Serial.print(" ");
        //        }
        //        Serial.println();

        str_semg = "";

      }

      else if (temp_semg == "12") {
        str_semg = str_semg + "<";
      }

      else {
        str_semg = str_semg + temp_semg;
      }

    }
  }

}
