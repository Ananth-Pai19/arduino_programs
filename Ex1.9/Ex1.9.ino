void setup(){
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
}

void loop(){
  int value = analogRead(A2);
  if(value > 255) analogWrite(2, 255-value);
  else analogWrite(3, value);
}