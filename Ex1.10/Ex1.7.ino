#include <LiquidCrystal.h>
#include <cmath>

LiquidCrystal lcd(2,3,4,8,12,13);

void setup() {
  lcd.begin(16, 2);
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("HelloDYDYSRYHT");
  delay(1000);
}
