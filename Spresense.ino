/*
  Device: Sony Spresense Main + Extension Board

  Pins needed:
  - RX -> ESP32 TX2
  - GND -> ESP32 GND
*/

// Import utility functions to control Bionic Arm
#include "arm_control.h"

// Import tensorflow lite for on-device model inference
// - Source: https://www.tensorflow.org/lite/microcontrollers/get_started_low_level#run_inference

// provides the operations used by the interpreter to run the model.
#include "tensorflow/lite/micro/all_ops_resolver.h"
// outputs debug information.
#include "tensorflow/lite/micro/micro_error_reporter.h"
// contains code to load and run models.
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Get quantized finetuned model's weights (stored in array: model_tflite)
#include "model.h"

// Get STD and MEAN for each local myo channel
#include "std_and_mean.h";

// Number of gestures to classify
int num_class = 4;

// Init tensorflow utilites
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

// Allocate model ram
constexpr int kTensorArenaSize = 47 * 1024; // 47 kbs
uint8_t tensor_arena[kTensorArenaSize];

// Define the array to store EMG samples for current window (8x32)
float current_window[8][32];

// Tracker for current sample in current_window
int curr_idx = 0;


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
      if (semg_sample > 127)
      {
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
}


void semg_handler(float semg_arr[]) {
  /*
    Purpose:
      Insert float sEMG array containing the 16 sEMG samples to current window.
  */
  for (int i = 0; i < 8; i++) {
    current_window[i][curr_idx] = semg_arr[i];
  }
  curr_idx = curr_idx + 1;

  for (int i = 0; i < 8; i++) {
    current_window[i][curr_idx] = semg_arr[i + 8];
  }
  curr_idx = curr_idx + 1;
}


// Majority Voting Of 4
// - Perform Bionic Arm gesture based on the most frequent prediction from every 4 predicted gestures.
int majority_counter = 4;

/*
  Keep Track of predictions, vote every 4 predictions.

  0: Relax Gesture
  1: Fist Gesture
  2: Ok Sign Gesture
  3: Thumbs Up Gesture
*/
int counters[4] = {0, 0, 0, 0};

void run_pred() {
  input = interpreter->input(0);
  output = interpreter->output(0);

  // semg: 8 x 32
  int myo_channels = 8;
  int window_size = 32;

  // Input sEMG sampled to the interpretor's input pointer
  for (int i = 0; i < myo_channels; i++) {
    for (int j = 0; j < window_size; j++) {
      input->data.f[j + (i * 32)] = current_window[i][j];
    }
  }

  // Run model inference
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed");
    return;
  }

  // Get model prediction by finding the largest probability
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

  switch(p_max_idx) {
    case 0: // 0 = Relax Gesture
      counters[0] = counters[0] + 1;
      break;

    case 1: // 1 = Fist Gesture
      counters[1] = counters[1] + 1;
      break;

    case 2: // 2 = Ok Sign Gesture
      counters[2] = counters[2] + 1;
      break;

    default: // 3 = Thumbs Up Gesture
      counters[3] = counters[3] + 1;
      break
  }
}


void majority_voting(int counters[]) {
  // Vote every 4 predictions, control Bionic Arm to perform most frequently predicted gesture.
  int max_idx = 0;
  for (int i = 0; i < 3; i++) {
    if (counters[max_idx] < counters[i + 1]) {
      max_idx = i + 1;
    }
  }

  // Control Arm Here

  switch(max_idx) {
    case 0: // 0 = Relax Gesture
      rest();
      Serial.println("Rest");
      break;

    case 1: // 1 = Fist Gesture
      fist();
      Serial.println("Fist");
      break;

    case 2: // 2 = Ok Sign Gesture
      ok_sign();
      Serial.println("Ok Sign");
      break;

    default: // 3 = Thumbs Up Gesture
      thumb_up();
      Serial.println("Thumb Up");
      break
  }

  // Reset Counter
  for (int i = 0; i < 4; i++) {
    counters[i] = 0;
  }
}


void step() {
  // Move last 16 samples from current to the front before next sliding window.
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 16; j++) {
      current_window[i][j] = current_window[i][j + 16];
    }
  }

  // Reset curr_idx to prepare for next sliding window of step size 16
  curr_idx = 16;
}


void setup() {
  // Begin Serial UART communication with baud rate 250000
  int baud_rate = 250000;
  Serial2.begin(baud_rate);
  Serial.begin(baud_rate);

  myservo_thumb.attach(6); // red thumb
  myservo_index.attach(5); // green index
  myservo_pinky.attach(3); // blue middle, ring, pinkie

  rest(); // defauly Bionic Arm to Rest Gesture

  tflite::InitializeTarget();
  memset(tensor_arena, 0, kTensorArenaSize * sizeof(uint8_t));

  // Set up logging.
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure..
  model = tflite::GetModel(model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model provided is schema version " + String(model->version()) + " not equal " + "to supported version " + String(TFLITE_SCHEMA_VERSION));
    return;
  }
  else {
    Serial.println("Model version: " + String(model->version()));
  }

  // This pulls in all the operation implementations we need.
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter
  );
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();

  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }
  else {
    Serial.println("AllocateTensor() Success");
  }

  size_t used_size = interpreter->arena_used_bytes();
  Serial.println("Area used bytes: " + String(used_size));

  // Setup input and output tensors
  input = interpreter->input(0);
  output = interpreter->output(0);

  /* check input data type*/
  if (input->type != kTfLiteFloat32) {
    Serial.println("input type mismatch. expected input type is float32");
    return;
  }
  else {
    Serial.println("input type is float32");
  }
}


String str_semg = "";
void loop() {
  // Receive in Ascii
  if ((Serial2.available()) && (curr_idx < 32)) {

    String temp_semg = String(Serial2.read() - '0');

    switch(temp_semg) {
      case "15":
        // Extract sEMG array to float array of size 16.
        float *semg = NULL;
        semg = extract_semg(str_semg);
        
        if (curr_idx < 32) {
          semg_handler(semg);
        }

        if (curr_idx >= 32) {
          // Run Prediction
          run_pred();

          // Vote every 4 predictions, control Bionic Arm to perform most frequently predicted gesture.
          int total_preds = counters[0] + counters[1] + counters[2] + counters[3];
          if (total_preds == majority_counter) {
            majority_voting(counters);
          }

          // Rest counters to prepare for next sliding window of 16.
          step();
          
          str_semg = "";
        }
        break;
      
      case "12":
        // Indicates next char must be sEMG sample.
        str_semg = str_semg + "<";
        break;

      default:
        // Indicates this char is part of an sEMG sample.
        str_semg = str_semg + temp_semg;
        break;
    }
  }
}
