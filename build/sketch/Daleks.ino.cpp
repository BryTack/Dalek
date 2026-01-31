#line 1 "D:\\Arduino\\Projects\\Daleks\\Daleks.ino"
// ---Project Dalek ---
// Author BryTack

#include <Arduino.h>

// My libraries
#include <Turntable.h>
#include <Dalek.h>

// Logger is global and accessed via LoggerAlias
#include "utils/SerialLogger.h"
#include "utils/LoggerAlias.h"

// Definition: create the actual reference
SerialLog &logger = SerialLog::instance();

Turntable::Limits turntableLimits{0.0f, 270.0f, true};

constexpr uint8_t STEP_X = 11;
constexpr uint8_t DIR_X = 6;
constexpr uint8_t EN_X = 4;
constexpr int MOTOR_STEPS_PER_REV = 2048;
constexpr int MOTOR_MAX_SPS = 1;
constexpr int GEAR_RATIO = 1;

Turntable tablel1(STEP_X, DIR_X, EN_X,
                 MOTOR_STEPS_PER_REV,
                 MOTOR_MAX_SPS,
                 GEAR_RATIO,
                 turntableLimits);

Dalek dalek(tablel1); // attach table1 to first dalek


// Track idle timing
uint32_t lastIdleTime = 0;

// Minimum idle time before triggering a new move
const uint32_t idleTriggerDelay = 200; 

#line 41 "D:\\Arduino\\Projects\\Daleks\\Daleks.ino"
void setup();
#line 59 "D:\\Arduino\\Projects\\Daleks\\Daleks.ino"
void loop();
#line 41 "D:\\Arduino\\Projects\\Daleks\\Daleks.ino"
void setup()
{

    logger.begin(115200);             // Initialize logger (defaults to 115200)
    logger.setLevel(LogLevel::DEBUG); // Show full detail
    logger.setShowTime(false);        // No timestamps unless you want them

    delay(500);

    logger.info("Initializing ...");
    //    InitializeGeometryLookupTables();

    uint32_t now = millis();
    dalek.begin(now);

    logger.info("Initializing done.\n");
}

void loop()
{
    uint32_t now = millis();
    dalek.update(now); // keep FSM ticking

    if (true /*some trigger to start patrol*/)
    {
        dalek.slowPatrol(now); // start slow patrol
    }
}

