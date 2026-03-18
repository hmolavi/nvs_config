/**
 * @file test_helpers.hpp
 * @brief Shared fixture and utilities for nvs_config unit tests.
 */

#pragma once

#include "test_framework.hpp"
#include "nvs_config.h"

/**
 * Base fixture that resets all parameters to defaults and sets
 * security level to 0 (admin) before each test.
 */
struct NvsTestFixture {
    void SetUp() {
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

    void TearDown() {}
};
