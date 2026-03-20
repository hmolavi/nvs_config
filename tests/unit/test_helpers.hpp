/**
 * @file test_helpers.hpp
 * @brief Shared fixture infrastructure for the CppUTest unit test suite.
 *
 * Defines:
 *  - nvs_reset_all_params(): resets every parameter to its default value;
 *    used by all TEST_GROUP setup() functions.
 *  - TEST_GROUP_CppUTestGroupNvsTestFixture: the base fixture class, written
 *    directly (not via the TEST_GROUP macro) so the class definition can live
 *    in this header while the mandatory `int externTestGroupNvsTestFixture`
 *    variable is defined exactly once in test_groups.cpp.
 *
 * Fixtures with additional setup (WearLevelFixture, CallbackFixture,
 * SecurityLevel1Fixture, SecurityLevel2Fixture) use TEST_GROUP locally in
 * their respective .cpp files, calling nvs_reset_all_params() as their first
 * step.
 */

#pragma once

#include <CppUTest/TestHarness.h>
#include "cpputest_compat.hpp"
#include "nvs_config.h"

#include <cstring>

/**
 * Reset every parameter to its compiled-in default and elevate to admin level.
 * Called at the start of every test group's setup().
 *
 * Note: Param_Reset* returns ESP_FAIL when the value is already at its default
 * (by design). Those return values are intentionally ignored here.
 */
inline void nvs_reset_all_params()
{
    NvsConfig_SecureLevelChange(0);
    Param_ResetLetter();
    Param_ResetAdminLock();
    Param_ResetTinyOffset();
    Param_ResetBrightness();
    Param_ResetBigTimestamp();
    Param_ResetDeviceUID();
    Param_ResetAltitude();
    Param_ResetSampleRate();
    Param_ResetCalibOffset();
    Param_ResetSerialNum();
    Param_ResetTempReading();
    Param_ResetGpsLongitude();
    Param_ResetMaxNameTestXXX1();
    Param_ResetDeviceName();
    Param_ResetBytePattern();
    Param_ResetCalibPoints();
    Param_ResetThresholds();
    Param_ResetFeatureFlags();
    Param_ResetRGBColor();
}

/**
 * Base fixture class for the majority of unit tests.
 *
 * Written out explicitly (not via TEST_GROUP macro) so the class definition
 * can appear in this header — visible to every TU — while the companion
 * external-linkage variable `int externTestGroupNvsTestFixture` is defined
 * exactly once in test_groups.cpp, avoiding a duplicate-symbol link error.
 *
 * CppUTest's TEST_GROUP(NvsTestFixture) expands to:
 *   extern int externTestGroupNvsTestFixture;
 *   int    externTestGroupNvsTestFixture = 0;               ← definition (once!)
 *   struct TEST_GROUP_CppUTestGroupNvsTestFixture : public Utest { ... };
 *
 * The struct part is ODR-safe in a header; the int definition is not.
 */
extern int externTestGroupNvsTestFixture;  // defined in test_groups.cpp

struct TEST_GROUP_CppUTestGroupNvsTestFixture : public Utest
{
    void setup()    { nvs_reset_all_params(); }
    void teardown() {}
};
