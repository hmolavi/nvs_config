/**
 * @file test_main.cpp
 * @brief Entry point for the nvs_config C++ unit test suite.
 */

#include "test_framework.hpp"
#include "nvs_config.h"
#include "esp_log.h"

// Force linker to include test translation units.
// Each test file defines one of these; calling them here prevents the
// linker from stripping the files (and their static Registrar objects).
extern void register_scalar_tests();
extern void register_array_tests();
extern void register_security_tests();
extern void register_print_tests();
extern void register_edge_case_tests();
extern void register_registry_tests();
extern void register_thread_safety_tests();
extern void register_console_tests();
extern void register_callback_tests();
extern void register_wear_level_tests();
extern void register_versioning_tests();

static const char* TAG = "TEST";

extern "C" void app_main(void)
{
    // Pull in all test files
    register_scalar_tests();
    register_array_tests();
    register_security_tests();
    register_print_tests();
    register_edge_case_tests();
    register_registry_tests();
    register_thread_safety_tests();
    register_console_tests();
    register_callback_tests();
    register_wear_level_tests();
    register_versioning_tests();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "NVS Config - C++ Unit Test Suite");
    ESP_LOGI(TAG, "");

    esp_err_t rc = NvsConfig_Init();
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "NvsConfig_Init failed (0x%x) - aborting", rc);
        return;
    }

    // Security level starts at highest (most restricted) by design;
    // elevate to admin so test fixtures can reset all params.
    NvsConfig_SecureLevelChange(0);

    nvs_test::TestRunner::get().run_all();
}
