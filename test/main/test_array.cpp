/**
 * @file test_array.cpp
 * @brief Unit tests for array parameter operations.
 */

#include "test_helpers.hpp"

void register_array_tests() {} // linker anchor
#include <cstring>

// ── char[16] (DeviceName) ──

TEST_F(NvsTestFixture, CharArrayDefault) {
    size_t len;
    const char* name = Param_GetDeviceName(&len);
    EXPECT_EQ(len, (size_t)16);
    EXPECT_STREQ(name, "Stress");
}

TEST_F(NvsTestFixture, CharArraySetGet) {
    char buf[16] = {0};
    memcpy(buf, "NewDevice", 9);
    EXPECT_OK(Param_SetDeviceName(buf, 16));
    size_t len;
    const char* got = Param_GetDeviceName(&len);
    EXPECT_STREQ(got, "NewDevice");
}

TEST_F(NvsTestFixture, CharArrayCopy) {
    char buf[16] = {0};
    EXPECT_OK(Param_CopyDeviceName(buf, sizeof(buf)));
    EXPECT_STREQ(buf, "Stress");
}

TEST_F(NvsTestFixture, CharArrayReset) {
    char buf[16] = {0};
    memcpy(buf, "Changed", 7);
    Param_SetDeviceName(buf, 16);
    EXPECT_OK(Param_ResetDeviceName());
    size_t len;
    const char* got = Param_GetDeviceName(&len);
    EXPECT_STREQ(got, "Stress");
}

// ── uint8_t[8] (BytePattern) ──

TEST_F(NvsTestFixture, Uint8ArrayDefault) {
    size_t len;
    const uint8_t* bp = Param_GetBytePattern(&len);
    EXPECT_EQ(len, (size_t)8);
    EXPECT_EQ(bp[0], (uint8_t)0xDE);
    EXPECT_EQ(bp[7], (uint8_t)0xBE);
}

TEST_F(NvsTestFixture, Uint8ArraySetGet) {
    const uint8_t data[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    EXPECT_OK(Param_SetBytePattern(data, 8));
    size_t len;
    const uint8_t* got = Param_GetBytePattern(&len);
    EXPECT_MEMEQ(got, data, 8);
}

TEST_F(NvsTestFixture, Uint8ArrayCopy) {
    uint8_t dest[8] = {0};
    EXPECT_OK(Param_CopyBytePattern(dest, sizeof(dest)));
    EXPECT_EQ(dest[0], (uint8_t)0xDE);
    EXPECT_EQ(dest[4], (uint8_t)0xCA);
}

// ── int32_t[6] (CalibPoints) ──

TEST_F(NvsTestFixture, Int32ArrayDefault) {
    size_t len;
    const int32_t* cp = Param_GetCalibPoints(&len);
    EXPECT_EQ(len, (size_t)6);
    EXPECT_EQ(cp[0], (int32_t)-1000);
    EXPECT_EQ(cp[2], (int32_t)0);
    EXPECT_EQ(cp[5], (int32_t)2000);
}

TEST_F(NvsTestFixture, Int32ArrayBoundaryValues) {
    const int32_t data[6] = {-2147483647, -1, 0, 1, 100, 2147483647};
    EXPECT_OK(Param_SetCalibPoints(data, 6));
    size_t len;
    const int32_t* got = Param_GetCalibPoints(&len);
    EXPECT_EQ(got[0], (int32_t)-2147483647);
    EXPECT_EQ(got[5], (int32_t)2147483647);
}

// ── float[4] (Thresholds) ──

TEST_F(NvsTestFixture, FloatArrayDefault) {
    size_t len;
    const float* th = Param_GetThresholds(&len);
    EXPECT_EQ(len, (size_t)4);
    EXPECT_EQ(th[0], 0.001f);
    EXPECT_EQ(th[3], 9999.99f);
}

TEST_F(NvsTestFixture, FloatArraySetGet) {
    const float data[4] = {-999.0f, 0.0f, 0.001f, 999999.0f};
    EXPECT_OK(Param_SetThresholds(data, 4));
    size_t len;
    const float* got = Param_GetThresholds(&len);
    EXPECT_EQ(got[0], -999.0f);
    EXPECT_EQ(got[3], 999999.0f);
}

// ── bool[8] (FeatureFlags) ──

TEST_F(NvsTestFixture, BoolArrayDefault) {
    size_t len;
    const bool* ff = Param_GetFeatureFlags(&len);
    EXPECT_EQ(len, (size_t)8);
    EXPECT_TRUE(ff[0]);
    EXPECT_FALSE(ff[1]);
    EXPECT_TRUE(ff[2]);
    EXPECT_FALSE(ff[3]);
}

TEST_F(NvsTestFixture, BoolArrayFlipAll) {
    const bool flipped[8] = {false, true, false, true, false, true, false, true};
    EXPECT_OK(Param_SetFeatureFlags(flipped, 8));
    size_t len;
    const bool* got = Param_GetFeatureFlags(&len);
    EXPECT_FALSE(got[0]);
    EXPECT_TRUE(got[1]);
}

// ── uint16_t[3] (RGBColor) ──

TEST_F(NvsTestFixture, Uint16ArrayDefault) {
    size_t len;
    const uint16_t* rgb = Param_GetRGBColor(&len);
    EXPECT_EQ(len, (size_t)3);
    EXPECT_EQ(rgb[0], (uint16_t)255);
    EXPECT_EQ(rgb[1], (uint16_t)128);
    EXPECT_EQ(rgb[2], (uint16_t)0);
}

TEST_F(NvsTestFixture, Uint16ArrayBoundaryValues) {
    const uint16_t data[3] = {0, 65535, 32768};
    EXPECT_OK(Param_SetRGBColor(data, 3));
    size_t len;
    const uint16_t* got = Param_GetRGBColor(&len);
    EXPECT_EQ(got[0], (uint16_t)0);
    EXPECT_EQ(got[1], (uint16_t)65535);
    EXPECT_EQ(got[2], (uint16_t)32768);
}

// ── Error handling ──

TEST_F(NvsTestFixture, ArrayOversizedLength) {
    const uint8_t data[9] = {0};
    EXPECT_ERR(Param_SetBytePattern(data, 9), ESP_ERR_INVALID_SIZE);
}

TEST_F(NvsTestFixture, ArrayUndersizedCopyBuffer) {
    uint8_t small[4];
    EXPECT_ERR(Param_CopyBytePattern(small, sizeof(small)), ESP_ERR_INVALID_SIZE);
}

TEST_F(NvsTestFixture, ArraySameValueNoChange) {
    size_t len;
    const int32_t* cp = Param_GetCalibPoints(&len);
    // Setting the same data should return ESP_ERR_INVALID_ARG
    EXPECT_ERR(Param_SetCalibPoints(cp, 6), ESP_ERR_INVALID_ARG);
}

TEST_F(NvsTestFixture, ArrayDoubleReset) {
    EXPECT_ERR(Param_ResetDeviceName(), ESP_FAIL);
}
