#include <Servo.h>

static const byte SERVO_COUNT = 4;
static const byte SERVO_PINS[SERVO_COUNT] = {3, 5, 6, 9};
static const byte DEFAULT_ANGLE = 90;

Servo servos[SERVO_COUNT];
byte currentAngles[SERVO_COUNT] = {
  DEFAULT_ANGLE,
  DEFAULT_ANGLE,
  DEFAULT_ANGLE,
  DEFAULT_ANGLE
};

void attachServos() {
  for (byte i = 0; i < SERVO_COUNT; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(DEFAULT_ANGLE);
  }
}

bool parseCommand(int &servoIndex, int &angle) {
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() < 4 || line.charAt(0) != 'S') {
    return false;
  }

  int separator = line.indexOf(':');
  if (separator <= 1 || separator == line.length() - 1) {
    return false;
  }

  servoIndex = line.substring(1, separator).toInt() - 1;
  angle = line.substring(separator + 1).toInt();
  if (servoIndex < 0 || servoIndex >= SERVO_COUNT) {
    return false;
  }

  if (angle < 0 || angle > 180) {
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(20);
  attachServos();
  Serial.println(F("4-servo arm controller ready"));
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  int servoIndex = -1;
  int angle = -1;
  if (!parseCommand(servoIndex, angle)) {
    Serial.println(F("ERR"));
    return;
  }

  currentAngles[servoIndex] = angle;
  servos[servoIndex].write(angle);
  Serial.print(F("OK S"));
  Serial.print(servoIndex + 1);
  Serial.print(F(":"));
  Serial.println(angle);
}
