#include <LiquidCrystal.h>

LiquidCrystal lcd(2,3,4,8,12,13);

int ldrPin = A3;
int threshold = 850;   
int count = 0;
bool fingerPresent = false;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready! Cross now");
  delay(1000);

  count = 0;
  fingerPresent = false;
  
  unsigned long startTime = millis();  

  // Count for 5 seconds
  while (millis() - startTime < 5000) {
    int ldrValue = analogRead(ldrPin);
    
    // Finger is blocking light (value drops below threshold)
    if (ldrValue > threshold && !fingerPresent) {
      fingerPresent = true;   // Finger just entered
      count++;
      
      Serial.print("Cross detected! Count: ");
      Serial.println(count);
    }
    
    // Finger removed
    if (ldrValue <= threshold && fingerPresent) {
      fingerPresent = false;  // Finger left, ready for next cross
    }
    
    // Show live countdown and count on LCD
    int timeLeft = 5 - (millis() - startTime) / 1000;
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(timeLeft);
    lcd.print("s  ");
    
    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(count);
    
    delay(50);
  }

  // Display final result
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Done!");
  lcd.setCursor(0, 1);
  lcd.print("Crosses: ");
  lcd.print(count);
  
  Serial.print("Final count: ");
  Serial.println(count);
  
  delay(3000);  // Show result for 3 seconds, then restart
}