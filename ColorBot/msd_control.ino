/*
This program should perform the following mission:
- At startup it should request the color to pick at
- Next it should request the color to go to to drop at
- Both are now chosen from the phone (WiFi page), see wifi_control.h
- After both are recieved, rotate the black servo from 0 to 180 (sweep)
- While sweeping, the RGB led should keep going to get the correct reading from LDR
- On detecting the color, the black servo should stop, the blue servo should lower until proximity sensor detects
- On detection, the blueservo is lowered further, the EM is energised and the blue servo arm is raised
- Sweeps again for drop and denergises the EM at drop
- Returns to home after dropping and resets all flags and asks for the next mission
*/

// Include the Servo library
#include <Servo.h>
#include "wifi_control.h"   // WIFI: phone control (hotspot + web page)

// The physical constants of the board
#define BLACK_SERVO 5
#define BLUE_SERVO 6
#define EM 7
#define IR A0
#define LDR A2
#define R 9
#define G 10
#define B 11

// Define the servos
Servo blackServo;
Servo blueServo;

// Sweep settings
const int STEP_ANGLE = 2;
const int NUM_SAMPLES = 50;
const int SERVO_SETTLE_TIME = 500;
const int LED_SETTLE_TIME = 150;

// Servo angles
const int ARM_UP_ANGLE = 180;
const int HOME_BASE = 0;

// The threshold for it to be considered that colour
const float MAX_COLOR_DISTANCE = 100.0;
const int NUM_COLOURS = 4;

// Define the state variables
// Calibration constants recorded over WHITE floor (Reflected Intensity = 1023 - raw)
const float WHITE_R = 1023.0 - 738.0; // 205
const float WHITE_G = 1023.0 - 653.0; // 264
const float WHITE_B = 1023.0 - 685.0; // 242

// Define the states enum
enum MissionState {
  WAIT_INPUT,
  SWEEP_FOR_PICK,
  PICK,
  SWEEP_FOR_DROP,
  DROP,
  HOME
};

MissionState currentState = WAIT_INPUT;    // The current mission state
int currentBlackAngle = 0;            // Tracks the current black servo angle   
int currentBlueAngle = ARM_UP_ANGLE;  // Tracks the current blue servo angle
const int ANGLE_OFFSET = 35;          // Angle offset to make the EM come over the color
const int BLUE_OFFSET = -6;          // Offset angle to lower the arm further before picking

unsigned long lastTimeStep = 0;
const unsigned long TIME_STEP = 300;  // Time step in milliseconds
String pickColor = "";
String dropColor = "";
int sweepBlackDirection = 1;
int sweepBlueDirection = 1;
bool capturedOffsetStartAngle = false;
bool newCapturedOffsetStartAngle = false;
bool blueOffset = false;

int offsetStartAngle = 0;
bool reachedPickColor = false;
bool reachedDropColor = false;
const int proximityDetection = 30;
int proximityReading = 0;

// WIFI: text shown on the phone when the arm is idle, and whether STOP was pressed
String lastResult = "Ready - choose colours";
bool stoppedByUser = false;


/*
Helper Functions
*/

// Function to pulse RGB LEDs, read LDR values, and detect the color
String readColor() {
  // 1. Read RED reflection
  digitalWrite(R, HIGH);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
  delay(LED_SETTLE_TIME);
  int rawR = analogRead(LDR);

  // 2. Read GREEN reflection
  digitalWrite(R, LOW);
  digitalWrite(G, HIGH);
  digitalWrite(B, LOW);
  delay(LED_SETTLE_TIME);
  int rawG = analogRead(LDR);

  // 3. Read BLUE reflection
  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, HIGH);
  delay(LED_SETTLE_TIME);
  int rawB = analogRead(LDR);

  // 4. Turn off all LEDs
  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);

  // 5. Pass raw values to the color detection function
  return detectFloorColor(rawR, rawG, rawB);
}

String detectFloorColor(int rawR, int rawG, int rawB) {
  // Convert inverted raw readings to intensity
  float intensityR = 1023.0 - rawR;
  float intensityG = 1023.0 - rawG;
  float intensityB = 1023.0 - rawB;

  // Normalize relative to White surface (0.0 to 1.0 scale)
  float normR = intensityR / WHITE_R;
  float normG = intensityG / WHITE_G;
  float normB = intensityB / WHITE_B;

  // 1. Check for White background (All channels high & balanced)
  if (normR > 0.85 && normG > 0.85 && normB > 0.85) {
    return "WHITE";
  }

  // 2. Identify highest normalized channel
  if (normR > normG && normR > normB) {
    return "RED";
  } else if (normG > normR && normG > normB) {
    return "GREEN";
  } else if (normB > normR && normB > normG) {
    return "BLUE";
  }

  return "UNKNOWN";
}

String getSerialInput(String prompt) {
  Serial.println(prompt);

  // Wait until user types something in Serial Monitor
  while (Serial.available() == 0) {
    // Do nothing, just wait
  }

  // Read the full string until Enter ('\n') is pressed
  String input = Serial.readStringUntil('\n');
  
  // Clean up whitespace and convert to uppercase (e.g., "red " -> "RED")
  input.trim();
  input.toUpperCase();

  return input;
}

bool isValidColor(String col) {
  return (col == "RED" || col == "GREEN" || col == "BLUE");
}

String getValidSerialInput(String prompt) {
  String input = "";
  while (true) {
    input = getSerialInput(prompt); // Call original read function
    
    if (isValidColor(input)) {
      return input; // Valid color entered, break out of loop
    }

    // Invalid input fallback
    Serial.print("ERROR: '");
    Serial.print(input);
    Serial.println("' is not valid. Please enter RED, GREEN, BLUE, or WHITE.\n");
  }
}


// WIFI: tell the phone what the arm is doing (only sends when something changes)
void reportStatus() {
  static int lastKey = -1;
  bool em = digitalRead(EM);
  int key = currentState * 10 + reachedPickColor * 4 + reachedDropColor * 2 + em;
  if (key == lastKey) return;
  lastKey = key;

  switch (currentState) {
    case WAIT_INPUT:     wifiSetStatus("idle", lastResult, em); break;
    case SWEEP_FOR_PICK: wifiSetStatus("sweep_pick", reachedPickColor ? "Found " + pickColor + ", lining up"
                                                                      : "Sweeping for " + pickColor, em); break;
    case PICK:           wifiSetStatus("pick", "Lowering arm to pick up", em); break;
    case SWEEP_FOR_DROP: wifiSetStatus("sweep_drop", reachedDropColor ? "Found " + dropColor + ", lining up"
                                                                      : "Carrying to " + dropColor, em); break;
    case DROP:           wifiSetStatus("drop", "Dropping at " + dropColor, em); break;
    case HOME:           wifiSetStatus("home", "Returning home", em); break;
  }
}


/* 
The main control code
*/
void setup() {
  // put your setup code here, to run once:
  pinMode(BLACK_SERVO, OUTPUT);
  pinMode(BLUE_SERVO, OUTPUT);
  pinMode(EM, OUTPUT);
  pinMode(IR, INPUT);
  pinMode(LDR, INPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  // Start the magnet being off
  digitalWrite(EM, LOW);

  // Attach the servos
  blackServo.attach(BLACK_SERVO);
  blueServo.attach(BLUE_SERVO);

  // Send the servos to home position
  blueServo.write(ARM_UP_ANGLE);
  blackServo.write(HOME_BASE);

  Serial.begin(9600);
  Serial.println("Arm initialized");
  delay(500);

  wifiSetup();   // WIFI: start hotspot "ColourBot" -> phone opens http://192.168.4.1

}

void loop() {
  wifiLoop();      // WIFI: answer the phone
  reportStatus();  // WIFI: update the phone's status display

  // WIFI: STOP button -> abandon the mission and go home
  if (wifiStopRequested() && currentState != WAIT_INPUT) {
    Serial.println("STOP pressed on phone");
    stoppedByUser = true;
    currentState = HOME;
  }

  switch(currentState) {
    case WAIT_INPUT: {
      // WIFI: colours now come from the phone instead of the Serial Monitor.
      // (The old getValidSerialInput() waited forever, which would freeze the web page.)
      if (!wifiGetMission(pickColor, dropColor)) break;   // nothing pressed yet -> keep looping

      Serial.print("Pick color set to: ");
      Serial.println(pickColor);
      Serial.print("Drop color set to: ");
      Serial.println(dropColor);

      // Begin sweep
      currentState = SWEEP_FOR_PICK;
      Serial.println("Starting sweep...");
      delay(2000);
      break;
    } 

    case SWEEP_FOR_PICK: {
      if (millis() - lastTimeStep >= TIME_STEP) {
        lastTimeStep = millis();

        // At each time step write what angle we want it to have
        blackServo.write(currentBlackAngle);
        blueServo.write(currentBlueAngle);

        // Detect the color of the floor
        String detected = readColor();

        Serial.print("Black Angle: "); Serial.print(currentBlackAngle);
        Serial.print(" | Blue Angle: "); Serial.print(currentBlueAngle);
        Serial.print(" | Target: "); Serial.print(pickColor);
        Serial.print(" | Detected: "); Serial.println(detected);

        if (detected == pickColor || reachedPickColor) {
          // Before proceeding to the next state
          // Rotate by offset
          reachedPickColor = true;
          if (!capturedOffsetStartAngle) {
            offsetStartAngle = currentBlackAngle;
            capturedOffsetStartAngle = true;
          }

          // Check if offset reached
          if (currentBlackAngle - offsetStartAngle >= ANGLE_OFFSET) {
            Serial.println("Offset done, starting pick...");
            currentState = PICK;

          }
          // Else increment angle
          else {
            currentBlackAngle += 2;
            Serial.println("Reached pick color, starting offset....");
          }

        } else {
          // Increment the angle we want
          currentBlackAngle += (3 * sweepBlackDirection);
          if (currentBlackAngle > 150 || currentBlackAngle <= 0) {
            sweepBlackDirection *= -1; // Reverse sweep direction
          }
        }
      }
      break;
    }

    case PICK: {
      // Lower the arm continuously until the proximity sensor detects
      if (millis() - lastTimeStep >= TIME_STEP) {
        lastTimeStep = millis();

        // At each time step write what angle we want it to have
        blackServo.write(currentBlackAngle);
        blueServo.write(currentBlueAngle);
      
        proximityReading = analogRead(IR);

        if (proximityReading <= proximityDetection) {
          Serial.println("Ground detected, lowering some more....");

          // Lower the arm a little more
          currentBlueAngle += BLUE_OFFSET;
          blueServo.write(currentBlueAngle);
          delay(500);

          // Start electromagnet
          digitalWrite(EM, HIGH);
          delay(2000);

          // Raise the arm
          currentBlueAngle -= 2 * BLUE_OFFSET;
          blueServo.write(currentBlueAngle);
          delay(300);

          // Change the state 
          capturedOffsetStartAngle = false;
          currentState = SWEEP_FOR_DROP;
        }
        // If not detected ground proceed downwards
        else {
          Serial.println("Ground not detected yet...");
          currentBlueAngle -= (2 * sweepBlueDirection);
          if (currentBlueAngle >= 180 || currentBlueAngle <= 120) {
            sweepBlueDirection *= -1; // Reverse sweep direction
          }
        }
      }
      break;
    }

    case SWEEP_FOR_DROP: {
      // Sweep again until it reaches the drop location
      if (millis() - lastTimeStep >= TIME_STEP) {
        lastTimeStep = millis();

        // At each time step write what angle we want it to have
        blackServo.write(currentBlackAngle);
        blueServo.write(currentBlueAngle);

        // Detect the color of the floor
        String detected = readColor();

        Serial.print("Black Angle: "); Serial.print(currentBlackAngle);
        Serial.print(" | Blue Angle: "); Serial.print(currentBlueAngle);
        Serial.print(" | Target: "); Serial.print(dropColor);
        Serial.print(" | Detected: "); Serial.println(detected);

        if (detected == dropColor || reachedDropColor) {
          // Before proceeding to the next state
          // Rotate by offset
          reachedDropColor = true;
          if (!capturedOffsetStartAngle) {
            offsetStartAngle = currentBlackAngle;
            capturedOffsetStartAngle = true;
          }

          // Check if offset reached
          int offset = (sweepBlackDirection == 1) ? ANGLE_OFFSET : ANGLE_OFFSET - 25;
          if (currentBlackAngle - offsetStartAngle >= ANGLE_OFFSET) {
            Serial.println("Offset done, starting drop...");
            currentState = DROP;

          }
          // Else increment angle
          else {
            currentBlackAngle += 2;
            Serial.println("Reached drop color, starting offset....");
          }

        } else {
          // Increment the angle we want
          currentBlackAngle += (3 * sweepBlackDirection);
          if (currentBlackAngle > 150 || currentBlackAngle <= 0) {
            sweepBlackDirection *= -1; // Reverse sweep direction
          }
        }
      }
      break;
    }

    case DROP: {
      if (millis() - lastTimeStep >= TIME_STEP) {
        // 
          // Lower the arm a little more
        // currentBlueAngle += BLUE_OFFSET;
        // blueServo.write(currentBlueAngle);

        // Start electromagnet
        digitalWrite(EM, LOW);
        delay(2000);

        // Raise the arm
        currentBlueAngle -= 2 * BLUE_OFFSET;
        blueServo.write(currentBlueAngle);
        delay(500);

        capturedOffsetStartAngle = false;
        currentState = HOME;
      }
      break;
    }

    case HOME: {
      // Send the servos to home position
      blueServo.write(ARM_UP_ANGLE);
      blackServo.write(HOME_BASE);

      // Reset everything
      currentBlackAngle = HOME_BASE;
      currentBlueAngle = ARM_UP_ANGLE;
      sweepBlackDirection = 1;

      // Reset all mission flags
      reachedPickColor         = false;
      reachedDropColor         = false;
      capturedOffsetStartAngle = false;
      offsetStartAngle         = 0;

      // WIFI: make sure the magnet is off and remember the result for the phone
      digitalWrite(EM, LOW);
      lastResult = stoppedByUser ? "Stopped - arm returned home"
                                 : "Delivered " + pickColor + " -> " + dropColor;
      stoppedByUser = false;

      // Change the state for next mission
      Serial.println("Finished the mission. Resetting....");
      delay(3000);
      currentState = WAIT_INPUT;

      break;
    }
    
  }
}
