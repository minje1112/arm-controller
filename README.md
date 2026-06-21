# arm-controller

Arduino Uno R3 repository for controlling a 4-servo robotic arm.

## Hardware

- Arduino Uno R3
- 4x hobby servos
- External 5V servo power supply (recommended)
- Common GND between Arduino and servo power supply

## Wiring

- Servo 1 signal -> D3
- Servo 2 signal -> D5
- Servo 3 signal -> D6
- Servo 4 signal -> D9
- All servo grounds -> GND

## Sketch

Upload `/home/runner/work/arm-controller/arm-controller/src/arm_controller.ino` to the Arduino Uno R3.

Default servo position is 90 degrees for all four joints.

Serial control at `115200` baud:

- `S1:120` -> set servo 1 to 120°
- `S4:30` -> set servo 4 to 30°

`S<servo_index>:<angle>` where:

- `servo_index` is `1` to `4`
- `angle` is `0` to `180`