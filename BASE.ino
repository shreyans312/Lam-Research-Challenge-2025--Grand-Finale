#include <PS4Controller.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


//(FL)
#define ENA 33
#define IN1 23
#define IN2 22

//(FR)
#define ENB 32
#define IN3 21
#define IN4 19

//(RL)
#define EN_2A 25
#define IN2_1 14
#define IN2_2 15

//(RR)
#define EN_2B 12
#define IN2_3 2
#define IN2_4 0

#define PWM_FREQ 1000
#define PWM_RES 8
#define MIN_PWM 75
#define RAMP_STEP 10

unsigned long lastDebugTime = 0;

int targetFL = 0, targetFR = 0, targetRL = 0, targetRR = 0;
int currentFL = 0, currentFR = 0, currentRL = 0, currentRR = 0;


int squareMap(int val) {
  int sign = (val >= 0) ? 1 : -1;
  int x = abs(val);        // 0–255
  int y = (x * x) / 65;   // square + normalize
  return y * sign;
}


void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  PS4.begin();

  Serial.println("PS4 Controller ready. Waiting for connection...");

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN2_1, OUTPUT); pinMode(IN2_2, OUTPUT);
  pinMode(IN2_3, OUTPUT); pinMode(IN2_4, OUTPUT);

  ledcAttach(ENA, PWM_FREQ, PWM_RES);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);
  ledcAttach(EN_2A, PWM_FREQ, PWM_RES);
  ledcAttach(EN_2B, PWM_FREQ, PWM_RES);
}


void driveMotor(int in1Pin, int in2Pin, int enaPin, int val) {
  int absVal = abs(val);
  int outputSpeed = 0;

  if (absVal < 45) outputSpeed = 0;
  else outputSpeed = map(absVal, 0, 255, MIN_PWM, 255);

  if (val > 0) {
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
  } 
  else if (val < 0) {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
  } 
  else {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    outputSpeed = 0;
  }

  ledcWrite(enaPin, outputSpeed);
}


int rampValue(int current, int target) {
  if (current < target) current += RAMP_STEP;
  else if (current > target) current -= RAMP_STEP;
  return current;
}


void loop() {

  if (PS4.isConnected()) {

    int ly = -PS4.data.analog.stick.ry; // Forward/back
    int lx =  PS4.data.analog.stick.rx; // Strafe
    int rx =  -(PS4.data.analog.stick.lx); // Rotation

    // Deadzone
    if (abs(ly) < 50) ly = 0;
    if (abs(lx) < 50) lx = 0;
    if (abs(rx) < 50) rx = 0;


    int fl_raw = ly + lx + rx;
    int fr_raw = ly - lx - rx;
    int rl_raw = ly - lx + rx;
    int rr_raw = ly + lx - rx;

    // Clamp
    fl_raw = constrain(fl_raw, -255, 255);
    fr_raw = constrain(fr_raw, -255, 255);
    rl_raw = constrain(rl_raw, -255, 255);
    rr_raw = constrain(rr_raw, -255, 255);


    targetFL = squareMap(fl_raw);
    targetFR = squareMap(fr_raw);
    targetRL = squareMap(rl_raw);
    targetRR = squareMap(rr_raw);

  } else {
    targetFL = targetFR = targetRL = targetRR = 0;
  }

  // Smooth ramping
  currentFL = rampValue(currentFL, targetFL);
  currentFR = rampValue(currentFR, targetFR);
  currentRL = rampValue(currentRL, targetRL);
  currentRR = rampValue(currentRR, targetRR);

  // Drive motors
  driveMotor(IN1, IN2, ENA, -currentFL);
  driveMotor(IN3, IN4, ENB, -currentFR);
  driveMotor(IN2_1, IN2_2, EN_2A, -currentRL);
  driveMotor(IN2_3, IN2_4, EN_2B, -currentRR);

  // // Debug print every 200 ms
  // if (millis() - lastDebugTime > 200) {
  //   Serial.printf("FL:%d FR:%d RL:%d RR:%d\n", currentFL, currentFR, currentRL, currentRR);
  //   lastDebugTime = millis();
  // }

  delay(10);
}