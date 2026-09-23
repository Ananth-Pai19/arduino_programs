#include <LiquidCrystal.h>

LiquidCrystal lcd(2,3,4,8, 12, 13);

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);

}

void loop() {
  int value = analogRead(A2);
  lcd.setCursor(0,0);
  lcd.print(value);
}
