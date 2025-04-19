/**
 * @file array_example_main.c
 * @author Hossein Molavi
 *
 * @brief Example main.c for array_example
 *
 * Overview:
 * This example demonstrates how to initialize and use the non-volatile storage (NVS)
 * configuration for handling an integer array parameter. The example covers:
 *   1. Initializing the NVS configuration.
 *   2. Retrieving and printing the default array parameter.
 *   3. Updating the array parameter with a new set of values.
 *   4. Copying the current array value and verifying the copy.
 *   5. Resetting the array parameter to its default value.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_config.h"  // Include component's header

static const char* TAG = "ARRAY_EXAMPLE";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing NVS Config...");
    esp_err_t err = NvsConfig_Init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NvsConfig: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "NvsConfig Initialized.");

    /* --- Array Example Usage --- */
    size_t array_len;
    char print_buf[128];

    // 1. Get the default/initial value
    const int32_t* current_array = Param_GetExampleIntArray(&array_len);
    Param_PrintExampleIntArray(print_buf, sizeof(print_buf));
    ESP_LOGI(TAG, "Initial array (length %d): %s", array_len, print_buf);

    // 2. Define a new array and set the parameter
    int32_t new_array_data[4] = {99, 88, 77, 66};  // Must match size defined in param_table.inc
    ESP_LOGI(TAG, "Attempting to set array to:");
    // (The new array is not printed here since the macro prints the stored ExampleIntArray.)
    err = Param_SetExampleIntArray(new_array_data, sizeof(new_array_data) / sizeof(int32_t));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully set ExampleIntArray to some other values.");
        current_array = Param_GetExampleIntArray(&array_len);  // Get updated value
        Param_PrintExampleIntArray(print_buf, sizeof(print_buf));
        ESP_LOGI(TAG, "Updated array value: %s", print_buf);
    }
    else {
        ESP_LOGE(TAG, "Failed to set ExampleIntArray: %s", esp_err_to_name(err));
    }

    // 3. Make a copy of the current array value
    int32_t* copied_array = (int32_t*)malloc(array_len * sizeof(int32_t));
    if (copied_array == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for copy!");
    }
    else {
        ESP_LOGI(TAG, "Attempting to copy the Updated array value...");
        err = Param_CopyExampleIntArray(copied_array, array_len * sizeof(int32_t));
        if (err == ESP_OK) {
            // Since the macro prints the parameter's stored array, we use it here.
            Param_PrintExampleIntArray(print_buf, sizeof(print_buf));
            ESP_LOGI(TAG, "Successfully copied array: %s", print_buf);
            // Verify copy using memcmp on the original and copied array data.
            if (memcmp(current_array, copied_array, array_len * sizeof(int32_t)) == 0) {
                ESP_LOGI(TAG, "Copy verified successfully.");
            }
            else {
                ESP_LOGW(TAG, "Copy verification FAILED!");
            }
        }
        else {
            ESP_LOGE(TAG, "Failed to copy ExampleIntArray: %s", esp_err_to_name(err));
        }
        free(copied_array);
    }

    // 4. Reset the array parameter to its default value
    ESP_LOGI(TAG, "Attempting to reset ExampleIntArray to default...");
    err = Param_ResetExampleIntArray();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully reset ExampleIntArray.");
        current_array = Param_GetExampleIntArray(&array_len);  // Get reset value
        Param_PrintExampleIntArray(print_buf, sizeof(print_buf));
        ESP_LOGI(TAG, "Current array value after reset: %s", print_buf);
    }
    else if (err == ESP_FAIL) {  // ESP_FAIL means the value was already default
        ESP_LOGI(TAG, "ExampleIntArray was already at its default value.");
    }
    else {
        ESP_LOGE(TAG, "Failed to reset ExampleIntArray: %s", esp_err_to_name(err));
    }

    // Note: Changes are saved periodically by the timer in NvsConfig_Init
    // NvsConfig_SaveDirtyParameters(); // or force a save

    ESP_LOGI(TAG, "Array example finished.");
}