

#define TRIG_PIN 3
#define ECHO_PIN 4
#define STOP_DISTANCE 25  


#define ENA 10    // Left motor speed (PWM)
#define IN1 9     // Left motor direction
#define IN2 8     // Left motor direction
#define IN3 7     // Right motor direction
#define IN4 6     // Right motor direction
#define ENB 5     // Right motor speed (PWM)


#define SENSOR_0 A0
#define SENSOR_1 A1
#define SENSOR_2 A2
#define SENSOR_3 A3
#define SENSOR_4 A4
#define SENSOR_5 A5
#define SENSOR_6 A6
#define SENSOR_7 A7


#define BASE_SPEED 90     
#define MAX_SPEED 255      
#define TURN_SPEED 100     
#define MIN_PWM 55     


#define KP 15.0   
#define KI 0.0    
#define KD 12.0   


#define THRESHOLD 500  


int sensorPins[8] = {SENSOR_0, SENSOR_1, SENSOR_2, SENSOR_3, 
                     SENSOR_4, SENSOR_5, SENSOR_6, SENSOR_7};
int sensorValues[8];           
bool digitalSensors[8];        
int minValues[8] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int maxValues[8] = {0, 0, 0, 0, 0, 0, 0, 0};

float lastError = 0;           
int position = 0;              
bool lineDetected = false;     


unsigned long lastDistanceCheck = 0;
int currentDistance = 0; 

void setup() {
  Serial.begin(115200);
  

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);


  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  stopBot();
  
  Serial.println("=== LINE FOLLOWING BOT ===");
  Serial.println("Place bot on track...");
  delay(1000);
  
  Serial.println("Calibrating sensors...");
  calibrateSensors();
  
  Serial.println("Calibration complete!");
  Serial.println("Starting in 2 seconds...");
  delay(2000);
}

void loop() {

  for(int i=0;i<8;i++){
    Serial.print("value of ");Serial.print(i);Serial.print(" is "); Serial.println(digitalSensors[i]);
  }
  Serial.println(lineDetected);


  if (millis() - lastDistanceCheck > 100) {
    currentDistance = getDistance();
    lastDistanceCheck = millis();
    
  }



  if (currentDistance > 0 && currentDistance < STOP_DISTANCE) {
    stopBot();
    return; 
  }


  readSensors();
  position = calculatePosition();
  float correction = calculatePID(position);
  
  if (lineDetected) {
    int leftSpeed = constrain(BASE_SPEED + correction, -MAX_SPEED, MAX_SPEED);
    int rightSpeed = constrain(BASE_SPEED - correction, -MAX_SPEED, MAX_SPEED);
    moveBot(leftSpeed, rightSpeed);
  } else {
    stopBot();
  }
}


int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // FIX: Increased timeout to 15000us (15ms). 
  // 3000us was too short for your sensor, causing false 0 readings.
  long duration = pulseIn(ECHO_PIN, HIGH, 15000); 
  
  if (duration == 0) return 999; // 0 usually means "out of range" (far)
  return duration * 0.034 / 2; 
}

// --- SENSOR FUNCTIONS ---
void readSensors() {
  lineDetected = false;
  for (int i = 0; i < 8; i++) {
    int raw = analogRead(sensorPins[i]);
    sensorValues[i] = raw;
    
    int normalized = map(raw, minValues[i], maxValues[i], 0, 1000);
    normalized = constrain(normalized, 0, 1000);
    
    digitalSensors[i] = (raw < THRESHOLD) ? 0 : 1;
    if (!digitalSensors[i]) lineDetected=1;
  }
}

int calculatePosition() {
  long weightedSum = 0;
  long sum = 0;
  int weights[8] = {-70, -35, -20, -10, 10, 20, 35, 70};
  
  for (int i = 0; i < 8; i++) {
    int value = 1023 - sensorValues[i]; 
    weightedSum += (long)value * weights[i];
    sum += value;
  }
  
  if (sum == 0) {
    return (lastError > 0) ? 40 : -40; 
  }
  return weightedSum / sum;
}

// --- PID CONTROL ---
float calculatePID(int error) {
  float P = error;
  float derivative = error - lastError;
  lastError = error;
  return (KP * P) + (KD * derivative);
}

// --- MOTOR CONTROL ---
#define LEFT_MOTOR_BIAS  1.0 
#define RIGHT_MOTOR_BIAS 1.0

void moveBot(int leftSpeed, int rightSpeed) {
  leftSpeed = leftSpeed * LEFT_MOTOR_BIAS;
  rightSpeed = rightSpeed * RIGHT_MOTOR_BIAS;

  // Anti-Stall Logic
  if (leftSpeed != 0 && abs(leftSpeed) < MIN_PWM) {
    leftSpeed = (leftSpeed > 0) ? MIN_PWM : -MIN_PWM;
  }
  if (rightSpeed != 0 && abs(rightSpeed) < MIN_PWM) {
    rightSpeed = (rightSpeed > 0) ? MIN_PWM : -MIN_PWM;
  }

  if (leftSpeed >= 0) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    analogWrite(ENA, abs(leftSpeed));
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    analogWrite(ENA, abs(leftSpeed));
  }
  
  if (rightSpeed >= 0) {
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENB, abs(rightSpeed));
  } else {
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENB, abs(rightSpeed));
  }
}

void stopBot() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}

void searchLine() {
  if (lastError > 0) turnRight();
  else turnLeft();
}

void turnLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
}

void calibrateSensors() {
  unsigned long startTime = millis();
  while (millis() - startTime < 4000) {
    for (int i = 0; i < 8; i++) {
      int value = analogRead(sensorPins[i]);
      if (value < minValues[i]) minValues[i] = value;
      if (value > maxValues[i]) maxValues[i] = value;
    }
    if ((millis() - startTime) % 400 < 200) moveBot(80, -80);
    else moveBot(-80, 80);
    delay(5);
  }
  stopBot();
}