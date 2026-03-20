/**
 * @file test_init_and_save.cpp
 * @brief Tests for NvsConfig_Init() error paths, NvsConfig_SaveDirtyParameters(),
 *        schema versioning migration, and callback registration limits.
 *
 * These tests exercise branches that the other test files cannot reach because
 * they require specific NVS/timer/mutex failure modes or a second Init() call.
 * Mock control knobs from mock_control.h are used and reset in teardown().
 */

#include "test_helpers.hpp"
#include "mock_control.h"
#include "nvs.h"
#include <cstring>
#include <cstdint>

// ── Fixture ──────────────────────────────────────────────────────────────────

TEST_GROUP(InitAndSaveFixture)
{
    void setup()
    {
        mock_reset_controls();
        nvs_reset_all_params();
    }
    void teardown()
    {
        // Clear migration callback and restore mock defaults
        NvsConfig_RegisterMigration(nullptr);
        NvsConfig_ClearCallbacks();
        mock_reset_controls();
    }
};

// ── NvsConfig_RegisterMigration ───────────────────────────────────────────────

TEST(InitAndSaveFixture, RegisterMigrationReturnsOk)
{
    auto cb = [](uint32_t, uint32_t) -> esp_err_t { return ESP_OK; };
    EXPECT_OK(NvsConfig_RegisterMigration(cb));
}

// ── NvsConfig_SaveDirtyParameters ─────────────────────────────────────────────

/** All 19 parameters are dirty after reset; SaveDirty should commit them. */
TEST(InitAndSaveFixture, SaveDirtyParametersSuccess)
{
    // Mark something dirty so parametersChanged > 0
    Param_SetBrightness(100);
    NvsConfig_SaveDirtyParameters();  // should not crash, commit succeeds
}

/** nvs_open failure: SaveDirty logs error and returns without crashing. */
TEST(InitAndSaveFixture, SaveDirtyParametersNvsOpenFails)
{
    Param_SetBrightness(100);
    g_mock_nvs_open_ret = ESP_FAIL;
    NvsConfig_SaveDirtyParameters();  // exercises the early-return error path
}

/** nvs_set_blob failure: error is logged, parametersChanged stays 0, no commit. */
TEST(InitAndSaveFixture, SaveDirtyParametersNvsSetBlobFails)
{
    Param_SetBrightness(100);
    g_mock_nvs_set_blob_ret = ESP_FAIL;
    NvsConfig_SaveDirtyParameters();
}

/** nvs_commit failure: commit is attempted but fails; error is logged. */
TEST(InitAndSaveFixture, SaveDirtyParametersNvsCommitFails)
{
    Param_SetBrightness(100);
    g_mock_nvs_commit_ret = ESP_FAIL;
    NvsConfig_SaveDirtyParameters();
}

// ── NvsConfig_Init error paths ────────────────────────────────────────────────

/**
 * Second Init() call: mutex is already non-NULL so xSemaphoreCreateMutex is
 * skipped.  The rest of Init (flash, NVS open, timer) runs again.
 */
TEST(InitAndSaveFixture, InitAlreadyInitialized)
{
    EXPECT_OK(NvsConfig_Init());  // second call; mutex already exists
}

/**
 * nvs_flash_init returns ESP_ERR_NVS_NO_FREE_PAGES → Init erases flash and
 * calls flash_init a second time (both covered by the one-shot mock).
 */
TEST(InitAndSaveFixture, InitNvsFlashInitNoFreePages)
{
    g_mock_nvs_flash_init_ret = ESP_ERR_NVS_NO_FREE_PAGES;
    EXPECT_OK(NvsConfig_Init());  // erase path exercised; second flash_init → OK
}

/** esp_timer_create failure → Init returns ESP_FAIL. */
TEST(InitAndSaveFixture, InitTimerCreateFails)
{
    g_mock_esp_timer_create_ret = ESP_FAIL;
    EXPECT_ERR(NvsConfig_Init(), ESP_FAIL);
}

/** esp_timer_start_periodic failure → Init returns the error code. */
TEST(InitAndSaveFixture, InitTimerStartFails)
{
    g_mock_esp_timer_start_ret = ESP_FAIL;
    EXPECT_ERR(NvsConfig_Init(), ESP_FAIL);
}

// ── NVS load-from-flash paths in Init ────────────────────────────────────────

/**
 * Make nvs_get_blob return ESP_OK for every call (schema key + all parameters).
 * The blob data is all-zeros, which means scalar parameters read as 0 and
 * arrays read as zero-filled — exercising the "load from NVS" else-branch
 * (is_dirty = false) and the is_default comparisons for both matching and
 * non-matching defaults.
 *
 * Schema key (first call): stored version = 0 → data is 0 = version mismatch
 * — but we want the plain "load success" path here, so use the current version.
 */
TEST(InitAndSaveFixture, InitLoadsParametersFromNvs)
{
    // Set mock data to the current schema version so no mismatch occurs
    uint32_t current = NVS_CONFIG_SCHEMA_VERSION;
    memcpy(g_mock_nvs_get_blob_data, &current, sizeof(current));
    // 1 (schema) + 13 scalars + 6 arrays = 20 successful reads
    g_mock_nvs_get_blob_ok_calls = 20;
    EXPECT_OK(NvsConfig_Init());
}

// ── Schema version mismatch paths ────────────────────────────────────────────

/**
 * Stored version differs and no migration callback is registered →
 * NvsConfig_Init calls nvs_erase_all + nvs_commit to reset storage.
 */
TEST(InitAndSaveFixture, SchemaMismatchNoMigrationCallback)
{
    uint32_t old_version = NVS_CONFIG_SCHEMA_VERSION + 1;
    memcpy(g_mock_nvs_get_blob_data, &old_version, sizeof(old_version));
    g_mock_nvs_get_blob_ok_calls = 1;  // only schema key succeeds
    // No migration callback registered
    EXPECT_OK(NvsConfig_Init());
}

/**
 * Stored version differs, migration callback returns ESP_OK →
 * parameters are NOT erased; Init continues normally.
 */
TEST(InitAndSaveFixture, SchemaMismatchMigrationSucceeds)
{
    uint32_t old_version = NVS_CONFIG_SCHEMA_VERSION + 1;
    memcpy(g_mock_nvs_get_blob_data, &old_version, sizeof(old_version));
    g_mock_nvs_get_blob_ok_calls = 1;

    auto ok_cb = [](uint32_t, uint32_t) -> esp_err_t { return ESP_OK; };
    NvsConfig_RegisterMigration(ok_cb);
    EXPECT_OK(NvsConfig_Init());
}

/**
 * Stored version differs, migration callback returns failure →
 * Init falls back to erasing NVS (nvs_erase_all + nvs_commit).
 */
TEST(InitAndSaveFixture, SchemaMismatchMigrationFails)
{
    uint32_t old_version = NVS_CONFIG_SCHEMA_VERSION + 1;
    memcpy(g_mock_nvs_get_blob_data, &old_version, sizeof(old_version));
    g_mock_nvs_get_blob_ok_calls = 1;

    auto fail_cb = [](uint32_t, uint32_t) -> esp_err_t { return ESP_FAIL; };
    NvsConfig_RegisterMigration(fail_cb);
    EXPECT_OK(NvsConfig_Init());
}

// ── Callback overflow ─────────────────────────────────────────────────────────

/** Registering more than NVS_CONFIG_MAX_CALLBACKS callbacks returns NO_MEM. */
TEST(InitAndSaveFixture, CallbackRegistrationOverflow)
{
    auto dummy_cb = [](const char*, void*) {};
    // Fill up the callback table
    for (int i = 0; i < 16; i++) {
        EXPECT_OK(NvsConfig_RegisterOnChange(nullptr, dummy_cb, nullptr));
    }
    // One more should fail
    EXPECT_ERR(NvsConfig_RegisterOnChange(nullptr, dummy_cb, nullptr), ESP_ERR_NO_MEM);
}
