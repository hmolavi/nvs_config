/**
 * @file test_print.cpp
 * @brief Unit tests for Param_Print* formatting functions.
 */

#include "test_helpers.hpp"
#include <cstring>

// ── Scalar print tests ──

TEST_F(NvsTestFixture, PrintChar) {
    char buf[32];
    int n = Param_PrintLetter(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "A");
}

TEST_F(NvsTestFixture, PrintBool) {
    char buf[32];
    int n = Param_PrintAdminLock(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "0");  // false = 0
}

TEST_F(NvsTestFixture, PrintInt8) {
    char buf[32];
    int n = Param_PrintTinyOffset(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "-127");
}

TEST_F(NvsTestFixture, PrintUint8) {
    char buf[32];
    int n = Param_PrintBrightness(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "255");
}

TEST_F(NvsTestFixture, PrintInt64) {
    char buf[32];
    int n = Param_PrintBigTimestamp(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "1700000000");
}

TEST_F(NvsTestFixture, PrintUint64) {
    char buf[32];
    int n = Param_PrintDeviceUID(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "0");
}

TEST_F(NvsTestFixture, PrintInt16) {
    char buf[32];
    int n = Param_PrintAltitude(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "-32000");
}

TEST_F(NvsTestFixture, PrintUint16) {
    char buf[32];
    int n = Param_PrintSampleRate(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "60000");
}

TEST_F(NvsTestFixture, PrintInt32) {
    char buf[32];
    int n = Param_PrintCalibOffset(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "-2000000000");
}

TEST_F(NvsTestFixture, PrintUint32) {
    char buf[32];
    int n = Param_PrintSerialNum(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "4000000000");
}

TEST_F(NvsTestFixture, PrintFloat) {
    char buf[32];
    int n = Param_PrintTempReading(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "-40.5");
}

TEST_F(NvsTestFixture, PrintDouble) {
    char buf[32];
    int n = Param_PrintGpsLongitude(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "-122.419");
}

// ── Array print tests ──

TEST_F(NvsTestFixture, PrintInt32Array) {
    char buf[64];
    int n = Param_PrintCalibPoints(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "[-1000,-500,0,500,1000,2000]");
}

TEST_F(NvsTestFixture, PrintBoolArray) {
    char buf[32];
    int n = Param_PrintFeatureFlags(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "[1,0,1,0,1,0,1,0]");
}

TEST_F(NvsTestFixture, PrintUint16Array) {
    char buf[32];
    int n = Param_PrintRGBColor(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "[255,128,0]");
}

// ── Truncation safety ──

TEST_F(NvsTestFixture, PrintTruncation) {
    char buf[10];
    int n = Param_PrintCalibPoints(buf, sizeof(buf));
    // Should not crash, and n indicates how many chars would be needed
    EXPECT_GE(n, 0);
    // Buffer should be null-terminated
    EXPECT_TRUE(strlen(buf) < sizeof(buf));
}

TEST_F(NvsTestFixture, PrintMinimalBuffer) {
    char buf[2];
    int n = Param_PrintLetter(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_EQ(buf[0], 'A');
}
