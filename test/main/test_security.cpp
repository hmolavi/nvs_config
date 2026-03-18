/**
 * @file test_security.cpp
 * @brief Unit tests for security level enforcement.
 *
 * Security model: level 0 = admin (most privileged), higher = more restricted.
 * At level N, only params with secure_level >= N are writable.
 */

#include "test_helpers.hpp"

void register_security_tests() {} // linker anchor

// ── Level 0 (Admin): full access ──

TEST_F(NvsTestFixture, Level0CanSetLevel0Scalar) {
    EXPECT_OK(Param_SetLetter('Z'));
}

TEST_F(NvsTestFixture, Level0CanSetLevel1Scalar) {
    EXPECT_OK(Param_SetAltitude(100));
}

TEST_F(NvsTestFixture, Level0CanSetLevel2Scalar) {
    EXPECT_OK(Param_SetTempReading(50.0f));
}

TEST_F(NvsTestFixture, Level0CanSetLevel0Array) {
    char buf[16] = {0};
    memcpy(buf, "Admin", 5);
    EXPECT_OK(Param_SetDeviceName(buf, 16));
}

TEST_F(NvsTestFixture, Level0CanSetLevel2Array) {
    const bool flags[8] = {true, true, true, true, true, true, true, true};
    EXPECT_OK(Param_SetFeatureFlags(flags, 8));
}

// ── Level 1 (Operator): level 0 denied ──

struct SecurityLevel1Fixture : NvsTestFixture {
    void SetUp() {
        NvsTestFixture::SetUp();
        NvsConfig_SecureLevelChange(1);
    }
};

TEST_F(SecurityLevel1Fixture, Level1DeniesLevel0Scalar) {
    EXPECT_ERR(Param_SetLetter('Z'), ESP_ERR_INVALID_STATE);
}

TEST_F(SecurityLevel1Fixture, Level1AllowsLevel1Scalar) {
    EXPECT_OK(Param_SetAltitude(200));
}

TEST_F(SecurityLevel1Fixture, Level1AllowsLevel2Scalar) {
    EXPECT_OK(Param_SetTempReading(60.0f));
}

TEST_F(SecurityLevel1Fixture, Level1DeniesLevel0Array) {
    char buf[16] = {0};
    EXPECT_ERR(Param_SetDeviceName(buf, 16), ESP_ERR_INVALID_STATE);
}

TEST_F(SecurityLevel1Fixture, Level1AllowsLevel1Array) {
    const int32_t data[6] = {1, 2, 3, 4, 5, 6};
    EXPECT_OK(Param_SetCalibPoints(data, 6));
}

// ── Level 2 (User): levels 0+1 denied ──

struct SecurityLevel2Fixture : NvsTestFixture {
    void SetUp() {
        NvsTestFixture::SetUp();
        NvsConfig_SecureLevelChange(2);
    }
};

TEST_F(SecurityLevel2Fixture, Level2DeniesLevel0) {
    EXPECT_ERR(Param_SetLetter('Z'), ESP_ERR_INVALID_STATE);
}

TEST_F(SecurityLevel2Fixture, Level2DeniesLevel1) {
    EXPECT_ERR(Param_SetAltitude(300), ESP_ERR_INVALID_STATE);
}

TEST_F(SecurityLevel2Fixture, Level2AllowsLevel2) {
    EXPECT_OK(Param_SetTempReading(70.0f));
}

// ── Level transitions ──

TEST_F(NvsTestFixture, InvalidLevelRejected) {
    EXPECT_ERR(NvsConfig_SecureLevelChange(99), ESP_ERR_INVALID_ARG);
}

TEST_F(NvsTestFixture, LevelTransitionRoundTrip) {
    EXPECT_EQ(NvsConfig_SecureLevel(), (uint8_t)0);

    EXPECT_OK(NvsConfig_SecureLevelChange(1));
    EXPECT_EQ(NvsConfig_SecureLevel(), (uint8_t)1);

    EXPECT_OK(NvsConfig_SecureLevelChange(2));
    EXPECT_EQ(NvsConfig_SecureLevel(), (uint8_t)2);

    EXPECT_OK(NvsConfig_SecureLevelChange(0));
    EXPECT_EQ(NvsConfig_SecureLevel(), (uint8_t)0);
}
