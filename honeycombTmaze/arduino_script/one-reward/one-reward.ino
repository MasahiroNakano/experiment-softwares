
const byte SOLENOID_PIN = 8;

const unsigned long PULSE_MS = 50;

bool pulseActive = false;
unsigned long pulseStartTime = 0;

void setup() {
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);

  Serial.begin(115200);
}

void loop() {
  // Receive trigger from Python
  while (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'p') {
      startPulse();
    }

    if (command == '0') {
      stopPulse();
    }
  }

  // End pulse after 50 ms
  if (pulseActive && millis() - pulseStartTime >= PULSE_MS) {
    stopPulse();
  }
}

void startPulse() {
  digitalWrite(SOLENOID_PIN, HIGH);
  pulseActive = true;
  pulseStartTime = millis();
}

void stopPulse() {
  digitalWrite(SOLENOID_PIN, LOW);
  pulseActive = false;
}