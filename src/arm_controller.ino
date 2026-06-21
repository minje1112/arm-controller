#include <Servo.h>

static const byte SERVO_COUNT = 4;
static const byte SERVO_PINS[SERVO_COUNT] = {3, 5, 6, 9};
static const byte DEFAULT_ANGLE = 90;
static const unsigned long SERIAL_TIMEOUT_MS = 20;

Servo servos[SERVO_COUNT];

enum ParseResult {
  PARSE_OK,
  PARSE_ERR_FORMAT,
  PARSE_ERR_SERVO,
  PARSE_ERR_ANGLE
};

struct ParsedCommand {
  ParseResult result;
  int servoIndex;
  int angle;
};

void attachServos() {
  for (byte i = 0; i < SERVO_COUNT; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(DEFAULT_ANGLE);
  }
}

bool isAllDigits(const String &value) {
  unsigned int length = value.length();
  if (length == 0) {
    return false;
  }

  for (unsigned int i = 0; i < length; i++) {
    if (!isDigit(value.charAt(i))) {
      return false;
    }
  }

  return true;
}

ParsedCommand parseCommand() {
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() < 3 || line.charAt(0) != 'S') {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  int separator = line.indexOf(':');
  // Require at least one digit for servo token and angle token.
  if (separator <= 1 || separator >= (int)line.length() - 1) {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  String servoToken = line.substring(1, separator);
  String angleToken = line.substring(separator + 1);
  if (!isAllDigits(servoToken) || !isAllDigits(angleToken)) {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  int servoIndex = servoToken.toInt() - 1;
  int angle = angleToken.toInt();
  if (servoIndex < 0 || servoIndex >= SERVO_COUNT) {
    return {PARSE_ERR_SERVO, servoIndex, angle};
  }

  if (angle < 0 || angle > 180) {
    return {PARSE_ERR_ANGLE, servoIndex, angle};
  }

  return {PARSE_OK, servoIndex, angle};
}

void setup() {
  Serial.begin(115200);
  // Keep command reads responsive for short serial command lines.
  Serial.setTimeout(SERIAL_TIMEOUT_MS);
  attachServos();
  Serial.println(F("4-servo arm controller ready"));
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  ParsedCommand command = parseCommand();
  if (command.result != PARSE_OK) {
    if (command.result == PARSE_ERR_FORMAT) {
      Serial.println(F("ERR: Invalid format"));
    } else if (command.result == PARSE_ERR_SERVO) {
      Serial.println(F("ERR: Servo index out of range (1-4)"));
    } else {
      Serial.println(F("ERR: Angle out of range (0-180)"));
    }
    return;
  }

  servos[command.servoIndex].write(command.angle);
  Serial.print(F("OK S"));
  Serial.print(command.servoIndex + 1);
  Serial.print(F(":"));
  Serial.println(command.angle);
}
