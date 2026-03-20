/**
 * @file test_console.cpp
 * @brief Unit tests for registry set() with void* interface across all scalar types.
 *
 * These tests verify that the generic set(void*, size) API correctly handles
 * every supported type, since the console shell will use this interface.
 */

#include "test_helpers.hpp"
#include <cstring>

// ── set() round-trips for each scalar type ──

TEST_F(NvsTestFixture, SetVoidPtrChar) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Letter");
    EXPECT_TRUE(e != nullptr);
    char val = 'Z';
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetLetter(), 'Z');
}

TEST_F(NvsTestFixture, SetVoidPtrBool) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("AdminLock");
    EXPECT_TRUE(e != nullptr);
    bool val = true;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetAdminLock(), true);
}

TEST_F(NvsTestFixture, SetVoidPtrInt8) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("TinyOffset");
    EXPECT_TRUE(e != nullptr);
    int8_t val = 42;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetTinyOffset(), (int8_t)42);
}

TEST_F(NvsTestFixture, SetVoidPtrUint16) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("SampleRate");
    EXPECT_TRUE(e != nullptr);
    uint16_t val = 12345;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetSampleRate(), (uint16_t)12345);
}

TEST_F(NvsTestFixture, SetVoidPtrInt32) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("CalibOffset");
    EXPECT_TRUE(e != nullptr);
    int32_t val = -999999;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetCalibOffset(), (int32_t)-999999);
}

TEST_F(NvsTestFixture, SetVoidPtrUint32) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("SerialNum");
    EXPECT_TRUE(e != nullptr);
    uint32_t val = 123456789U;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetSerialNum(), 123456789U);
}

TEST_F(NvsTestFixture, SetVoidPtrInt64) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("BigTimestamp");
    EXPECT_TRUE(e != nullptr);
    int64_t val = 9999999999LL;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetBigTimestamp(), 9999999999LL);
}

TEST_F(NvsTestFixture, SetVoidPtrFloat) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("TempReading");
    EXPECT_TRUE(e != nullptr);
    float val = 25.5f;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetTempReading(), 25.5f);
}

TEST_F(NvsTestFixture, SetVoidPtrDouble) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("GpsLongitude");
    EXPECT_TRUE(e != nullptr);
    double val = -73.9857;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetGpsLongitude(), -73.9857);
}

// ── Error cases ──

TEST_F(NvsTestFixture, SetVoidPtrNullReturnsError) {
    // Passing size 0 should fail with INVALID_SIZE (scalar expects sizeof(type))
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Brightness");
    EXPECT_TRUE(e != nullptr);
    uint8_t val = 42;
    EXPECT_ERR(e->set(&val, 0), ESP_ERR_INVALID_SIZE);
}
