/**
 * @file test_versioning.cpp
 * @brief Unit tests for NVS schema versioning.
 */

#include "test_helpers.hpp"

// ── Tests ──

TEST_F(NvsTestFixture, SchemaVersionReturnsCurrentVersion) {
    // After NvsConfig_Init(), schema version should be the compiled-in value
    EXPECT_EQ(NvsConfig_GetSchemaVersion(), (uint32_t)NVS_CONFIG_SCHEMA_VERSION);
}

TEST_F(NvsTestFixture, SchemaVersionIsNonZero) {
    EXPECT_GT(NvsConfig_GetSchemaVersion(), 0U);
}
