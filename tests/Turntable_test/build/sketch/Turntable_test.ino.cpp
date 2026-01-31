#include <Arduino.h>
#line 1 "D:\\Arduino\\Projects\\Daleks\\tests\\Turntable_test\\Turntable_test.ino"
#include <DalekHardwareConfig.h>
#include <TurntableTmc2209.h>
#include <IRDalek.h>

constexpr size_t   TURNTABLE_COUNT = 3;
constexpr float    TURN_ANGLE_DEG  = 45.0f;
constexpr uint32_t PAUSE_MS        = 750;  // small pause between moves

TurntableTmc2209::MotorConfig motorCfgs[TURNTABLE_COUNT]{
  {MOTOR1_STEP_PIN, MOTOR1_DIR_PIN, DEFAULT_ENABLE_PIN, static_cast<long>(MOTOR_EFFECTIVE_STEPS_PER_REV), MOTOR_TO_TURNTABLE_GEAR_RATIO},
  {MOTOR2_STEP_PIN, MOTOR2_DIR_PIN, DEFAULT_ENABLE_PIN, static_cast<long>(MOTOR_EFFECTIVE_STEPS_PER_REV), MOTOR_TO_TURNTABLE_GEAR_RATIO},
  {MOTOR3_STEP_PIN, MOTOR3_DIR_PIN, DEFAULT_ENABLE_PIN, static_cast<long>(MOTOR_EFFECTIVE_STEPS_PER_REV), MOTOR_TO_TURNTABLE_GEAR_RATIO}
};

TurntableTmc2209 turntables[TURNTABLE_COUNT]{
  TurntableTmc2209(motorCfgs[0]),
  TurntableTmc2209(motorCfgs[1]),
  TurntableTmc2209(motorCfgs[2])
};

enum class Stage {
  MovePos,
  WaitPos,
  MoveNeg,
  WaitNeg
};

struct TurntableState {
  TurntableTmc2209& tt;
  Stage             stage;
  uint32_t          waitStartMs;
  float             targetDeg;
};

TurntableState states[TURNTABLE_COUNT]{
  {turntables[0], Stage::MovePos, 0, 0.0f},
  {turntables[1], Stage::MovePos, 0, 0.0f},
  {turntables[2], Stage::MovePos, 0, 0.0f}
};

// Optional logging scope for table motion commands (not IR events)
enum class TableLogMode { Off, Table1Only, All };
TableLogMode tableLogMode = TableLogMode::Table1Only;
bool logTableMoves       = true; // gated by tableLogMode

bool     pauseActive   = false;
uint32_t pauseEndMs    = 0;
bool     homeRequested = false;
bool     homeIssued[TURNTABLE_COUNT]{false, false, false};
uint32_t homeLogLastMs = 0;

// IR-triggered helper: cancels any active moves and re-arms homing for all tables.
#line 53 "D:\\Arduino\\Projects\\Daleks\\tests\\Turntable_test\\Turntable_test.ino"
void cancelAndHome(uint32_t nowMicros);
#line 68 "D:\\Arduino\\Projects\\Daleks\\tests\\Turntable_test\\Turntable_test.ino"
void setup();
#line 98 "D:\\Arduino\\Projects\\Daleks\\tests\\Turntable_test\\Turntable_test.ino"
void loop();
#line 53 "D:\\Arduino\\Projects\\Daleks\\tests\\Turntable_test\\Turntable_test.ino"
void cancelAndHome(uint32_t nowMicros) {
  Serial.println(F("[TurntableTest] IR command received -> homing, then will pause for 20s"));
  pauseActive   = false;
  pauseEndMs    = 0;

  for (size_t i = 0; i < TURNTABLE_COUNT; ++i) {
    homeIssued[i]       = false;
    states[i].waitStartMs = 0;
    states[i].stage     = Stage::MovePos; // reset sequence
    states[i].tt.stop(nowMicros);         // halt any active move immediately
  }

  homeRequested = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial on boards that need it
  }
  Serial.println(F("[TurntableTest] starting"));

  Serial.print(F("[TurntableTest] motorStepsPerRev="));
  Serial.print(MOTOR_EFFECTIVE_STEPS_PER_REV);
  Serial.print(F(", Motor to turntable gearRatio="));
  Serial.print(MOTOR_TO_TURNTABLE_GEAR_RATIO);
  Serial.print(F(", Motor steps per motor degree="));
  Serial.print(MOTOR_EFFECTIVE_STEPS_PER_REV / 360.0f);
  Serial.print(F(", Motor steps per turntable degree="));
  Serial.println((MOTOR_EFFECTIVE_STEPS_PER_REV * MOTOR_TO_TURNTABLE_GEAR_RATIO) / 360.0f);

  uint32_t now = micros();

  for (size_t i = 0; i < TURNTABLE_COUNT; ++i) {
    turntables[i].begin(now);
    Serial.print(F("[TurntableTest] turntable "));
    Serial.print(i);
    Serial.println(F(" ready"));
  }

  TableLogMode tableLogMode = TableLogMode::Table1Only;
  IRDalek_begin();  //Defaults to pin 12
  delay(500);
}

void loop() {

  uint32_t nowMicros = micros();
  uint32_t nowMs     = millis();
  bool irCommandSeen = IRDalek_read_any();

  if (irCommandSeen) {
    cancelAndHome(nowMicros);
  }

  bool allIdle = true;
  bool allHomeIssued = true;

  auto shouldLogTable = [&](size_t idx) {
    return logTableMoves &&
           (tableLogMode == TableLogMode::All ||
            (tableLogMode == TableLogMode::Table1Only && idx == 0));
  };

  auto logMove = [&](size_t idx, const __FlashStringHelper* msg, float value = 0.0f, bool useValue = false) {
    if (!shouldLogTable(idx)) {
      return;
    }
    Serial.print(F("[TurntableTest] table "));
    Serial.print(idx);
    Serial.print(F(" "));
    Serial.print(msg);
    if (useValue) {
      Serial.print(F(" "));
      Serial.println(value);
    } else {
      Serial.println();
    }
  };

  for (size_t i = 0; i < TURNTABLE_COUNT; ++i) {
    TurntableState& state = states[i];
    state.tt.update(nowMicros);

    if (!state.tt.isIdle()) {
      allIdle = false;
    }

    if (state.tt.isError()) {
      Serial.print(F("[TurntableTest] motor error detected on table "));
      Serial.println(i);
      continue;
    }

    bool logTable = shouldLogTable(i);

    if (homeRequested) {
      if (!homeIssued[i] && state.tt.isIdle()) {
        if (logTable) {
          Serial.print(F("[TurntableTest] table "));
          Serial.print(i);
          Serial.print(F(" currentAngleDeg before homing: "));
          Serial.println(state.tt.currentAngleDeg());
        }
        if (state.tt.moveToHome(SWEEP_SPEED_DPS, SWEEP_TIMEOUT_MS, nowMicros)) {
          state.tt.setLogEnabled(logTable);           // show driver logs as it moves toward zero
          state.tt.setLogEveryDegrees(0.5f);          // finer-grained logging while homing
          logTableMoves = true;                       // enable high-level move logging (scoped by mode)
          homeIssued[i] = true;
          if (logTable) {
            Serial.print(F("[TurntableTest] table "));
            Serial.print(i);
            Serial.println(F(" homing to 0 deg"));
            logMove(i, F("homing to 0 deg"));
          }
        } else {
          if (logTable) {
            Serial.print(F("[TurntableTest] table "));
            Serial.print(i);
            Serial.println(F(" homing command rejected"));
          }
        }
      }
      allHomeIssued = allHomeIssued && homeIssued[i];
      // Periodic status while homing
      if (logTable && nowMs - homeLogLastMs >= 500) {
        Serial.print(F("[TurntableTest] table "));
        Serial.print(i);
        Serial.print(F(" currentAngleDeg during homing: "));
        Serial.println(state.tt.currentAngleDeg());
        homeLogLastMs = nowMs;
      }
      continue;  // skip normal sequencing while homing is requested
    }

    if (pauseActive) {
      continue;  // pause active: let updates run but skip new motion commands
    }

    auto ensureWait = [&](TurntableState& s, Stage nextStage) {
      if (!s.tt.isIdle()) {
        return false;
      }
      if (s.waitStartMs == 0) {
        s.waitStartMs = nowMs;
      }
      if (nowMs - s.waitStartMs >= PAUSE_MS) {
        s.waitStartMs = 0;
        s.stage       = nextStage;
        return true;
      }
      return false;
    };

    switch (state.stage) {
      case Stage::MovePos:
        if (state.tt.isIdle()) {
          state.targetDeg = TURN_ANGLE_DEG;
          logMove(i, F("moveToAngle"), state.targetDeg, true);
          if (state.tt.moveToAngle(state.targetDeg, SWEEP_SPEED_DPS, SWEEP_TIMEOUT_MS, nowMicros)) {
            state.stage = Stage::WaitPos;
          }
        }
        break;

      case Stage::WaitPos:
        ensureWait(state, Stage::MoveNeg);
        break;

      case Stage::MoveNeg:
        if (state.tt.isIdle() &&
            state.tt.moveToAngle(-TURN_ANGLE_DEG, SWEEP_SPEED_DPS, SWEEP_TIMEOUT_MS, nowMicros)) {
          logMove(i, F("moveToAngle"), -TURN_ANGLE_DEG, true);
          state.stage = Stage::WaitNeg;
        }
        break;

      case Stage::WaitNeg:
        if (ensureWait(state, Stage::MovePos)) {
          // Next loop will move back to +TURN_ANGLE_DEG.
        }
        break;
    }
  }

  if (homeRequested && allHomeIssued && allIdle && !pauseActive) {
    for (size_t i = 0; i < TURNTABLE_COUNT; ++i) {
      states[i].tt.setLogEnabled(false);
    }
    logTableMoves = false; // back to default until next homing
    Serial.println(F("[TurntableTest] homed; 20s timer started"));
    pauseActive   = true;
    pauseEndMs    = nowMs + 20000UL;
    homeRequested = false;
  }

  if (pauseActive && nowMs >= pauseEndMs && allIdle) {
    pauseActive   = false;
    for (auto& issued : homeIssued) {
      issued = false;
    }
    for (auto& s : states) {
      s.stage       = Stage::MovePos;
      s.waitStartMs = 0;
    }
    Serial.println(F("[TurntableTest] 20s pause complete; normal movement resumed"));
  }
}

