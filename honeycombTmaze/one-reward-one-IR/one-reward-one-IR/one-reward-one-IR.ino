const int sensorPin = 2;
const int valvePin = 8;

const int BLOCKED_STATE = HIGH;  // change to LOW if your IR sensor is inverted

const unsigned long PULSE_MS = 50;

bool armed = false;
bool previousBlocked = false;

bool valveActive = false;
unsigned long valveStartTime = 0;

bool isBeamBlocked() {
  return digitalRead(sensorPin) == BLOCKED_STATE;
}

void logEvent(const char* eventName, int pinNumber) {
  Serial.print("EVENT,");
  Serial.print(millis());
  Serial.print(",");
  Serial.print(eventName);
  Serial.print(",");
  Serial.println(pinNumber);
}

void setup() {
  Serial.begin(115200);

  pinMode(sensorPin, INPUT);
  pinMode(valvePin, OUTPUT);

  digitalWrite(valvePin, LOW);

  previousBlocked = isBeamBlocked();

  Serial.println("READY");
}

void loop() {
  unsigned long now = millis();

  bool blocked = isBeamBlocked();

  // Receive commands from Python
  while (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'a') {
      armed = true;
      previousBlocked = blocked;  // require a new beam break after arming
      logEvent("ARMED", -1);
    }

    if (command == '0') {
      armed = false;
      valveActive = false;
      digitalWrite(valvePin, LOW);
      logEvent("DISARMED", -1);
    }
  }

  bool newBeamBreak = blocked && !previousBlocked;

  // Log every new IR activation, even if reward is suppressed
  if (newBeamBreak) {
    logEvent("IR_TRIGGER", sensorPin);
  }

  // If armed, one new beam break gives one valve pulse
  if (armed && newBeamBreak) {
    digitalWrite(valvePin, HIGH);
    valveActive = true;
    valveStartTime = now;

    logEvent("VALVE_ON", valvePin);

    // Suppress until space is pressed again
    armed = false;
  }

  // Turn valve off after 50 ms
  if (valveActive && now - valveStartTime >= PULSE_MS) {
    digitalWrite(valvePin, LOW);
    valveActive = false;

    logEvent("VALVE_OFF", valvePin);
  }

  previousBlocked = blocked;
}