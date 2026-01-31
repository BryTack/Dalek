#include <Arduino.h>
#line 1 "D:\\Arduino\\Projects\\Daleks\\tests\\Tmc2209_test\\Tmc2209_test.ino"
#include <Tmc2209Motor.h>

// STEP/DIR pins for each motor; sharing a single EN line.
constexpr uint8_t STEP_X = 2;
constexpr uint8_t DIR_X  = 5;
constexpr uint8_t STEP_Y = 3;
constexpr uint8_t DIR_Y  = 6;
constexpr uint8_t STEP_Z = 4;
constexpr uint8_t DIR_Z  = 7;

const uint32_t MIN_PAUSE_US        = 1 * 1000000UL; // at least 1 second
const uint32_t MAX_PAUSE_US        = 4 * 1000000UL; // up to 4 seconds

Tmc2209Motor motorX(STEP_X, DIR_X);
Tmc2209Motor motorY(STEP_Y, DIR_Y);
Tmc2209Motor motorZ(STEP_Z, DIR_Z);

// Per-motor bookkeeping
long     positionSteps[3] = {0, 0, 0}; // relative position from home
bool     needsReturn[3]   = {false, false, false};
bool     returning[3]     = {false, false, false};
uint32_t pauseUntilUs[3]  = {0, 0, 0};

#line 24 "D:\\Arduino\\Projects\\Daleks\\tests\\Tmc2209_test\\Tmc2209_test.ino"
void configureMotor(Tmc2209Motor& motor, uint8_t index, uint32_t now);
#line 30 "D:\\Arduino\\Projects\\Daleks\\tests\\Tmc2209_test\\Tmc2209_test.ino"
void setup();
#line 44 "D:\\Arduino\\Projects\\Daleks\\tests\\Tmc2209_test\\Tmc2209_test.ino"
void handleMotor(Tmc2209Motor& motor, uint32_t now);
#line 88 "D:\\Arduino\\Projects\\Daleks\\tests\\Tmc2209_test\\Tmc2209_test.ino"
void loop();
#line 24 "D:\\Arduino\\Projects\\Daleks\\tests\\Tmc2209_test\\Tmc2209_test.ino"
void configureMotor(Tmc2209Motor& motor, uint8_t index, uint32_t now) {
  motor.setIndex(index);
  motor.setDirSetupDelayUs(100); // add settle time after DIR changes before first step
  motor.begin(now);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for native USB if needed */ }
  Serial.println(F("[init] TMC2209 test: random out-and-back cycles"));

  uint32_t now = micros();
  configureMotor(motorX, 0, now);
  configureMotor(motorY, 1, now);
  configureMotor(motorZ, 2, now);

  randomSeed(analogRead(A0));
}

// Drives one motor through its outbound/return cycle: waits out pauses, picks a random move, then commands the return-to-home leg and logs when homed.
void handleMotor(Tmc2209Motor& motor, uint32_t now) {

  if (!motor.isIdle()) {
    return;
  }

  uint8_t index = motor.index();

  // If we just finished a return-to-home move, finalize and schedule pause.
  if (returning[index]) {
    positionSteps[index] = 0;
    returning[index] = false;
    needsReturn[index]  = false;
    pauseUntilUs[index] = now + random(MIN_PAUSE_US, static_cast<long>(MAX_PAUSE_US) + 1);
    Serial.print(F("[cycle] Motor "));
    Serial.print(index);
    Serial.println(F(" homed; pausing before next move"));
    return;
  }

  // Respect pause after a full out-and-back cycle.
  if (static_cast<int32_t>(now - pauseUntilUs[index]) < 0) {
    return;
  }

  if (!needsReturn[index]) {
    // Pick a random move: 200..800 steps, random direction.
    long steps = static_cast<long>(random(1, 4)) * STEPS_PER_REV;
    if (random(2) == 0) {
      steps = -steps;
    }
    needsReturn[index]   = true;
    positionSteps[index] += steps; // expected position relative to home
    motor.startMove(steps, STEP_INTERVAL_US, now);
  } else {
    // Return directly to home (absolute position 0).
    long stepsToHome = -positionSteps[index];
    if (stepsToHome != 0) {
      motor.startMove(stepsToHome, STEP_INTERVAL_US, now);
      returning[index] = true;
    }
  }
}

void loop() {
  uint32_t now = micros();

  // Always pump updates so in-flight moves progress/finish.
  motorX.update(now);
  motorY.update(now);
  motorZ.update(now);

  handleMotor(motorX, now);
  handleMotor(motorY, now);
  handleMotor(motorZ, now);
}

