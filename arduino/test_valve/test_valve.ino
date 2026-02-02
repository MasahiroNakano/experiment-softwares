const int valvePin = 8;

void setup() {
  pinMode(valvePin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(valvePin, HIGH);  // trigger
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(50);                     // 50 ms pulse
  digitalWrite(valvePin, LOW);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(3000);                   // long off time
}
