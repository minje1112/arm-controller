#include <Servo.h>

static const byte SERVO_COUNT = 4;
static const byte SERVO_PINS[SERVO_COUNT] = {3, 5, 6, 9};
static const byte DEFAULT_ANGLE = 90;
static const unsigned long SERIAL_TIMEOUT_MS = 20;
static const byte SERIAL_BUFFER_SIZE = 32;
static const byte MIN_COMMAND_LENGTH = 3;
static const byte SERVO_INDEX_OFFSET = 1;
static const byte MIN_SERVO_ANGLE = 0;
static const byte MAX_SERVO_ANGLE = 180;

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
  if (line == NULL || strlen(line) < MIN_COMMAND_LENGTH || line[0] != 'S') {
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
  if (servoValue < SERVO_INDEX_OFFSET || servoValue > SERVO_COUNT) {
    return {PARSE_ERR_SERVO, 0, (int)angleValue};
  }

  if (angleValue < MIN_SERVO_ANGLE || angleValue > MAX_SERVO_ANGLE) {
    return {PARSE_ERR_ANGLE, (int)servoValue - SERVO_INDEX_OFFSET, 0};
  }

  int servoIndex = (int)servoValue - SERVO_INDEX_OFFSET;
  int angle = (int)angleValue;
  if (servoIndex < 0 || servoIndex >= SERVO_COUNT) {
    return {PARSE_ERR_SERVO, servoIndex, angle};
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
  size_t charsRead = Serial.readBytesUntil('\n', commandBuffer, SERIAL_BUFFER_SIZE - 1);
  commandBuffer[charsRead] = '\0';
  if (charsRead == SERIAL_BUFFER_SIZE - 1) {
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
