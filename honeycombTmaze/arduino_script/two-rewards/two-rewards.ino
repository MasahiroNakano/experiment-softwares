const byte VALVE_8_PIN = 8;
const byte VALVE_9_PIN = 9;

const unsigned long PULSE_MS = 50;

bool valve8Active = false;
bool valve9Active = false;

unsigned long valve8StartTime = 0;
unsigned long valve9StartTime = 0;

void setup() {
  pinMode(VALVE_8_PIN, OUTPUT);
  pinMode(VALVE_9_PIN, OUTPUT);

  digitalWrite(VALVE_8_PIN, LOW);
  digitalWrite(VALVE_9_PIN, LOW);

  Serial.begin(115200);
}

void loop() {
  while (Serial.available() > 0) {
    char command = Serial.read();

    if (command == '8') {
      pulseValve8();
    }

    if (command == '9') {
      pulseValve9();
    }

    if (command == '0') {
      stopAllValves();
    }
  }

  unsigned long now = millis();

  if (valve8Active && now - valve8StartTime >= PULSE_MS) {
    digitalWrite(VALVE_8_PIN, LOW);
    valve8Active = false;
  }

  if (valve9Active && now - valve9StartTime >= PULSE_MS) {
    digitalWrite(VALVE_9_PIN, LOW);
    valve9Active = false;
  }
}

void pulseValve8() {
  digitalWrite(VALVE_8_PIN, HIGH);
  valve8Active = true;
  valve8StartTime = millis();
}

void pulseValve9() {
  digitalWrite(VALVE_9_PIN, HIGH);
  valve9Active = true;
  valve9StartTime = millis();
}

void stopAllValves() {
  digitalWrite(VALVE_8_PIN, LOW);
  digitalWrite(VALVE_9_PIN, LOW);

  valve8Active = false;
  valve9Active = false;
}