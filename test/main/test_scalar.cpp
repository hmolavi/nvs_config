/**
 * @file test_scalar.cpp
 * @brief Unit tests for every scalar parameter type supported by nvs_config.
 */

#include "test_helpers.hpp"

void register_scalar_tests() {} // linker anchor

// ── char ──

TEST_F(NvsTestFixture, CharDefault) {
    EXPECT_EQ(Param_GetLetter(), 'A');
}

TEST_F(NvsTestFixture, CharSetGet) {
    EXPECT_OK(Param_SetLetter('Z'));
    EXPECT_EQ(Param_GetLetter(), 'Z');
}

TEST_F(NvsTestFixture, CharSameValueFails) {
    EXPECT_ERR(Param_SetLetter('A'), ESP_FAIL);
}

TEST_F(NvsTestFixture, CharReset) {
    Param_SetLetter('Z');
    EXPECT_OK(Param_ResetLetter());
    EXPECT_EQ(Param_GetLetter(), 'A');
}

TEST_F(NvsTestFixture, CharDoubleReset) {
    EXPECT_ERR(Param_ResetLetter(), ESP_FAIL);
}

// ── bool ──

TEST_F(NvsTestFixture, BoolDefault) {
    EXPECT_EQ(Param_GetAdminLock(), false);
}

TEST_F(NvsTestFixture, BoolSetGet) {
    EXPECT_OK(Param_SetAdminLock(true));
    EXPECT_EQ(Param_GetAdminLock(), true);
}

TEST_F(NvsTestFixture, BoolReset) {
    Param_SetAdminLock(true);
    EXPECT_OK(Param_ResetAdminLock());
    EXPECT_EQ(Param_GetAdminLock(), false);
}

// ── int8_t ──

TEST_F(NvsTestFixture, Int8Default) {
    EXPECT_EQ(Param_GetTinyOffset(), (int8_t)-127);
}

TEST_F(NvsTestFixture, Int8SetGet) {
    EXPECT_OK(Param_SetTinyOffset(127));
    EXPECT_EQ(Param_GetTinyOffset(), (int8_t)127);
}

TEST_F(NvsTestFixture, Int8Reset) {
    Param_SetTinyOffset(0);
    EXPECT_OK(Param_ResetTinyOffset());
    EXPECT_EQ(Param_GetTinyOffset(), (int8_t)-127);
}

// ── uint8_t ──

TEST_F(NvsTestFixture, Uint8Default) {
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)255);
}

TEST_F(NvsTestFixture, Uint8SetGet) {
    EXPECT_OK(Param_SetBrightness(0));
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)0);
}

TEST_F(NvsTestFixture, Uint8Reset) {
    Param_SetBrightness(0);
    EXPECT_OK(Param_ResetBrightness());
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)255);
}

// ── int16_t ──

TEST_F(NvsTestFixture, Int16Default) {
    EXPECT_EQ(Param_GetAltitude(), (int16_t)-32000);
}

TEST_F(NvsTestFixture, Int16SetGet) {
    EXPECT_OK(Param_SetAltitude(32767));
    EXPECT_EQ(Param_GetAltitude(), (int16_t)32767);
}

TEST_F(NvsTestFixture, Int16Reset) {
    Param_SetAltitude(0);
    EXPECT_OK(Param_ResetAltitude());
    EXPECT_EQ(Param_GetAltitude(), (int16_t)-32000);
}

// ── uint16_t ──

TEST_F(NvsTestFixture, Uint16Default) {
    EXPECT_EQ(Param_GetSampleRate(), (uint16_t)60000);
}

TEST_F(NvsTestFixture, Uint16SetGet) {
    EXPECT_OK(Param_SetSampleRate(0));
    EXPECT_EQ(Param_GetSampleRate(), (uint16_t)0);
}

TEST_F(NvsTestFixture, Uint16Reset) {
    Param_SetSampleRate(0);
    EXPECT_OK(Param_ResetSampleRate());
    EXPECT_EQ(Param_GetSampleRate(), (uint16_t)60000);
}

// ── int32_t ──

TEST_F(NvsTestFixture, Int32Default) {
    EXPECT_EQ(Param_GetCalibOffset(), (int32_t)-2000000000);
}

TEST_F(NvsTestFixture, Int32SetGetMax) {
    EXPECT_OK(Param_SetCalibOffset(2147483647));
    EXPECT_EQ(Param_GetCalibOffset(), (int32_t)2147483647);
}

TEST_F(NvsTestFixture, Int32Reset) {
    Param_SetCalibOffset(0);
    EXPECT_OK(Param_ResetCalibOffset());
    EXPECT_EQ(Param_GetCalibOffset(), (int32_t)-2000000000);
}

// ── uint32_t ──

TEST_F(NvsTestFixture, Uint32Default) {
    EXPECT_EQ(Param_GetSerialNum(), 4000000000U);
}

TEST_F(NvsTestFixture, Uint32SetGet) {
    EXPECT_OK(Param_SetSerialNum(0U));
    EXPECT_EQ(Param_GetSerialNum(), 0U);
}

TEST_F(NvsTestFixture, Uint32Reset) {
    Param_SetSerialNum(0U);
    EXPECT_OK(Param_ResetSerialNum());
    EXPECT_EQ(Param_GetSerialNum(), 4000000000U);
}

// ── int64_t ──

TEST_F(NvsTestFixture, Int64Default) {
    EXPECT_EQ(Param_GetBigTimestamp(), 1700000000LL);
}

TEST_F(NvsTestFixture, Int64SetGet) {
    EXPECT_OK(Param_SetBigTimestamp(9999999999LL));
    EXPECT_EQ(Param_GetBigTimestamp(), 9999999999LL);
}

TEST_F(NvsTestFixture, Int64Reset) {
    Param_SetBigTimestamp(0LL);
    EXPECT_OK(Param_ResetBigTimestamp());
    EXPECT_EQ(Param_GetBigTimestamp(), 1700000000LL);
}

// ── uint64_t ──

TEST_F(NvsTestFixture, Uint64Default) {
    EXPECT_EQ(Param_GetDeviceUID(), 0ULL);
}

TEST_F(NvsTestFixture, Uint64SetGetMax) {
    EXPECT_OK(Param_SetDeviceUID(0xFFFFFFFFFFFFFFFFULL));
    EXPECT_EQ(Param_GetDeviceUID(), 0xFFFFFFFFFFFFFFFFULL);
}

TEST_F(NvsTestFixture, Uint64SameValueFails) {
    EXPECT_ERR(Param_SetDeviceUID(0ULL), ESP_FAIL);
}

// ── float ──

TEST_F(NvsTestFixture, FloatDefault) {
    EXPECT_EQ(Param_GetTempReading(), -40.5f);
}

TEST_F(NvsTestFixture, FloatSetGet) {
    EXPECT_OK(Param_SetTempReading(99.9f));
    EXPECT_EQ(Param_GetTempReading(), 99.9f);
}

TEST_F(NvsTestFixture, FloatReset) {
    Param_SetTempReading(0.0f);
    EXPECT_OK(Param_ResetTempReading());
    EXPECT_EQ(Param_GetTempReading(), -40.5f);
}

// ── double ──

TEST_F(NvsTestFixture, DoubleDefault) {
    EXPECT_EQ(Param_GetGpsLongitude(), -122.419418);
}

TEST_F(NvsTestFixture, DoubleSetGet) {
    EXPECT_OK(Param_SetGpsLongitude(179.999999));
    EXPECT_EQ(Param_GetGpsLongitude(), 179.999999);
}

TEST_F(NvsTestFixture, DoubleReset) {
    Param_SetGpsLongitude(0.0);
    EXPECT_OK(Param_ResetGpsLongitude());
    EXPECT_EQ(Param_GetGpsLongitude(), -122.419418);
}

// ── 15-character parameter name (NVS key limit) ──

TEST_F(NvsTestFixture, MaxLengthNameDefault) {
    EXPECT_EQ(Param_GetMaxNameTestXXX1(), 42U);
}

TEST_F(NvsTestFixture, MaxLengthNameSetGet) {
    EXPECT_OK(Param_SetMaxNameTestXXX1(0xDEADBEEFU));
    EXPECT_EQ(Param_GetMaxNameTestXXX1(), 0xDEADBEEFU);
}
