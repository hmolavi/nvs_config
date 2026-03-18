/**
 * @file nvs_config.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief NVS_Config Component Header
 *
 * This file is responsible for parameter initialization and management.
 *
 * @copyright Copyright (c) 2025
 */

#include "nvs_config.h"

#include <esp_err.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
#include "esp_timer/esp_timer.h"
#else
#include "esp_timer.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "NVS_CONFIG";

#define NVS_NAMESPACE "param_storage"

/* Use compiler to verify that the array is initialized properly.
    Char arrays are exept from this check */
#define ARRAY(s, type, size, name, default, d)                           \
    static const type name##_init[] = default;                           \
    _Static_assert(__builtin_types_compatible_p(type, char) ||           \
                       ((sizeof(name##_init) / sizeof(type)) == (size)), \
                   "Initializer count mismatch for " #name);
#include "param_table.inc"
#undef ARRAY

/*
 * Populating 'g_nvsconfig_controller'
 */
#define PARAM(secure_lvl_, type_, name_, default_value_, description_) \
    .name_ = {                                                         \
        .secure_level = secure_lvl_,                                   \
        .name = #name_,                                                \
        .is_dirty = false,                                             \
        .is_default = true,                                            \
        .default_value = default_value_,                               \
        .key = #name_,                                                 \
    },
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_) \
    .name_ = {                                                                \
        .secure_level = secure_lvl_,                                          \
        .name = #name_,                                                       \
        .size = size_,                                                        \
        .is_dirty = false,                                                    \
        .is_default = true,                                                   \
        .default_value = default_value_,                                      \
        .key = #name_,                                                        \
    },
NvsConfigMasterController_t g_nvsconfig_controller = {
#include "param_table.inc"
};
#undef PARAM
#undef ARRAY

/**
 * Helper Macro for Print Formats
 */
#define PRINT_FORMAT(type, format) \
    static inline const char* GetPrintFormat_##type(void) { return format; }
#include "format.inc"
#undef PRINT_FORMAT

/**
 * @brief Getters and Setters ( and Reset and Print functions )
 */
#define PARAM(secure_lvl_, type_, name_, default_value_, description_)                          \
    esp_err_t Param_Set##name_(const type_ value)                                               \
    {                                                                                           \
        if (NvsConfig_SecureLevel() > secure_lvl_) {                                            \
            return ESP_ERR_INVALID_STATE;                                                       \
        }                                                                                       \
        if (g_nvsconfig_controller.name_.value != value) {                                      \
            g_nvsconfig_controller.name_.value = value;                                         \
            g_nvsconfig_controller.name_.is_default = false;                                    \
            g_nvsconfig_controller.name_.is_dirty = true;                                       \
            return ESP_OK;                                                                      \
        }                                                                                       \
        return ESP_FAIL;                                                                        \
    }                                                                                           \
    type_ Param_Get##name_(void)                                                                \
    {                                                                                           \
        return g_nvsconfig_controller.name_.value;                                              \
    }                                                                                           \
    esp_err_t Param_Reset##name_(void)                                                          \
    {                                                                                           \
        if (g_nvsconfig_controller.name_.value != g_nvsconfig_controller.name_.default_value) { \
            g_nvsconfig_controller.name_.value = g_nvsconfig_controller.name_.default_value;    \
            g_nvsconfig_controller.name_.is_default = true;                                     \
            g_nvsconfig_controller.name_.is_dirty = true;                                       \
            return ESP_OK;                                                                      \
        }                                                                                       \
        return ESP_FAIL;                                                                        \
    }                                                                                           \
    int Param_Print##name_(char* buf, size_t buf_size)                                          \
    {                                                                                           \
        return snprintf(buf, buf_size, GetPrintFormat_##type_(),                                \
                        g_nvsconfig_controller.name_.value);                                    \
    }

#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_)                                                     \
    esp_err_t Param_Set##name_(const type_* value, size_t length)                                                                 \
    {                                                                                                                             \
        if (NvsConfig_SecureLevel() > secure_lvl_) {                                                                              \
            return ESP_ERR_INVALID_STATE;                                                                                         \
        }                                                                                                                         \
        if (length > size_) {                                                                                                     \
            return ESP_ERR_INVALID_SIZE;                                                                                          \
        }                                                                                                                         \
        if (memcmp(&g_nvsconfig_controller.name_.value, value, size_ * sizeof(type_)) != 0) {                                     \
            memcpy(&g_nvsconfig_controller.name_.value, value, size_ * sizeof(type_));                                            \
            g_nvsconfig_controller.name_.is_default = false;                                                                      \
            g_nvsconfig_controller.name_.is_dirty = true;                                                                         \
            return ESP_OK;                                                                                                        \
        }                                                                                                                         \
        return ESP_ERR_INVALID_ARG;                                                                                               \
    }                                                                                                                             \
    const type_* Param_Get##name_(size_t* out_array_length)                                                                       \
    {                                                                                                                             \
        if (out_array_length) *out_array_length = g_nvsconfig_controller.name_.size;                                              \
        return g_nvsconfig_controller.name_.value;                                                                                \
    }                                                                                                                             \
    esp_err_t Param_Copy##name_(type_* buffer, size_t buffer_size)                                                                \
    {                                                                                                                             \
        const size_t required_size = g_nvsconfig_controller.name_.size * sizeof(type_);                                           \
        if (buffer_size < required_size) {                                                                                        \
            return ESP_ERR_INVALID_SIZE;                                                                                          \
        }                                                                                                                         \
        memcpy(buffer, g_nvsconfig_controller.name_.value, required_size);                                                        \
        return ESP_OK;                                                                                                            \
    }                                                                                                                             \
    esp_err_t Param_Reset##name_(void)                                                                                            \
    {                                                                                                                             \
        if (memcmp(g_nvsconfig_controller.name_.value, g_nvsconfig_controller.name_.default_value, size_ * sizeof(type_)) != 0) { \
            memcpy(g_nvsconfig_controller.name_.value, g_nvsconfig_controller.name_.default_value, size_ * sizeof(type_));        \
            g_nvsconfig_controller.name_.is_default = true;                                                                       \
            g_nvsconfig_controller.name_.is_dirty = true;                                                                         \
            return ESP_OK;                                                                                                        \
        }                                                                                                                         \
        return ESP_FAIL;                                                                                                          \
    }                                                                                                                             \
    int Param_Print##name_(char* buf, size_t buf_size)                                                                            \
    {                                                                                                                             \
        /* Special handling for char arrays (strings) */                                                                          \
        if (strcmp(GetPrintFormat_##type_(), "%s") == 0) {                                                                        \
            /* Assuming value is null-terminated or size_ includes null */                                                        \
            return snprintf(buf, buf_size, "%s", (char*)g_nvsconfig_controller.name_.value);                                      \
        }                                                                                                                         \
        else {                                                                                                                    \
            int offset = 0;                                                                                                       \
            size_t remaining_size = buf_size;                                                                                     \
            /* Print array elements separated by commas */                                                                        \
            offset += snprintf(buf + offset, remaining_size, "[");                                                                \
            if (offset >= buf_size) return offset; /* Check bounds */                                                             \
            remaining_size = buf_size - offset;                                                                                   \
            for (size_t i = 0; i < g_nvsconfig_controller.name_.size; ++i) {                                                      \
                int written = snprintf(buf + offset, remaining_size,                                                              \
                                       GetPrintFormat_##type_(),                                                                  \
                                       g_nvsconfig_controller.name_.value[i]);                                                    \
                if (written < 0 || (size_t)written >= remaining_size) {                                                           \
                    buf[buf_size - 1] = '\0'; /* Ensure termination */                                                            \
                    return buf_size;          /* Indicate truncation/error */                                                     \
                }                                                                                                                 \
                offset += written;                                                                                                \
                remaining_size -= written;                                                                                        \
                                                                                                                                  \
                if (i < g_nvsconfig_controller.name_.size - 1) {                                                                  \
                    /* Add separator if space allows */                                                                           \
                    if (remaining_size > 1) {                                                                                     \
                        buf[offset++] = ',';                                                                                      \
                        buf[offset] = '\0';                                                                                       \
                        remaining_size--;                                                                                         \
                    }                                                                                                             \
                    else {                                                                                                        \
                        buf[buf_size - 1] = '\0';                                                                                 \
                        return buf_size; /* Not enough space */                                                                   \
                    }                                                                                                             \
                }                                                                                                                 \
            }                                                                                                                     \
            /* Add closing bracket if space allows */                                                                             \
            if (remaining_size > 1) {                                                                                             \
                buf[offset++] = ']';                                                                                              \
                buf[offset] = '\0';                                                                                               \
                remaining_size--;                                                                                                 \
            }                                                                                                                     \
            else {                                                                                                                \
                buf[buf_size - 1] = '\0';                                                                                         \
                return buf_size; /* Not enough space */                                                                           \
            }                                                                                                                     \
            return offset; /* Return total characters written */                                                                  \
        }                                                                                                                         \
    }
#include "param_table.inc"
#undef PARAM
#undef ARRAY

void NvsConfig_SaveDirtyParameters(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace '%s' (Error: 0x%x %s)",
                 NVS_NAMESPACE, err, esp_err_to_name(err));
        return;  // Cannot proceed without NVS handle
    }

    int parametersChanged = 0;

#define PARAM(secure_lvl_, type_, name_, default_value_, description_)                                                           \
    if (g_nvsconfig_controller.name_.is_dirty) {                                                                                 \
        size_t name_##required_size = sizeof(type_);                                                                             \
        /* Log before attempting to save */                                                                                      \
        ESP_LOGD(TAG, "Saving PARAM '%s', key '%s', size %u",                                                                    \
                 g_nvsconfig_controller.name_.name,                                                                              \
                 g_nvsconfig_controller.name_.key,                                                                               \
                 (unsigned int)name_##required_size);                                                                            \
        err = nvs_set_blob(handle, g_nvsconfig_controller.name_.key, &g_nvsconfig_controller.name_.value, name_##required_size); \
        if (err != ESP_OK) {                                                                                                     \
            ESP_LOGE(TAG, "Failed to set blob for PARAM: %s (Key: %s, Error: 0x%x %s)",                                          \
                     g_nvsconfig_controller.name_.name, g_nvsconfig_controller.name_.key, err, esp_err_to_name(err));            \
        }                                                                                                                        \
        else {                                                                                                                   \
            g_nvsconfig_controller.name_.is_dirty = false;                                                                       \
            parametersChanged++;                                                                                                 \
            ESP_LOGD(TAG, "Successfully saved PARAM '%s'", g_nvsconfig_controller.name_.name);                                   \
        }                                                                                                                        \
    }
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_)                                                    \
    if (g_nvsconfig_controller.name_.is_dirty) {                                                                                 \
        size_t name_##required_size = size_ * sizeof(type_);                                                                     \
        /* Log before attempting to save */                                                                                      \
        ESP_LOGD(TAG, "Saving ARRAY '%s', key '%s', size %u (elements %u, element_size %u)",                                     \
                 g_nvsconfig_controller.name_.name,                                                                              \
                 g_nvsconfig_controller.name_.key,                                                                               \
                 (unsigned int)name_##required_size,                                                                             \
                 (unsigned int)size_,                                                                                            \
                 (unsigned int)sizeof(type_));                                                                                   \
        err = nvs_set_blob(handle, g_nvsconfig_controller.name_.key, &g_nvsconfig_controller.name_.value, name_##required_size); \
        if (err != ESP_OK) {                                                                                                     \
            ESP_LOGE(TAG, "Failed to set blob for ARRAY: %s (Key: %s, Error: 0x%x %s)",                                          \
                     g_nvsconfig_controller.name_.name, g_nvsconfig_controller.name_.key, err, esp_err_to_name(err));            \
        }                                                                                                                        \
        else {                                                                                                                   \
            g_nvsconfig_controller.name_.is_dirty = false;                                                                       \
            parametersChanged++;                                                                                                 \
            ESP_LOGD(TAG, "Successfully saved ARRAY '%s'", g_nvsconfig_controller.name_.name);                                   \
        }                                                                                                                        \
    }
#include "param_table.inc"
#undef PARAM
#undef ARRAY

    // Commit changes if any parameters were successfully saved
    if (parametersChanged > 0) {
        char buf[128];
        int len = snprintf(buf, sizeof(buf), "%d dirty parameters committing to flash...", parametersChanged);
        err = nvs_commit(handle);
        if (err != ESP_OK) {
            len += snprintf(buf + len, sizeof(buf) - len, " Failed");
            ESP_LOGE(TAG, "%s (Error: 0x%x %s)", buf, err, esp_err_to_name(err));
            ESP_LOGE(TAG, "NVS commit failed!");
        }
        else {
            len += snprintf(buf + len, sizeof(buf) - len, " Done");
            ESP_LOGI(TAG, "%s", buf);
        }
    }
    nvs_close(handle);
}

#if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
static void save_dirty_parameters_callback(void *arg)
{
    (void) arg;
    NvsConfig_SaveDirtyParameters();
}
#else 
static void save_dirty_parameters_callback(TimerHandle_t xTimer)
{
    NvsConfig_SaveDirtyParameters();
}
#endif // defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)

esp_err_t NvsConfig_Init(void)
{
    // NVS initialization
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
#define PARAM(secure_lvl_, type_, name_, default_value_, description_)                                                                   \
    size_t name_##_required_size = sizeof(g_nvsconfig_controller.name_.value);                                                           \
    if (nvs_get_blob(handle, g_nvsconfig_controller.name_.key, &g_nvsconfig_controller.name_.value, &name_##_required_size) != ESP_OK) { \
        g_nvsconfig_controller.name_.value = g_nvsconfig_controller.name_.default_value;                                                 \
        g_nvsconfig_controller.name_.is_default = true;                                                                                  \
        g_nvsconfig_controller.name_.is_dirty = true;                                                                                    \
    }                                                                                                                                    \
    else {                                                                                                                               \
        g_nvsconfig_controller.name_.is_dirty = false;                                                                                   \
        if (g_nvsconfig_controller.name_.value != g_nvsconfig_controller.name_.default_value) {                                          \
            g_nvsconfig_controller.name_.is_default = false;                                                                             \
        }                                                                                                                                \
        else {                                                                                                                           \
            g_nvsconfig_controller.name_.is_default = true;                                                                              \
        }                                                                                                                                \
    }
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_)                                                                 \
    size_t name_##_required_size = sizeof(g_nvsconfig_controller.name_.value);                                                                \
    if (nvs_get_blob(handle, g_nvsconfig_controller.name_.key, &g_nvsconfig_controller.name_.value, &name_##_required_size) != ESP_OK) {      \
        memcpy(&g_nvsconfig_controller.name_.value, &g_nvsconfig_controller.name_.default_value, sizeof(g_nvsconfig_controller.name_.value)); \
        g_nvsconfig_controller.name_.is_dirty = true;                                                                                         \
        g_nvsconfig_controller.name_.is_default = true;                                                                                       \
    }                                                                                                                                         \
    else {                                                                                                                                    \
        g_nvsconfig_controller.name_.is_dirty = false;                                                                                        \
        if (memcmp(&g_nvsconfig_controller.name_.value, &g_nvsconfig_controller.name_.default_value, size_ * sizeof(type_)) != 0) {           \
            g_nvsconfig_controller.name_.is_default = false;                                                                                  \
        }                                                                                                                                     \
        else {                                                                                                                                \
            g_nvsconfig_controller.name_.is_default = true;                                                                                   \
        }                                                                                                                                     \
    }

#include "param_table.inc"
#undef PARAM
#undef ARRAY

        nvs_close(handle);
    }

    #if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
    // Using esp_timer in ESP-IDF v5 and later
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &save_dirty_parameters_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "g_param_save"
    };
    esp_timer_handle_t periodic_timer;
    esp_err_t err = esp_timer_create(&periodic_timer_args, &periodic_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create periodic timer");
        return ESP_FAIL;
    }
    err = esp_timer_start_periodic(periodic_timer, 30000 * 1000); // period in microseconds (30 seconds)
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start periodic timer");
        return err;
    }
    #else
    // Using FreeRTOS timers for earlier ESP-IDF versions
    TimerHandle_t xTimer = xTimerCreate(
        "g_param_save",
        pdMS_TO_TICKS(30000),  // Timer period in ticks (30 seconds)
        pdTRUE,
        (void*)0,
        save_dirty_parameters_callback);

    if (xTimer != NULL) {
        xTimerStart(xTimer, 0);
    }
    else {
        ESP_LOGE(TAG, "Failed to create periodic timer");
        return ESP_FAIL;
    }
    #endif // defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
    
    return ESP_OK;
}