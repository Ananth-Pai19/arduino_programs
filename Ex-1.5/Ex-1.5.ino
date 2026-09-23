void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int brightness = 0; brightness <= 255; brightness++){
  analogWrite(3, brightness);
  analogWrite(5, brightness);
  analogWrite(6, brightness);
  delay(10);
  }
  for(int brightness = 255; brightness >= 0; brightness--){
  analogWrite(3, brightness);
  analogWrite(5, brightness);
  analogWrite(6, brightness);
  delay(10);
  }
}
