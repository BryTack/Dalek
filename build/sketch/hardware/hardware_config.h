#line 1 "D:\\Arduino\\Projects\\Daleks\\hardware\\hardware_config.h"
#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Counts / sizes
// ---------------------------------------------------------------------------

#define HW_NUM_TURNTABLES        5
#define HW_NUM_STRIPS            13

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

// Distance from camera to path (meters)
#define HW_DISTANCE_CAMERA_TO_PATH_M      3.0f

// Length of the turntable line (meters)
// Turntable 0 at 0.0 m, last at this position.
#define HW_TURNTABLE_LINE_LENGTH_M        2.0f

// Camera horizontal FOV (degrees)
#define HW_CAMERA_FOV_DEG                 120.0f

// Degrees → radians
#define HW_DEGREES_TO_RADIANS             (PI / 180.0f)

// ---------------------------------------------------------------------------
// Stepper + gearing
// ---------------------------------------------------------------------------

// Common: 200 full steps / rev (1.8°)
#define HW_MOTOR_FULL_STEPS_PER_REV       200

// Driver microstepping (e.g. 16 = 1/16)
#define HW_MICROSTEPS_PER_FULL_STEP       16

// Gear ratio motor_rev : turntable_rev (1.0 = direct)
#define HW_GEAR_RATIO_MOTOR_TO_TURNTABLE  1.0f

// Total microsteps for one complete turntable revolution
#define HW_TOTAL_MICROSTEPS_PER_REV  \
  ((long)(HW_MOTOR_FULL_STEPS_PER_REV * \
          HW_MICROSTEPS_PER_FULL_STEP * \
          HW_GEAR_RATIO_MOTOR_TO_TURNTABLE))

#endif  // HARDWARE_CONFIG_H
