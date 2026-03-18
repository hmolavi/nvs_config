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

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * @brief Parameter registry entry with function pointers for runtime introspection.
 *
 * Each parameter in param_table.inc gets one entry in the global registry array.
 * This enables generic iteration, lookup-by-name, and polymorphic operations
 * without knowing the parameter's concrete type at the call site.
 */
typedef struct {
    const char* name;
    const char* description;
    uint8_t secure_level;
    bool is_array;
    size_t element_size;    /**< sizeof(type) for one element */
    size_t element_count;   /**< 1 for scalars, array size for arrays */
    bool (*is_dirty)(void);
    bool (*is_default)(void);
    esp_err_t (*reset)(void);
    int (*print)(char* buf, size_t buf_size);
    /**
     * @brief Set a parameter value from a raw pointer.
     *
     * For scalars: data points to a single value of the parameter's type,
     *              data_size must equal element_size.
     * For arrays:  data points to an array of elements. If data_size < full
     *              array size, remaining elements are zeroed and ESP_ERR_INVALID_SIZE
     *              is returned as a warning (the write still occurs).
     *              If data_size > full array size, ESP_ERR_INVALID_SIZE is returned
     *              and no write occurs.
     *
     * @param data      Pointer to the value(s) to write.
     * @param data_size Total size in bytes of the data pointed to.
     * @return ESP_OK on success, ESP_ERR_INVALID_SIZE on partial write (warning)
     *         or oversize, ESP_ERR_INVALID_STATE if security level insufficient.
     */
    esp_err_t (*set)(const void* data, size_t data_size);
} NvsConfigParamEntry_t;

/** Array of registry entries, one per parameter (order matches param_table.inc). */
extern const NvsConfigParamEntry_t g_nvsconfig_params[];

/** Number of entries in g_nvsconfig_params. */
extern const size_t g_nvsconfig_param_count;

/**
 * @brief Find a parameter registry entry by name.
 * @param name The parameter name (case-sensitive).
 * @return Pointer to the entry, or NULL if not found.
 */
const NvsConfigParamEntry_t* NvsConfig_FindParam(const char* name);

/**
 * @brief Reset all parameters to their default values.
 */
void NvsConfig_ResetAll(void);

/**
 * @brief Print all parameters (name = value) to the log.
 */
void NvsConfig_PrintAll(void);

/**
 * @brief Callback type for parameter change notifications.
 * @param param_name Name of the changed parameter.
 * @param user_data User-supplied context pointer.
 */
typedef void (*NvsConfigOnChange_t)(const char* param_name, void* user_data);

/**
 * @brief Register a callback that fires when a specific parameter changes.
 * @param param_name Parameter name to watch (case-sensitive).
 * @param cb Callback function.
 * @param user_data Passed to callback on invocation.
 * @return ESP_OK on success, ESP_ERR_NO_MEM if callback slots full.
 */
esp_err_t NvsConfig_RegisterOnChange(const char* param_name,
                                     NvsConfigOnChange_t cb,
                                     void* user_data);

/**
 * @brief Register a callback that fires when any parameter changes.
 * @param cb Callback function.
 * @param user_data Passed to callback on invocation.
 * @return ESP_OK on success, ESP_ERR_NO_MEM if callback slots full.
 */
esp_err_t NvsConfig_RegisterGlobalOnChange(NvsConfigOnChange_t cb,
                                           void* user_data);

/**
 * @brief Remove all registered callbacks (useful for testing).
 */
void NvsConfig_ClearCallbacks(void);

/**
 * @brief Get the number of successful writes to a parameter since init.
 * @param name Parameter name (case-sensitive).
 * @return Write count, or 0 if parameter not found.
 */
uint32_t NvsConfig_GetWriteCount(const char* name);

/**
 * @brief Get the total number of successful writes across all parameters.
 * @return Sum of all per-parameter write counts.
 */
uint32_t NvsConfig_GetTotalWriteCount(void);

/**
 * @brief Reset all write counters to zero (useful for testing).
 */
void NvsConfig_ResetWriteCounts(void);

/**
 * @brief Schema version for detecting param_table changes across firmware updates.
 *
 * Users should define NVS_CONFIG_SCHEMA_VERSION before including this header,
 * or it defaults to 1. Increment when adding/removing/changing parameters.
 */
#ifndef NVS_CONFIG_SCHEMA_VERSION
#define NVS_CONFIG_SCHEMA_VERSION 1
#endif

/**
 * @brief Migration callback type for schema version changes.
 * @param old_version Version found in NVS flash.
 * @param new_version Current compiled-in version.
 * @return ESP_OK if migration succeeded and params should be loaded normally.
 *         Any error causes a full reset to defaults.
 */
typedef esp_err_t (*NvsConfigMigrationCb_t)(uint32_t old_version, uint32_t new_version);

/**
 * @brief Register a migration callback.
 *
 * Must be called BEFORE NvsConfig_Init(). When a version mismatch is
 * detected, the callback is invoked. If no callback is registered,
 * all parameters are reset to defaults on version mismatch.
 *
 * @param cb Migration callback.
 * @return ESP_OK on success.
 */
esp_err_t NvsConfig_RegisterMigration(NvsConfigMigrationCb_t cb);

/**
 * @brief Get the current schema version stored in NVS.
 * @return The schema version.
 */
uint32_t NvsConfig_GetSchemaVersion(void);

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
    esp_err_t Param_Reset##name_(void);                                \
    int Param_Print##name_(char* buf, size_t buf_size);
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_) \
    esp_err_t Param_Set##name_(const type_* value, size_t length);            \
    const type_* Param_Get##name_(size_t* out_array_length);                  \
    esp_err_t Param_Copy##name_(type_* buffer, size_t buffer_size);           \
    esp_err_t Param_Reset##name_(void);                                       \
    int Param_Print##name_(char* buf, size_t buf_size);
#include "param_table.inc"
#undef PARAM
#undef ARRAY

#ifdef __cplusplus
}
#endif

#endif  // __NVS_CONFIG_H__