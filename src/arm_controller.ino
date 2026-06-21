#include <Servo.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const byte SERVO_COUNT = 4;
static const byte SERVO_PINS[SERVO_COUNT] = {3, 5, 6, 9};
static const byte DEFAULT_ANGLE = 90;
static const unsigned long SERIAL_TIMEOUT_MS = 20;
static const byte SERIAL_BUFFER_SIZE = 32;

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

bool isAllDigits(const char *value) {
  if (value[0] == '\0') {
    return false;
  }

  for (const char *current = value; *current != '\0'; current++) {
    if (!isdigit((unsigned char)*current)) {
      return false;
    }
  }

  return true;
}

void trimInPlace(char *value) {
  char *start = value;
  while (*start != '\0' && isspace((unsigned char)*start)) {
    start++;
  }

  if (start != value) {
    memmove(value, start, strlen(start) + 1);
  }

  size_t length = strlen(value);
  while (length > 0 && isspace((unsigned char)value[length - 1])) {
    value[--length] = '\0';
  }
}

void flushUntilNewline() {
  while (Serial.available()) {
    if (Serial.read() == '\n') {
      break;
    }
  }
}

ParsedCommand parseCommand(const char *line) {
  if (line == NULL || strlen(line) < 3 || line[0] != 'S') {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  const char *separator = strchr(line, ':');
  // Require at least one digit for servo token and angle token.
  if (separator == NULL || separator <= line + 1 || *(separator + 1) == '\0') {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  size_t servoTokenLength = separator - (line + 1);
  if (servoTokenLength >= SERIAL_BUFFER_SIZE) {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  char servoToken[SERIAL_BUFFER_SIZE];
  memcpy(servoToken, line + 1, servoTokenLength);
  servoToken[servoTokenLength] = '\0';

  const char *angleToken = separator + 1;
  if (!isAllDigits(servoToken) || !isAllDigits(angleToken)) {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  long servoValue = strtol(servoToken, NULL, 10);
  long angleValue = strtol(angleToken, NULL, 10);
  if (servoValue > INT_MAX || angleValue > INT_MAX) {
    return {PARSE_ERR_FORMAT, 0, 0};
  }

  int servoIndex = (int)servoValue - 1;
  int angle = (int)angleValue;
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

  char commandBuffer[SERIAL_BUFFER_SIZE];
  size_t bytesRead = Serial.readBytesUntil('\n', commandBuffer, SERIAL_BUFFER_SIZE - 1);
  commandBuffer[bytesRead] = '\0';
  if (bytesRead == SERIAL_BUFFER_SIZE - 1) {
    flushUntilNewline();
  }
  trimInPlace(commandBuffer);

  ParsedCommand command = parseCommand(commandBuffer);
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
