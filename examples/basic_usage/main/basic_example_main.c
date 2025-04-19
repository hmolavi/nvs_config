/**
 * @file basic_example_main.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Example main.c for basic_usage example
 *
 */

#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_config.h"  // Include component's header

static const char* TAG = "EXAMPLE_MAIN";

void app_main(void)
{
    esp_err_t err = NvsConfig_Init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NvsConfig: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "NvsConfig Initialized.");

    /* --- Example Usage --- */

    // Get initial counter value
    uint8_t counter = Param_GetExampleCounter();
    ESP_LOGI(TAG, "Initial ExampleCounter value: %u", counter);

    // Increment and set counter
    counter++;
    err = Param_SetExampleCounter(counter);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Set ExampleCounter to: %u", counter);
    }
    else if (err != ESP_FAIL) {  // ESP_FAIL just means value didn't change
        ESP_LOGE(TAG, "Failed to set ExampleCounter: %s", esp_err_to_name(err));
    }

    // Optionally save dirty parameters
    // NvsConfig_SaveDirtyParameters(); // Already called periodically by the component timer

    ESP_LOGI(TAG, "Example finished.");
}