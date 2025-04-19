/**
 * @file nvs_config.h
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Header file for the NVS_Config library.
 *
 * This file declares the public interface for the Non-Volatile Storage (NVS) Configuration library,
 * which manages persistent application parameters in flash storage. The library is responsible for:
 *   - Initializing the NVS flash storage and handling necessary erasures.
 *   - Loading configuration parameters using macros defined in an external parameter table (param_table.inc).
 *   - Marking parameters as “dirty” if their values are modified relative to defaults.
 *   - Periodically saving any modified parameters using a FreeRTOS timer to ensure persistence.
 *   - Managing security levels for accessing or modifying parameters.
 *
 * The library utilizes two macro definitions:
 *   - PARAM: For standard configuration parameters.
 *   - ARRAY: For configuration parameters that are arrays.
 *
 * These macros create structure definitions for storing the parameters, and also generate prototype functions
 * for setting, getting, copying (for arrays), and resetting default values.
 *
 * @note The actual parameter definitions and associated functions are further generated from "param_table.inc",
 *       making it easy to expand or customize the configuration parameters without modifying the core library code.
 *
 *
 * @copyright Copyright (c) 2025
 */

#ifndef __NVS_CONFIG_H__
#define __NVS_CONFIG_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "esp_err.h"

/**
 * @brief Initializes the NVS configuration.
 *
 * This function:
 *  - Initializes the NVS flash and erases it if necessary.
 *  - Opens an NVS handle to load configuration parameters using macros from param_table.inc.
 *    Each parameter is loaded from flash; if unavailable, it is set to its default value and marked as dirty.
 *  - Closes the NVS handle after reading.
 *  - Calls a function to save any dirty parameters.
 *  - Creates and starts a periodic FreeRTOS timer (30 seconds interval) to trigger saving of dirty parameters.
 *
 * @return esp_err_t ESP_OK if initialization and timer creation were successful; otherwise, ESP_FAIL.
 */
esp_err_t NvsConfig_Init(void);

/**
 * @brief Identifies parameters that have been modified (marked as dirty) and saves them to nvs
 */
void NvsConfig_SaveDirtyParameters(void);

/**
 * @brief Get the current security level.
 *
 * @return uint8_t The current security level.
 */
uint8_t NvsConfig_SecureLevel(void);

/**
 * @brief Change the current security level.
 *
 * @param new_secure_level The new security level to set.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t NvsConfig_SecureLevelChange(uint8_t new_secure_level);

/*
 * @brief Non-Volatile Storage (NVS) Configuration Macros and Master Controller.
 *
 * - PARAM:
 *   Creates a structure representing a single configuration parameter.
 *   The structure contains a secure level, parameter name, current value,
 *   default value, status flags (is_dirty and is_default), and a constant key.
 *
 * - ARRAY:
 *   Similar to PARAM, but designed for parameters that are arrays.
 *   In addition to the fields in PARAM, it includes a size field and
 *   appropriately sized arrays for both the current and default values.
 */
#define PARAM(secure_lvl_, type_, name_, default_value_, description_) \
    struct {                                                           \
        const uint8_t secure_level;                                    \
        const char* name;                                              \
        type_ value;                                                   \
        const type_ default_value;                                     \
        bool is_dirty;                                                 \
        bool is_default;                                               \
        const char* const key;                                         \
    } name_;
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_) \
    struct {                                                                  \
        const uint8_t secure_level;                                           \
        const char* name;                                                     \
        type_ value[size_];                                                   \
        const size_t size;                                                    \
        const type_ default_value[size_];                                     \
        bool is_dirty;                                                        \
        bool is_default;                                                      \
        const char* const key;                                                \
    } name_;
typedef struct ParamMasterControl_s {
#include "param_table.inc"
} NvsConfigMasterController_t;
#undef PARAM
#undef ARRAY

/**
 * Global Non-Volatile Storage configuration controller.
 *
 * This variable holds program settings in SRAM. upon calling NvsConfig_Init, it
 * loads settings from NVS Flash; if not present, default values are used.
 */
extern NvsConfigMasterController_t g_nvsconfig_controller;

/*
 * Use macros to generate function declarations for configuration parameters.
 *
 * - The PARAM macro creates functions for single parameters:
 *     • Param_Set<name> to set the value.
 *     • Param_Get<name> to retrieve the value.
 *     • Param_Reset<name> to reset the parameter to its default.
 *
 * - The ARRAY macro creates functions for array parameters:
 *     • Param_Set<name> to set an array of values with its length.
 *     • Param_Get<name> to retrieve the array and its length.
 *     • Param_Copy<name> to copy array contents into a provided buffer.
 *     • Param_Reset<name> to reset the array to its default.
 */
#define PARAM(secure_lvl_, type_, name_, default_value_, description_) \
    esp_err_t Param_Set##name_(const type_ value);                     \
    type_ Param_Get##name_(void);                                      \
    esp_err_t Param_Reset##name_(void);
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_) \
    esp_err_t Param_Set##name_(const type_* value, size_t length);            \
    const type_* Param_Get##name_(size_t* out_array_length);                  \
    esp_err_t Param_Copy##name_(type_* buffer, size_t buffer_size);           \
    esp_err_t Param_Reset##name_(void);
#include "param_table.inc"
#undef PARAM
#undef ARRAY

#endif  // __NVS_CONFIG_H__