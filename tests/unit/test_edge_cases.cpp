/**
 * @file test_edge_cases.cpp
 * @brief Edge-case and stress tests: boundary values, rapid updates, persistence.
 */

#include "test_helpers.hpp"
#include <cstring>

// ── Boundary values ──

TEST_F(NvsTestFixture, Int8BothExtremes) {
    EXPECT_OK(Param_SetTinyOffset(-128));
    EXPECT_EQ(Param_GetTinyOffset(), (int8_t)-128);
    EXPECT_OK(Param_SetTinyOffset(127));
    EXPECT_EQ(Param_GetTinyOffset(), (int8_t)127);
}

TEST_F(NvsTestFixture, Int16BothExtremes) {
    EXPECT_OK(Param_SetAltitude(-32768));
    EXPECT_EQ(Param_GetAltitude(), (int16_t)-32768);
    EXPECT_OK(Param_SetAltitude(32767));
    EXPECT_EQ(Param_GetAltitude(), (int16_t)32767);
}

TEST_F(NvsTestFixture, Uint32BothExtremes) {
    EXPECT_OK(Param_SetSerialNum(0U));
    EXPECT_EQ(Param_GetSerialNum(), 0U);
    EXPECT_OK(Param_SetSerialNum(4294967295U));
    EXPECT_EQ(Param_GetSerialNum(), 4294967295U);
}

TEST_F(NvsTestFixture, Uint64Max) {
    EXPECT_OK(Param_SetDeviceUID(0xFFFFFFFFFFFFFFFFULL));
    EXPECT_EQ(Param_GetDeviceUID(), 0xFFFFFFFFFFFFFFFFULL);
}

// ── Rapid-fire updates (dirty flag stress) ──

TEST_F(NvsTestFixture, RapidUint8Writes) {
    for (int i = 0; i < 256; i++) {
        Param_SetBrightness((uint8_t)i);
    }
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)255);
}

TEST_F(NvsTestFixture, RapidBoolToggles) {
    for (int i = 0; i < 1000; i++) {
        Param_SetAdminLock(i % 2 == 0);
    }
    // i=999: 999%2==1, so (999%2==0) is false
    EXPECT_EQ(Param_GetAdminLock(), false);
}

TEST_F(NvsTestFixture, RapidFloatWrites) {
    for (int i = 0; i < 500; i++) {
        Param_SetTempReading((float)i * 0.1f);
    }
    EXPECT_GT(Param_GetTempReading(), 0.0f);
}

// ── Persistence: save and verify from controller struct ──

TEST_F(NvsTestFixture, ExplicitSave) {
    Param_SetLetter('Z');
    Param_SetBrightness(42);
    NvsConfig_SaveDirtyParameters();
    // After save, dirty flags should be cleared
    EXPECT_FALSE(g_nvsconfig_controller.Letter.is_dirty);
    EXPECT_FALSE(g_nvsconfig_controller.Brightness.is_dirty);
}

TEST_F(NvsTestFixture, DirtyFlagSetOnChange) {
    Param_SetLetter('Z');
    EXPECT_TRUE(g_nvsconfig_controller.Letter.is_dirty);
    EXPECT_FALSE(g_nvsconfig_controller.Letter.is_default);
}

TEST_F(NvsTestFixture, IsDefaultFlagOnReset) {
    Param_SetLetter('Z');
    EXPECT_FALSE(g_nvsconfig_controller.Letter.is_default);
    Param_ResetLetter();
    EXPECT_TRUE(g_nvsconfig_controller.Letter.is_default);
}

// ── Array edge cases ──

TEST_F(NvsTestFixture, ArrayCopyExactSize) {
    // Copy buffer exactly matches required size
    int32_t dest[6];
    EXPECT_OK(Param_CopyCalibPoints(dest, sizeof(dest)));
    EXPECT_EQ(dest[0], (int32_t)-1000);
    EXPECT_EQ(dest[5], (int32_t)2000);
}

TEST_F(NvsTestFixture, ArrayGetNullLengthPtr) {
    // Passing NULL for out_length should not crash
    const uint16_t* rgb = Param_GetRGBColor(nullptr);
    EXPECT_TRUE(rgb != nullptr);
    EXPECT_EQ(rgb[0], (uint16_t)255);
}
