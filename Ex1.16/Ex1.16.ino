int ledPins[] = {11, 12, 13};

void setup() {
  Serial.begin(9600);
  
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  Serial.println("Enter a number (0-7):");
}

void loop() {
  if (Serial.available() > 0) {
    int num = Serial.parseInt(); 
    
    if (num >= 0 && num <= 7) {
    
      for (int i = 0; i < 3; i++) {
        int bit = (num >> i) & 1;  
        digitalWrite(ledPins[i], bit);
      }
    } else {
      Serial.println("Invalid! Enter between 0 and 7 only.");
    }
    
    Serial.println("Enter a number (0-7):");
  }
}
