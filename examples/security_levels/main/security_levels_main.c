/**
 * @file security_levels_main.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Demonstrates role-based parameter access control.
 *
 * Parameters are assigned a security level in param_table.inc. At runtime,
 * a global security level controls which parameters can be written. A higher
 * numeric level means more restriction — at level N, only parameters with
 * secure_level >= N are writable.
 *
 * This example shows:
 *   1. Admin (level 0) can access everything
 *   2. Technician (level 1) is blocked from admin params
 *   3. End user (level 2) is blocked from admin and technician params
 *   4. Security level transitions
 */

#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_config.h"

static const char *TAG = "SECURITY_DEMO";

static void try_set_serial(uint32_t val)
{
    esp_err_t rc = Param_SetSerialNumber(val);
    ESP_LOGI(TAG, "  Set SerialNumber=%lu: %s",
             (unsigned long)val, esp_err_to_name(rc));
}

static void try_set_calib(int16_t val)
{
    esp_err_t rc = Param_SetCalibOffset(val);
    ESP_LOGI(TAG, "  Set CalibOffset=%d: %s", val, esp_err_to_name(rc));
}

static void try_set_brightness(uint8_t val)
{
    esp_err_t rc = Param_SetBrightness(val);
    ESP_LOGI(TAG, "  Set Brightness=%u: %s", val, esp_err_to_name(rc));
}

void app_main(void)
{
    esp_err_t err = NvsConfig_Init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NvsConfig_Init failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=== Security Levels Demo ===");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Security model: level 0 = Admin, 1 = Technician, 2 = End User");
    ESP_LOGI(TAG, "At level N, only params with secure_level >= N are writable.");
    ESP_LOGI(TAG, "");

    /* --- Level 0: Admin (full access) --- */
    NvsConfig_SecureLevelChange(0);
    ESP_LOGI(TAG, "--- Level 0 (Admin): full access ---");
    try_set_serial(12345);       // level 0 param -> should succeed
    try_set_calib(42);           // level 1 param -> should succeed
    try_set_brightness(200);     // level 2 param -> should succeed
    ESP_LOGI(TAG, "");

    /* --- Level 1: Technician (admin params blocked) --- */
    NvsConfig_SecureLevelChange(1);
    ESP_LOGI(TAG, "--- Level 1 (Technician): admin params blocked ---");
    try_set_serial(99999);       // level 0 param -> should be DENIED
    try_set_calib(-10);          // level 1 param -> should succeed
    try_set_brightness(100);     // level 2 param -> should succeed
    ESP_LOGI(TAG, "");

    /* --- Level 2: End User (admin + tech params blocked) --- */
    NvsConfig_SecureLevelChange(2);
    ESP_LOGI(TAG, "--- Level 2 (End User): admin + technician params blocked ---");
    try_set_serial(99999);       // level 0 param -> should be DENIED
    try_set_calib(100);          // level 1 param -> should be DENIED
    try_set_brightness(50);      // level 2 param -> should succeed
    ESP_LOGI(TAG, "");

    /* --- Invalid level --- */
    ESP_LOGI(TAG, "--- Attempting invalid security level ---");
    err = NvsConfig_SecureLevelChange(99);
    ESP_LOGI(TAG, "  Set level 99: %s (expected INVALID_ARG)", esp_err_to_name(err));
    ESP_LOGI(TAG, "");

    /* --- Final state --- */
    NvsConfig_SecureLevelChange(0);
    ESP_LOGI(TAG, "--- Final parameter values ---");
    NvsConfig_PrintAll();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Security levels demo complete.");
}
