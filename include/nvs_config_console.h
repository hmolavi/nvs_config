/**
 * @file nvs_config_console.h
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Optional ESP-IDF console commands for interactive parameter management.
 *
 * Call NvsConfig_ConsoleInit() after esp_console_init() and NvsConfig_Init()
 * to register UART shell commands for inspecting and modifying parameters.
 *
 * Enable via Kconfig: CONFIG_NVS_CONFIG_CONSOLE_ENABLED=y
 *
 * @copyright Copyright (c) 2025
 */

#ifndef __NVS_CONFIG_CONSOLE_H__
#define __NVS_CONFIG_CONSOLE_H__

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register NVS config console commands.
 *
 * Registers the following commands:
 *   param list                - List all parameters with values and flags
 *   param get <name>          - Print a parameter's current value
 *   param set <name> <value>  - Set a parameter from a string value
 *   param reset <name>        - Reset a parameter to its default
 *   param reset-all           - Reset all parameters to defaults
 *   param save                - Force-save dirty parameters to NVS
 *   param level [N]           - Get or set the security level
 *
 * @return ESP_OK on success.
 */
esp_err_t NvsConfig_ConsoleInit(void);

#ifdef __cplusplus
}
#endif

#endif // __NVS_CONFIG_CONSOLE_H__
