#line 1 "D:\\Arduino\\Projects\\Daleks\\geometry\\geometry_lookup.h"
#ifndef GEOMETRY_LOOKUP_H
#define GEOMETRY_LOOKUP_H

#include <Arduino.h>
#include "../hardware/hardware_config.h"

#include "../utils/LoggerAlias.h"

// ---------------------------------------------------------------------------
// External data (defined once in Daleks.ino)
// ---------------------------------------------------------------------------

extern float g_turntableXPositionsMeters[HW_NUM_TURNTABLES];
extern float g_stripTargetXPositionsMeters[HW_NUM_STRIPS];

extern float g_turntableAnglesRadians[HW_NUM_TURNTABLES][HW_NUM_STRIPS];
extern long  g_turntableSteps[HW_NUM_TURNTABLES][HW_NUM_STRIPS];

extern long  g_turntableMinSteps[HW_NUM_TURNTABLES];
extern long  g_turntableMaxSteps[HW_NUM_TURNTABLES];
extern long  g_turntableRequiredStepRange[HW_NUM_TURNTABLES];

// ---------------------------------------------------------------------------
// Initialization routine (header-only, for now)
// ---------------------------------------------------------------------------

inline void InitializeGeometryLookupTables()
{
    // Microsteps per radian for the turntable
    const float MICROSTEPS_PER_RADIAN =
        (float)HW_TOTAL_MICROSTEPS_PER_REV / (2.0f * PI);

    // -----------------------------------------------------
    // 1. Turntable X positions relative to camera (x = 0)
    // -----------------------------------------------------

    const float cameraPhysicalX =
        HW_TURNTABLE_LINE_LENGTH_M / 2.0f;

    float spacingMeters = 0.0f;
    if (HW_NUM_TURNTABLES > 1)
    {
        spacingMeters =
            HW_TURNTABLE_LINE_LENGTH_M /
            (float)(HW_NUM_TURNTABLES - 1);
    }

    for (uint8_t i = 0; i < HW_NUM_TURNTABLES; ++i)
    {
        float physicalX = (float)i * spacingMeters;
        g_turntableXPositionsMeters[i] = physicalX - cameraPhysicalX;
    }

    // -----------------------------------------------------
    // 2. Target X on path for each strip
    // -----------------------------------------------------

    const float halfFovRadians =
        (HW_CAMERA_FOV_DEG * HW_DEGREES_TO_RADIANS) / 2.0f;

    for (uint8_t s = 0; s < HW_NUM_STRIPS; ++s)
    {
        float stripCenterFraction =
            ((float)s + 0.5f) / (float)HW_NUM_STRIPS;   // 0..1

        float u = stripCenterFraction * 2.0f - 1.0f;    // -1..+1

        float phi = u * halfFovRadians;                 // radians

        g_stripTargetXPositionsMeters[s] =
            HW_DISTANCE_CAMERA_TO_PATH_M * tanf(phi);
    }

    // -----------------------------------------------------
    // 3. Angles (radians) for each turntable & strip
    // -----------------------------------------------------

    for (uint8_t i = 0; i < HW_NUM_TURNTABLES; ++i)
    {
        float turntableX = g_turntableXPositionsMeters[i];

        for (uint8_t s = 0; s < HW_NUM_STRIPS; ++s)
        {
            float targetX = g_stripTargetXPositionsMeters[s];
            float dx = targetX - turntableX;

            g_turntableAnglesRadians[i][s] =
                atan2f(dx, HW_DISTANCE_CAMERA_TO_PATH_M);
        }
    }

    // -----------------------------------------------------
    // 4. Convert angles → steps and compute min/max/range
    // -----------------------------------------------------

    for (uint8_t i = 0; i < HW_NUM_TURNTABLES; ++i)
    {
        long minSteps =  2147483647L;   // +2^31 - 1
        long maxSteps = -2147483648L;   // -2^31

        for (uint8_t s = 0; s < HW_NUM_STRIPS; ++s)
        {
            float angle = g_turntableAnglesRadians[i][s];

            long steps = (long)lroundf(angle * MICROSTEPS_PER_RADIAN);

            g_turntableSteps[i][s] = steps;

            if (steps < minSteps) minSteps = steps;
            if (steps > maxSteps) maxSteps = steps;
        }

        g_turntableMinSteps[i]          = minSteps;
        g_turntableMaxSteps[i]          = maxSteps;
        g_turntableRequiredStepRange[i] = maxSteps - minSteps;
    }

// if required, log details
/*
    logger.info("Per-turntable required microstep ranges:");

    for (uint8_t i = 0; i < HW_NUM_TURNTABLES; ++i)
    {
        logger.infof("TT%u: min = %lu   max = %lu   range = %lu",
                     i,
                     g_turntableMinSteps[i],
                     g_turntableMaxSteps[i],
                     g_turntableRequiredStepRange[i]);
    }

    logger.info(""); // blank line

    logger.info("Example: steps for middle strip:");

    uint8_t middleStrip = HW_NUM_STRIPS / 2;
    for (uint8_t i = 0; i < HW_NUM_TURNTABLES; ++i)
    {
        logger.infof("TT%u: %lu", i, g_turntableSteps[i][middleStrip]);
    }
*/
}

#endif  // GEOMETRY_LOOKUP_H
