/**
 * @file registry_iterator_main.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Demonstrates the parameter registry for generic, type-agnostic operations.
 *
 * The registry (g_nvsconfig_params[]) provides a vtable for every parameter,
 * enabling code that works on all parameters without knowing their concrete
 * types at compile time.
 *
 * This example shows:
 *   1. Iterating all parameters and printing metadata + values
 *   2. Looking up a parameter by name
 *   3. Setting a value via the generic set(void*, size) interface
 *   4. Resetting all parameters to defaults in one call
 *   5. Checking dirty/default flags through the vtable
 */

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_config.h"

static const char *TAG = "REGISTRY_DEMO";

void app_main(void)
{
    esp_err_t err = NvsConfig_Init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NvsConfig_Init failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=== Parameter Registry Demo ===");
    ESP_LOGI(TAG, "");

    /* --- 1. Iterate all parameters --- */
    ESP_LOGI(TAG, "--- All %d registered parameters ---", (int)g_nvsconfig_param_count);

    char buf[128];
    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        const NvsConfigParamEntry_t *p = &g_nvsconfig_params[i];

        p->print(buf, sizeof(buf));
        ESP_LOGI(TAG, "[%d] %-12s = %-20s  type=%s  elem_size=%d  count=%d  desc=\"%s\"",
                 (int)i,
                 p->name,
                 buf,
                 p->is_array ? "array" : "scalar",
                 (int)p->element_size,
                 (int)p->element_count,
                 p->description);
    }

    /* --- 2. Find by name --- */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Find by name ---");

    const NvsConfigParamEntry_t *vol = NvsConfig_FindParam("Volume");
    if (vol) {
        vol->print(buf, sizeof(buf));
        ESP_LOGI(TAG, "Found 'Volume': value=%s, is_default=%s, is_dirty=%s",
                 buf,
                 vol->is_default() ? "yes" : "no",
                 vol->is_dirty() ? "yes" : "no");
    }

    const NvsConfigParamEntry_t *missing = NvsConfig_FindParam("NonExistent");
    ESP_LOGI(TAG, "Lookup 'NonExistent': %s", missing ? "found" : "not found (as expected)");

    /* --- 3. Generic set via void pointer --- */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Generic set() via void* ---");

    // Set a scalar: Volume = 90
    uint8_t new_volume = 90;
    err = vol->set(&new_volume, sizeof(new_volume));
    ESP_LOGI(TAG, "Set Volume to 90: %s", esp_err_to_name(err));
    vol->print(buf, sizeof(buf));
    ESP_LOGI(TAG, "Volume is now: %s", buf);

    // Set an array: RGBColor = {0, 255, 128}
    const NvsConfigParamEntry_t *rgb = NvsConfig_FindParam("RGBColor");
    if (rgb) {
        uint8_t new_color[3] = {0, 255, 128};
        err = rgb->set(new_color, sizeof(new_color));
        ESP_LOGI(TAG, "Set RGBColor to {0, 255, 128}: %s", esp_err_to_name(err));
        rgb->print(buf, sizeof(buf));
        ESP_LOGI(TAG, "RGBColor is now: %s", buf);
    }

    /* --- 4. Check flags --- */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Dirty/default flags after changes ---");
    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        const NvsConfigParamEntry_t *p = &g_nvsconfig_params[i];
        ESP_LOGI(TAG, "  %-12s  dirty=%s  default=%s",
                 p->name,
                 p->is_dirty() ? "yes" : "no",
                 p->is_default() ? "yes" : "no");
    }

    /* --- 5. Reset all --- */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- ResetAll ---");
    NvsConfig_ResetAll();
    NvsConfig_PrintAll();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Registry iterator demo complete.");
}
