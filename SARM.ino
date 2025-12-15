#include "BluetoothSerial.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// --- BLUETOOTH SETUP ---
BluetoothSerial SerialBT;


Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN  150  
#define SERVOMAX  600  
#define SERVO_FREQ 50  


#define LOOP_DELAY 40      
#define MAX_SPEED_STEP 3.0 
#define DEADZONE 15        


#define IDX_BASE 0
#define IDX_SHOULDER 1
#define IDX_ELBOW 2
#define IDX_PITCH 3
#define IDX_ROLL 4

const int NUM_SERVOS = 5;

struct ServoConfig {
  uint8_t channel;
  int minLimit;
  int maxLimit;
  float currentAngle; // Float for smooth accumulation
};


ServoConfig armServos[NUM_SERVOS] = {
  {0,  0,   180,  90.0},  // Base
  {1,  15,  165,  90.0},  // Shoulder
  {2,  0,   150,  90.0},  // Elbow
  {3,  0,   180,  90.0},  // Wrist Pitch
  {4,  0,   180,  90.0}   // Wrist Roll
};


int inputs[NUM_SERVOS] = {0, 0, 0, 0, 0};

void setup() {
  Serial.begin(115200);


  SerialBT.begin("Mecanum_Arm"); 
  Serial.println("Bluetooth Started. Ready to Pair.");


  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);


  for (int i = 0; i < NUM_SERVOS; i++) {
    updateServoHardware(i, armServos[i].currentAngle);
  }
  delay(500);
}

void loop() {


  if (SerialBT.available()) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      inputs[i] = SerialBT.parseInt();
    }

    while (SerialBT.available() && SerialBT.read() != '\n');
  }


  for (int i = 0; i < NUM_SERVOS; i++) {
    processIncrementalMove(i, inputs[i]);
  }

  delay(LOOP_DELAY);
}



void processIncrementalMove(int index, int inputVal) {

  if (abs(inputVal) < DEADZONE) return;


  float stepSize = map(abs(inputVal), DEADZONE, 127, 0, MAX_SPEED_STEP * 100) / 100.0;


  if (inputVal < 0) {
    armServos[index].currentAngle -= stepSize;
  } else {
    armServos[index].currentAngle += stepSize;
  }

  // Update the Servo
  updateServoHardware(index, armServos[index].currentAngle);
}

void updateServoHardware(int index, float targetAngle) {
  ServoConfig &s = armServos[index];


  if (targetAngle < s.minLimit) targetAngle = s.minLimit;
  if (targetAngle > s.maxLimit) targetAngle = s.maxLimit;


  s.currentAngle = targetAngle;


  int pulse = map((int)targetAngle, 0, 180, SERVOMIN, SERVOMAX);


  pwm.setPWM(s.channel, 0, pulse);
}





