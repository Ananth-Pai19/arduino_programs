#include <Servo.h>

Servo myServo;
void setup() {
  Serial.begin(9600);
  myServo.attach(6);
  pinMode(7, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(11, OUTPUT);
  myServo.write(20);
}

void loop() {
  int angle = myServo.read();
  Serial.println(angle);
  if(digitalRead(9) == LOW){
    angle = myServo.read();
    myServo.write(angle+20);
  }else{
    digitalWrite(11, LOW);
  }
  if(digitalRead(8) == LOW){
    angle = myServo.read();
    myServo.write(angle-20);
  }
  delay(1000);
}