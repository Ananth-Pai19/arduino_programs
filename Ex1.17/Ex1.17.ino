#include <LiquidCrystal.h>

LiquidCrystal lcd(2,3,4,8,12,13);

int value = 0;
void setup() {
  lcd.begin(16, 2);
  pinMode(7, INPUT_PULLUP);
  lcd.setCursor(0,0);
  lcd.print(value);
}

void loop() {
  lcd.setCursor(0,0);
  lcd.print(value);
  if(digitalRead(7) == LOW) {
    value++;
  }
  delay(100);
}
