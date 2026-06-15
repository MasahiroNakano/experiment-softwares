const int sensorPin = 2;
const int valvePin = 8;

const unsigned long PULSE_MS = 50;

bool armed = false;
bool previousBlocked = false;

bool valveActive = false;
unsigned long valveStartTime = 0;

bool isBeamBlocked() {
  int state = digitalRead(sensorPin);
  return state == HIGH;  // change to LOW if your sensor is inverted
}

void setup() {
  Serial.begin(115200);

  pinMode(sensorPin, INPUT);
  pinMode(valvePin, OUTPUT);

  digitalWrite(valvePin, LOW);

  previousBlocked = isBeamBlocked();

  Serial.println("Arduino ready");
}

void loop() {
  unsigned long now = millis();

  bool blocked = isBeamBlocked();

  // Receive commands from Python
  while (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'a') {
      // Arm reward delivery.
      // This version waits for a NEW beam break after arming.
      armed = true;
      previousBlocked = blocked;

      Serial.println("ARMED");
    }

    if (command == '0') {
      armed = false;
      valveActive = false;
      digitalWrite(valvePin, LOW);

      Serial.println("DISARMED");
    }
  }

  bool newBeamBreak = blocked && !previousBlocked;

  // If armed, one new beam break gives one reward
  if (armed && newBeamBreak) {
    Serial.println("Beam blocked: reward");

    digitalWrite(valvePin, HIGH);
    valveActive = true;
    valveStartTime = now;

    // Suppress further rewards until space is pressed again
    armed = false;
  }

  // Turn valve off after 50 ms
  if (valveActive && now - valveStartTime >= PULSE_MS) {
    digitalWrite(valvePin, LOW);
    valveActive = false;

    Serial.println("Valve off");
  }

  previousBlocked = blocked;
}