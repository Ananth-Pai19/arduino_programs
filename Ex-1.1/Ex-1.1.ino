int BLUELED = 13;

void setup() {
  pinMode(BLUELED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(BLUELED, HIGH);
  delay(1000);
  digitalWrite(BLUELED, LOW);
  delay(1000);

}
