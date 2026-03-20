/**
 * @file test_main.cpp
 * @brief Entry point for the nvs_config hardware (on-device) test suite.
 *
 * Runs only the tests that require a real FreeRTOS scheduler and hardware.
 * Build and flash with:
 *   idf.py set-target esp32s3
 *   idf.py build flash monitor
 */

#include "test_framework.hpp"
#include "nvs_config.h"
#include "esp_log.h"

extern void register_thread_safety_tests();

static const char* TAG = "TEST";

extern "C" void app_main(void)
{
    register_thread_safety_tests();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "NVS Config - Hardware Test Suite (4 tests)");
    ESP_LOGI(TAG, "");

    esp_err_t rc = NvsConfig_Init();
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "NvsConfig_Init failed (0x%x) - aborting", rc);
        return;
    }

    NvsConfig_SecureLevelChange(0);

    nvs_test::TestRunner::get().run_all();
}
