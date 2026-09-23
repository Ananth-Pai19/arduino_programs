#include <LiquidCrystal.h>
#include <string>
LiquidCrystal lcd(2,3,4,8,12,13);

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop(){
  String str;
  if(Serial.available() > 0){
    str = Serial.readString();
    str.trim();

    int firstNum = 0, secNum = 0;

    int i = 100;
    for(int j = 1; j < 4; j++){
      firstNum += (str[j] - '0') * i;
      i/=10;
    }
    i = 100;
    for(int j = 5; j < 8; j++){
      secNum += (str[j] - '0') * i;
      i/=10;
    }

    lcd.setCursor(0, 0);
    lcd.print(firstNum);

    lcd.setCursor(0, 1);
    lcd.print(secNum);

  } 
}