/**
 * @file console_demo_main.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Interactive UART console for managing NVS parameters.
 *
 * This example starts an ESP-IDF console (REPL) over UART, registers the
 * nvs_config console commands, and lets you inspect and modify parameters
 * interactively from a serial terminal.
 *
 * Available commands (type 'help' for full list):
 *   param-list              List all parameters with values and flags
 *   param-get <name>        Print a single parameter's value
 *   param-set <name> <val>  Set a scalar parameter
 *   param-reset <name|all>  Reset one or all parameters to defaults
 *   param-save              Force-save dirty parameters to NVS flash
 *   param-level [N]         Get or set the security level
 *
 * Build with:  idf.py set-target <your-esp32-model> && idf.py build flash monitor
 *
 * Requires CONFIG_NVS_CONFIG_CONSOLE_ENABLED=y (set in sdkconfig.defaults).
 */

#include <stdio.h>

#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"

#include "nvs_config.h"
#include "nvs_config_console.h"

static const char *TAG = "CONSOLE_DEMO";

void app_main(void)
{
    /* Initialize NVS configuration */
    esp_err_t err = NvsConfig_Init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NvsConfig_Init failed: %s", esp_err_to_name(err));
        return;
    }

    /* Elevate to admin so all parameters are accessible */
    NvsConfig_SecureLevelChange(0);

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  NVS Config - Interactive Console Demo");
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "");

    /* Print all parameters on startup */
    NvsConfig_PrintAll();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Type 'help' for available commands.");
    ESP_LOGI(TAG, "");

    /* Initialize the console REPL */
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "nvs> ";
    repl_config.max_cmdline_length = 256;

    /* Register nvs_config console commands */
    NvsConfig_ConsoleInit();

    /* Start UART-based REPL */
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
