void setup() {
  // put your setup code here, to run once:
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(7,INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  pinMode(9,INPUT_PULLUP);

}

void loop() {
  if (digitalRead(7) == LOW) {
    digitalWrite(11, HIGH);
  }else{
    digitalWrite(11, LOW);
  }
if (digitalRead(8) == LOW) {
    digitalWrite(12, HIGH);
  }else{
    digitalWrite(12, LOW);
  }
  if (digitalRead(9) == LOW) {
    digitalWrite(13, HIGH);
  }else{
    digitalWrite(13, LOW);
  }

}
