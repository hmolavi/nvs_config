/**
 * @file change_callbacks_main.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Demonstrates parameter change callbacks.
 *
 * The callback system allows you to react to parameter changes in real time,
 * without polling. This example shows:
 *   1. Registering a per-parameter callback (e.g., adjust hardware on change)
 *   2. Registering a global callback (e.g., logging all changes)
 *   3. Passing user_data to callbacks for context
 *   4. Verifying callbacks don't fire when a value doesn't actually change
 */

#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_config.h"

static const char *TAG = "CALLBACK_DEMO";

/* --- Callback functions --- */

/**
 * Global callback: logs every parameter change.
 */
static void on_any_change(const char *param_name, void *user_data)
{
    const char *app_name = (const char *)user_data;
    ESP_LOGW(TAG, "[%s] Parameter '%s' changed!", app_name, param_name);
}

/**
 * Per-parameter callback: "adjusts LED hardware" when Brightness changes.
 */
static void on_brightness_change(const char *param_name, void *user_data)
{
    (void)user_data;
    uint8_t new_brightness = Param_GetBrightness();
    ESP_LOGI(TAG, "LED brightness updated to %u — applying to hardware PWM", new_brightness);
    // In a real app: ledc_set_duty(..., new_brightness); ledc_update_duty(...);
}

/**
 * Per-parameter callback: reacts to temperature target changes.
 */
static void on_temp_target_change(const char *param_name, void *user_data)
{
    (void)user_data;
    float target = Param_GetTempTarget();
    ESP_LOGI(TAG, "Temperature target changed to %.1f C — updating PID controller", target);
}

void app_main(void)
{
    esp_err_t err = NvsConfig_Init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NvsConfig_Init failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=== Parameter Change Callbacks Demo ===");
    ESP_LOGI(TAG, "");

    /* --- Register callbacks --- */

    // Global callback: fires for ANY parameter change
    NvsConfig_RegisterGlobalOnChange(on_any_change, (void *)"MyApp");

    // Per-parameter callbacks: fire only for specific parameters
    NvsConfig_RegisterOnChange("Brightness", on_brightness_change, NULL);
    NvsConfig_RegisterOnChange("TempTarget", on_temp_target_change, NULL);

    ESP_LOGI(TAG, "Callbacks registered. Now modifying parameters...");
    ESP_LOGI(TAG, "");

    /* --- Trigger callbacks --- */

    // Change Brightness: both global + brightness callback should fire
    ESP_LOGI(TAG, ">> Setting Brightness to 200");
    Param_SetBrightness(200);
    ESP_LOGI(TAG, "");

    // Change TempTarget: both global + temp callback should fire
    ESP_LOGI(TAG, ">> Setting TempTarget to 24.5");
    Param_SetTempTarget(24.5f);
    ESP_LOGI(TAG, "");

    // Change LedEnabled: only global callback should fire
    ESP_LOGI(TAG, ">> Setting LedEnabled to false");
    Param_SetLedEnabled(false);
    ESP_LOGI(TAG, "");

    // Set same value again: NO callbacks should fire
    ESP_LOGI(TAG, ">> Setting LedEnabled to false again (same value)");
    esp_err_t rc = Param_SetLedEnabled(false);
    if (rc == ESP_FAIL) {
        ESP_LOGI(TAG, "   Value unchanged — no callbacks fired (as expected)");
    }
    ESP_LOGI(TAG, "");

    /* --- Summary --- */
    ESP_LOGI(TAG, "--- Final parameter state ---");
    NvsConfig_PrintAll();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Change callbacks demo complete.");
}
