#include <Servo.h>

// --------------------
// Pins
// --------------------
const int RED_PIN   = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN  = 11;

const int LDR_PIN   = A2;
const int SERVO_PIN = 5;

// --------------------
// Settings
// --------------------
const int STEP_ANGLE = 2;       // 2 degree resolution
const int NUM_SAMPLES = 50;
const int SERVO_SETTLE_TIME = 500;  // ms
const int LED_SETTLE_TIME   = 150;  // ms

Servo servo1;


// =====================================================
// Turn RGB LED OFF
// =====================================================
void rgbOff() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
}


// =====================================================
// Average multiple LDR readings
// =====================================================
int readLDR() {
  long sum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(LDR_PIN);
    delay(5);
  }

  return sum / NUM_SAMPLES;
}


// =====================================================
// Measure LDR response for one LED colour
// =====================================================
int measureLED(int ledPin) {

  rgbOff();

  digitalWrite(ledPin, HIGH);

  delay(LED_SETTLE_TIME);

  int value = readLDR();

  rgbOff();

  delay(50);

  return value;
}


// =====================================================
// Setup
// =====================================================
void setup() {

  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  pinMode(LDR_PIN, INPUT);

  servo1.attach(SERVO_PIN);

  rgbOff();

  // Move to start position
  servo1.write(0);
  delay(1000);

  // CSV header
  Serial.println("Angle,Red_LDR,Green_LDR,Blue_LDR");

  // -------------------------------------------------
  // Sweep from 0 to 180 degrees
  // -------------------------------------------------
  for (int angle = 0; angle <= 180; angle += STEP_ANGLE) {

    servo1.write(angle);

    // Allow servo to physically reach position
    delay(SERVO_SETTLE_TIME);

    int redValue   = measureLED(RED_PIN);
    int greenValue = measureLED(GREEN_PIN);
    int blueValue  = measureLED(BLUE_PIN);

    // CSV output
    Serial.print(angle);
    Serial.print(",");

    Serial.print(redValue);
    Serial.print(",");

    Serial.print(greenValue);
    Serial.print(",");

    Serial.println(blueValue);
  }

  rgbOff();

  Serial.println("SWEEP COMPLETE");
}


// =====================================================
// Nothing repeated automatically
// =====================================================
void loop() {
}