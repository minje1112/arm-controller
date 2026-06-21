#include <Servo.h>

static const byte SERVO_COUNT = 4;
static const byte SERVO_PINS[SERVO_COUNT] = {3, 5, 6, 9};
static const byte DEFAULT_ANGLE = 90;

Servo servos[SERVO_COUNT];

enum ParseResult {
  PARSE_OK,
  PARSE_ERR_FORMAT,
  PARSE_ERR_SERVO,
  PARSE_ERR_ANGLE
};

void attachServos() {
  for (byte i = 0; i < SERVO_COUNT; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(DEFAULT_ANGLE);
  }
}

ParseResult parseCommand(int &servoIndex, int &angle) {
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() < 3 || line.charAt(0) != 'S') {
    return PARSE_ERR_FORMAT;
  }

  int separator = line.indexOf(':');
  if (separator <= 1 || separator == line.length() - 1) {
    return PARSE_ERR_FORMAT;
  }

  servoIndex = line.substring(1, separator).toInt() - 1;
  angle = line.substring(separator + 1).toInt();
  if (servoIndex < 0 || servoIndex >= SERVO_COUNT) {
    return PARSE_ERR_SERVO;
  }

  if (angle < 0 || angle > 180) {
    return PARSE_ERR_ANGLE;
  }

  return PARSE_OK;
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
  ParseResult parseResult = parseCommand(servoIndex, angle);
  if (parseResult != PARSE_OK) {
    if (parseResult == PARSE_ERR_FORMAT) {
      Serial.println(F("ERR: Invalid format"));
    } else if (parseResult == PARSE_ERR_SERVO) {
      Serial.println(F("ERR: Servo index out of range"));
    } else {
      Serial.println(F("ERR: Angle out of range"));
    }
    return;
  }

  servos[servoIndex].write(angle);
  Serial.print(F("OK S"));
  Serial.print(servoIndex + 1);
  Serial.print(F(":"));
  Serial.println(angle);
}
