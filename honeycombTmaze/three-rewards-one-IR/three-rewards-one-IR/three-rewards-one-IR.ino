// three-rewards.ino
//
// IR-gated three-valve reward controller.
//
// Behavior:
//   - Python sends 'a' when the space bar is pressed.
//   - Arduino becomes ARMED for the current next target.
//   - The next new IR beam break triggers exactly one 50 ms valve pulse.
//   - After that pulse starts, Arduino suppresses further rewards until re-armed.
//   - Reward sequence: DIG8 -> DIG9 -> DIG10 -> DIG9 -> DIG8 -> ...
//   - Python can overwrite the next target with L/C/R keys.
//
// Serial commands from Python:
//   'a' or 'A' : arm next IR-triggered reward
//   '0'        : disarm and force all valves off
//   'l' or 'L' : set next target to DIG8
//   'c' or 'C' : set next target to DIG9
//   'r' or 'R' : set next target to DIG10
//
// Backward-compatible direct pulse commands:
//   '8' : pulse DIG8 immediately
//   '9' : pulse DIG9 immediately
//   't' : pulse DIG10 immediately

const byte SENSOR_PIN = 2;

const byte VALVE_8_PIN = 8;
const byte VALVE_9_PIN = 9;
const byte VALVE_10_PIN = 10;

const byte VALVE_PINS[] = {VALVE_8_PIN, VALVE_9_PIN, VALVE_10_PIN};
const byte NUM_VALVES = sizeof(VALVE_PINS) / sizeof(VALVE_PINS[0]);

// Change this to LOW if your IR sensor reports LOW when the beam is blocked.
const byte BLOCKED_STATE = HIGH;

const unsigned long PULSE_MS = 50;

// Repeating target sequence:
// DIG8 -> DIG9 -> DIG10 -> DIG9 -> DIG8 -> ...
const byte REWARD_SEQUENCE[] = {VALVE_8_PIN, VALVE_9_PIN, VALVE_10_PIN, VALVE_9_PIN};
const byte REWARD_SEQUENCE_LEN = sizeof(REWARD_SEQUENCE) / sizeof(REWARD_SEQUENCE[0]);

byte sequenceIndex = 0;
bool armed = false;
bool previousBlocked = false;

bool valveActive[NUM_VALVES] = {false, false, false};
unsigned long valveStartTime[NUM_VALVES] = {0, 0, 0};

bool isBeamBlocked() {
  return digitalRead(SENSOR_PIN) == BLOCKED_STATE;
}

void logEvent(const char* eventName, int pinNumber) {
  Serial.print("EVENT,");
  Serial.print(millis());
  Serial.print(",");
  Serial.print(eventName);
  Serial.print(",");
  Serial.println(pinNumber);
}

int valveIndexForPin(byte pinNumber) {
  for (byte i = 0; i < NUM_VALVES; i++) {
    if (VALVE_PINS[i] == pinNumber) {
      return i;
    }
  }
  return -1;
}

byte currentTargetPin() {
  return REWARD_SEQUENCE[sequenceIndex];
}

void advanceSequence() {
  sequenceIndex = (sequenceIndex + 1) % REWARD_SEQUENCE_LEN;
  logEvent("NEXT_TARGET", currentTargetPin());
}

void setNextTarget(byte pinNumber) {
  if (pinNumber == VALVE_8_PIN) {
    sequenceIndex = 0;
  } else if (pinNumber == VALVE_9_PIN) {
    // Chooses the first DIG9 in the sequence, so after this reward the next is DIG10.
    sequenceIndex = 1;
  } else if (pinNumber == VALVE_10_PIN) {
    sequenceIndex = 2;
  } else {
    return;
  }

  logEvent("NEXT_TARGET_SET", currentTargetPin());
}

void pulseValve(byte pinNumber, unsigned long now) {
  int index = valveIndexForPin(pinNumber);
  if (index < 0) {
    return;
  }

  digitalWrite(pinNumber, HIGH);
  valveActive[index] = true;
  valveStartTime[index] = now;

  logEvent("VALVE_ON", pinNumber);
}

void stopAllValves() {
  for (byte i = 0; i < NUM_VALVES; i++) {
    if (valveActive[i]) {
      logEvent("VALVE_OFF", VALVE_PINS[i]);
    }

    digitalWrite(VALVE_PINS[i], LOW);
    valveActive[i] = false;
  }
}

void handleSerialCommands(bool blockedNow, unsigned long now) {
  while (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'a' || command == 'A') {
      armed = true;

      // Require a NEW beam break after arming.
      // If the beam is already blocked when space is pressed, no reward is delivered
      // until the beam clears and is blocked again.
      previousBlocked = blockedNow;

      logEvent("ARMED", currentTargetPin());
    } else if (command == '0') {
      armed = false;
      stopAllValves();
      logEvent("DISARMED", -1);
    } else if (command == 'l' || command == 'L') {
      setNextTarget(VALVE_8_PIN);
    } else if (command == 'c' || command == 'C') {
      setNextTarget(VALVE_9_PIN);
    } else if (command == 'r' || command == 'R') {
      setNextTarget(VALVE_10_PIN);
    } else if (command == '8') {
      pulseValve(VALVE_8_PIN, now);
    } else if (command == '9') {
      pulseValve(VALVE_9_PIN, now);
    } else if (command == 't' || command == 'T') {
      pulseValve(VALVE_10_PIN, now);
    }
  }
}

void updateValves(unsigned long now) {
  for (byte i = 0; i < NUM_VALVES; i++) {
    if (valveActive[i] && now - valveStartTime[i] >= PULSE_MS) {
      digitalWrite(VALVE_PINS[i], LOW);
      valveActive[i] = false;
      logEvent("VALVE_OFF", VALVE_PINS[i]);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);

  for (byte i = 0; i < NUM_VALVES; i++) {
    pinMode(VALVE_PINS[i], OUTPUT);
    digitalWrite(VALVE_PINS[i], LOW);
  }

  previousBlocked = isBeamBlocked();

  logEvent("READY", -1);
  logEvent("NEXT_TARGET", currentTargetPin());
}

void loop() {
  unsigned long now = millis();
  bool blocked = isBeamBlocked();

  handleSerialCommands(blocked, now);

  bool newBeamBreak = blocked && !previousBlocked;

  // Log every new IR activation, even if rewards are currently suppressed.
  if (newBeamBreak) {
    logEvent("IR_TRIGGER", SENSOR_PIN);
  }

  // If armed, exactly one new beam break gives one reward.
  if (armed && newBeamBreak) {
    byte targetPin = currentTargetPin();

    pulseValve(targetPin, now);

    // Suppress further rewards until space is pressed again.
    armed = false;
    logEvent("DISARMED_AFTER_REWARD", -1);

    // Advance only after an actual rewarded valve activation.
    advanceSequence();
  }

  updateValves(now);

  previousBlocked = blocked;
}
