#include "HX711.h"
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 
#include <SPI.h>




#define TFT_CS     14
#define TFT_RST    17
#define TFT_DC     16
// Note: Hardware SPI uses GPIO 23 (MOSI) and 18 (SCK) automatically


const int IR_PIN_GATE1 = 26; 
const int IR_PIN_GATE2 = 27; 
const int IR_PIN_GATE3 = 33; 


const int PUMP_PIN = 25;      
const int LAM_LED_PIN = 32;   


const int LOADCELL_DOUT_PIN = 21;
const int LOADCELL_SCK_PIN = 22;


const float TARGET_FILL_WEIGHT = 118.0; 
const long SAFETY_TIMEOUT_MS = 10000;   


HX711 scale;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


float calibration_factor = 367.0; 
bool gate1Done = false;
bool gate2Done = false;
bool gate3Done = false;


unsigned long pumpStartTime = 0;
bool isPumping = false;


void tftPrint(int x, int y, String text, uint16_t color, int size) {
  tft.setCursor(x, y);
  tft.setTextColor(color, ST7735_BLACK); 
  tft.setTextSize(size);
  tft.print(text);
}

void setup() {
  Serial.begin(115200);

  pinMode(IR_PIN_GATE1, INPUT);
  pinMode(IR_PIN_GATE2, INPUT);
  pinMode(IR_PIN_GATE3, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAM_LED_PIN, OUTPUT);
  
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(LAM_LED_PIN, HIGH);


  tft.initR(INITR_BLACKTAB); 
  tft.setRotation(1); 
  tft.fillScreen(ST7735_BLACK);
  
  tftPrint(10, 10, "Arena System", ST7735_WHITE, 1);
  tftPrint(10, 30, "Ready...", ST7735_GREEN, 1);


  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare(); 

  Serial.println("System Ready.");
  delay(1000);
}

void loop() {

  bool objAtGate1 = !digitalRead(IR_PIN_GATE1);
  bool objAtGate2 = !digitalRead(IR_PIN_GATE2);
  bool objAtGate3 = !digitalRead(IR_PIN_GATE3);


  if (objAtGate1 && !gate1Done) {
    Serial.println("Gate 1: Robot Detected.");
    Serial.println("Starting Pump...");
    

    tft.fillScreen(ST7735_BLACK);
    delay(100);
    tftPrint(10, 30, "Filling...", ST7735_CYAN, 2);
    
    digitalWrite(PUMP_PIN, LOW);   // Pump ON
    pumpStartTime = millis();       
    isPumping = true;               
    gate1Done = true;               
  }


  if (isPumping) {
    float currentFill = scale.get_units(1); 
    
    // Debug
    Serial.print("Filling: "); Serial.print(currentFill); Serial.println("g");

    bool targetReached = (currentFill >= TARGET_FILL_WEIGHT);
    bool safetyTimeout = (millis() - pumpStartTime >= SAFETY_TIMEOUT_MS);

    if (targetReached) {
      digitalWrite(PUMP_PIN, HIGH);  
      isPumping = false;
      Serial.println("Target Weight Reached!");
      
      tft.fillScreen(ST7735_BLACK);
      delay(1000);
      tftPrint(10, 30, "Filled: 125g", ST7735_GREEN, 2);
    } 
    else if (safetyTimeout) {
      digitalWrite(PUMP_PIN, HIGH);  
      isPumping = false;
      Serial.println("ERROR: Safety Timeout");
      
      tft.fillScreen(ST7735_BLACK);
      delay(100);
      tftPrint(10, 30, "Error: Timeout", ST7735_RED, 1);
    }
  }


  if (objAtGate2 && !gate2Done) {
    Serial.println("Gate 2: LAM LED ON");

    
    digitalWrite(LAM_LED_PIN, LOW); 
    gate2Done = true;
  }


  if (objAtGate3 && !gate3Done) {
    Serial.println("Gate 3: Final Weighing Check");
    
    tft.fillScreen(ST7735_BLACK);
    delay(50);
    tftPrint(10, 30, "Verifying...", ST7735_YELLOW, 2);
    
    delay(1000); 

    if (scale.is_ready()) {
      float finalWeight = scale.get_units(10); 
      Serial.print("Final Weight: "); Serial.println(finalWeight);

      if (finalWeight >= 120.0 && finalWeight <= 130.0) {
        displaySuccess(finalWeight);
      } else {
        displayError(finalWeight);
      }
      gate3Done = true; 
    } else {
      Serial.println("Scale Error at Gate 3");
      delay(50);
      tftPrint(10, 50, "Scale Error", ST7735_RED, 1);
    }
  }
}



void displaySuccess(float w) {
  Serial.println("Result: SUCCESS");
  
  tft.fillScreen(ST7735_GREEN); 
  delay(50);
  tftPrint(10, 20, "MAHAGATBANDHAN", ST7735_BLACK, 2);
  tftPrint(10, 60, "Success!", ST7735_BLACK, 2);
  delay(1000);

  tft.setCursor(10, 80);

  tft.setTextColor(ST7735_BLACK, ST7735_RED); 
  tft.print("Got: "); tft.print(w, 1); tft.print("g");

  for(int i=0; i<3; i++) {
    digitalWrite(LAM_LED_PIN, HIGH); delay(200);
    digitalWrite(LAM_LED_PIN, LOW); delay(200);
  }
}

void displayError(float w) {
  Serial.println("Result: WEIGHT MISMATCH");
  
  tft.fillScreen(ST7735_RED); 
  delay(50);
  tftPrint(10, 20, "MISMATCH", ST7735_WHITE, 2);
  tftPrint(10, 50, "Retry", ST7735_WHITE, 2);
  
  delay(1000);

  tft.setCursor(10, 80);

  tft.setTextColor(ST7735_WHITE, ST7735_RED); 
  tft.print("Got: "); tft.print(w, 1); tft.print("g");
}