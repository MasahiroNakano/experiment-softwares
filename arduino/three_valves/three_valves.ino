const int valve_1_Pin = 8;
const int valve_2_Pin = 9;
const int valve_3_Pin = 10;

void setup() {
  pinMode(valve_1_Pin, OUTPUT);
  pinMode(valve_2_Pin, OUTPUT);
  pinMode(valve_3_Pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(valve_1_Pin, HIGH);  // trigger
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(50);                     // 50 ms pulse
  digitalWrite(valve_1_Pin, LOW);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(3000);                   // long off time

  digitalWrite(valve_2_Pin, HIGH);  // trigger
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(50);                     // 50 ms pulse
  digitalWrite(valve_2_Pin, LOW);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(3000);                   // long off time

  digitalWrite(valve_3_Pin, HIGH);  // trigger
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(50);                     // 50 ms pulse
  digitalWrite(valve_3_Pin, LOW);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(3000);                   // long off time
}
