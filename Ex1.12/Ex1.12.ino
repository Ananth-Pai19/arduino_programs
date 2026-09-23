#include <LiquidCrystal.h>

LiquidCrystal lcd(2,3,4,8,12,13);

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  pinMode(11, OUTPUT);
}

void loop() {
  int value = analogRead(A3);
  if(value <= 850) {
    digitalWrite(11, HIGH);

    Serial.println(value);
    lcd.setCursor(0,0);
    lcd.print(value);

    Serial.println("ON");
    lcd.setCursor(0,1);
    lcd.print("ON ");
  }else{
    digitalWrite(11, LOW);

    Serial.println(value);
    lcd.setCursor(0,0);
    lcd.print(value);

    Serial.println("OFF");
    lcd.setCursor(0,1);
    lcd.print("OFF");
  }
}
