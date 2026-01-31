#include <Arduino.h>
#line 1 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
#include <Drv8825Motor.h>

// STEP/DIR pins for each motor; sharing a single EN line.
constexpr uint8_t STEP_X = 2;
constexpr uint8_t DIR_X  = 5;
constexpr uint8_t STEP_Y = 3;
constexpr uint8_t DIR_Y  = 6;
constexpr uint8_t STEP_Z = 4;
constexpr uint8_t DIR_Z  = 7;


constexpr uint16_t PULSE_WIDTH_US    = 1000;    // match the working simple sketch
constexpr uint32_t STEP_INTERVAL_US  = 2000;    // slow to 4 ms between steps to reduce missed steps
constexpr uint32_t MIN_PAUSE_US      = 1 * 1000000; // at least 2 seconds
constexpr uint32_t MAX_PAUSE_US      = 4 * 1000000; // up to 10 seconds pause between cycles
constexpr uint16_t STEPS_PER_REV     = 200;     // assumed steps per revolution (for reference)
constexpr uint32_t POST_HOME_WAIT_US    = 15 * 1000000UL;  // wait after homing before resuming

Drv8825Motor motorX(STEP_X, DIR_X);
Drv8825Motor motorY(STEP_Y, DIR_Y);
Drv8825Motor motorZ(STEP_Z, DIR_Z);

// Per-motor bookkeeping
long     positionSteps[3] = {0, 0, 0}; // relative position from home
bool     needsReturn[3]   = {false, false, false};
bool     returning[3]     = {false, false, false};
uint32_t pauseUntilUs[3]  = {0, 0, 0};
enum class ControlState { Running, StopHold, Homing, WaitAfterHome };
ControlState controlState = ControlState::Running;
uint32_t stateDeadlineUs  = 0;
bool     manualSequence   = false; // true while cycling Ctrl+C actions
uint8_t  manualStage      = 0;     // 0=freeze, 1=home, 2=resume

#line 34 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
void configureMotor(Drv8825Motor& motor, uint32_t now);
#line 40 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
void handleCtrlC(uint32_t now);
#line 73 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
void processSerial(uint32_t now);
#line 82 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
void setup();
#line 95 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
void handleMotor(Drv8825Motor& motor, uint8_t index, uint32_t now);
#line 136 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
void loop();
#line 34 "D:\\Arduino\\Projects\\Daleks\\tests\\DRV8825_test\\DRV8825_test.ino"
void configureMotor(Drv8825Motor& motor, uint32_t now) {
  motor.setPulseWidthUs(PULSE_WIDTH_US);
  motor.setDirSetupDelayUs(100); // add settle time after DIR changes before first step
  motor.begin(now);
}

void handleCtrlC(uint32_t now) {
  switch (manualStage) {
    case 0: // freeze
      Serial.println(F("[ctrl-c] Freeze: stopping all motors"));
      motorX.stop(now);
      motorY.stop(now);
      motorZ.stop(now);
      needsReturn[0] = needsReturn[1] = needsReturn[2] = false;
      returning[0] = returning[1] = returning[2] = false;
      pauseUntilUs[0] = pauseUntilUs[1] = pauseUntilUs[2] = 0;
      manualSequence = true;
      controlState  = ControlState::StopHold;
      stateDeadlineUs = UINT32_MAX; // hold until next Ctrl+C
      Serial.print(F("[ctrl-c] positions X/Y/Z = "));
      Serial.print(positionSteps[0]); Serial.print(F("/"));
      Serial.print(positionSteps[1]); Serial.print(F("/"));
      Serial.println(positionSteps[2]);
      break;

    case 1: // home
      Serial.println(F("[ctrl-c] Home: commanding return to zero"));
      controlState = ControlState::Homing;
      break;

    case 2: // resume
      Serial.println(F("[ctrl-c] Resume: running normally"));
      manualSequence   = false;
      controlState = ControlState::Running;
      break;
  }
  manualStage = (manualStage + 1) % 3;
}

void processSerial(uint32_t now) {
  while (Serial.available() > 0) {
    int c = Serial.read();
    if (c == 0x03) { // Ctrl+C
      handleCtrlC(now);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for native USB if needed */ }
  Serial.println(F("[init] DRV8825 test with manual Ctrl+C freeze/home/resume"));

  uint32_t now = micros();
  configureMotor(motorX, now);
  configureMotor(motorY, now);
  configureMotor(motorZ, now);

  randomSeed(analogRead(A0));
}

void handleMotor(Drv8825Motor& motor, uint8_t index, uint32_t now) {
  if (!motor.isIdle()) {
    return;
  }

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
  processSerial(now);

  // Always pump updates so in-flight moves progress/finish.
  motorX.update(now);
  motorY.update(now);
  motorZ.update(now);

  switch (controlState) {
    case ControlState::Running:
      handleMotor(motorX, 0, now);
      handleMotor(motorY, 1, now);
      handleMotor(motorZ, 2, now);
      break;

    case ControlState::StopHold:
      // Manual Ctrl+C sequence holds here until next command.
      break;

    case ControlState::Homing: {
      // Command a home move for any motor not at home and idle.
      if (motorX.isIdle() && positionSteps[0] != 0) {
        Serial.print(F("[home] X steps to home: "));
        Serial.println(-positionSteps[0]);
        motorX.startMove(-positionSteps[0], STEP_INTERVAL_US, now);
        positionSteps[0] = 0;
      }
      if (motorY.isIdle() && positionSteps[1] != 0) {
        Serial.print(F("[home] Y steps to home: "));
        Serial.println(-positionSteps[1]);
        motorY.startMove(-positionSteps[1], STEP_INTERVAL_US, now);
        positionSteps[1] = 0;
      }
      if (motorZ.isIdle() && positionSteps[2] != 0) {
        Serial.print(F("[home] Z steps to home: "));
        Serial.println(-positionSteps[2]);
        motorZ.startMove(-positionSteps[2], STEP_INTERVAL_US, now);
        positionSteps[2] = 0;
      }
      bool allIdle = motorX.isIdle() && motorY.isIdle() && motorZ.isIdle();
      bool allHome = (positionSteps[0] == 0) && (positionSteps[1] == 0) && (positionSteps[2] == 0);
      if (allIdle && allHome) {
        stateDeadlineUs = now + POST_HOME_WAIT_US;
        controlState    = ControlState::WaitAfterHome;
        Serial.println(F("[state] Homed; waiting before resume"));
      }
      break;
    }

    case ControlState::WaitAfterHome:
      // Waiting after home; manual Ctrl+C resume can override.
      break;
  }
}

