/**
 * @file test_wear_level.cpp
 * @brief Unit tests for per-parameter write counting (wear-level tracking).
 */

#include "test_helpers.hpp"

// ── Fixture ──

TEST_GROUP(WearLevelFixture)
{
    void setup() {
        nvs_reset_all_params();
        NvsConfig_ResetWriteCounts();
    }
    void teardown() {}
};

// ── Tests ──

TEST_F(WearLevelFixture, InitialCountIsZero) {
    EXPECT_EQ(NvsConfig_GetWriteCount("Brightness"), 0U);
}

TEST_F(WearLevelFixture, CountIncrementsOnSet) {
    Param_SetBrightness(42);
    EXPECT_EQ(NvsConfig_GetWriteCount("Brightness"), 1U);
}

TEST_F(WearLevelFixture, CountDoesNotIncrementOnSameValue) {
    Param_SetBrightness(42);
    Param_SetBrightness(42); // same value, returns ESP_FAIL
    EXPECT_EQ(NvsConfig_GetWriteCount("Brightness"), 1U);
}

TEST_F(WearLevelFixture, TotalCountSumsAll) {
    Param_SetBrightness(42);
    Param_SetLetter('Z');
    Param_SetTempReading(99.0f);
    EXPECT_EQ(NvsConfig_GetTotalWriteCount(), 3U);
}

TEST_F(WearLevelFixture, RapidWritesCounted) {
    for (int i = 0; i < 100; i++) {
        Param_SetBrightness((uint8_t)(i % 256));
    }
    // i=0 sets 0 (changed from 255), i=1 sets 1 (changed), ... i=99 sets 99
    // All 100 should succeed since each value differs from previous
    EXPECT_EQ(NvsConfig_GetWriteCount("Brightness"), 100U);
}

TEST_F(WearLevelFixture, ArrayWritesCounted) {
    const uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    Param_SetBytePattern(data, 8);
    EXPECT_EQ(NvsConfig_GetWriteCount("BytePattern"), 1U);
}

TEST_F(WearLevelFixture, UnknownParamReturnsZero) {
    EXPECT_EQ(NvsConfig_GetWriteCount("NonExistent"), 0U);
}
