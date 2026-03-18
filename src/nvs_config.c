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
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "NVS_CONFIG";

#define NVS_NAMESPACE "param_storage"

/** Mutex protecting all access to g_nvsconfig_controller. */
static SemaphoreHandle_t s_nvs_mutex = NULL;

/**
 * @brief Change callback storage.
 */
#define NVS_CONFIG_MAX_CALLBACKS 16

typedef struct {
    const char* param_name;  /* NULL = global (fires for any param) */
    NvsConfigOnChange_t cb;
    void* user_data;
} _NvsConfigCallbackEntry_t;

static _NvsConfigCallbackEntry_t s_callbacks[NVS_CONFIG_MAX_CALLBACKS];
static size_t s_callback_count = 0;

esp_err_t NvsConfig_RegisterOnChange(const char* param_name,
                                     NvsConfigOnChange_t cb,
                                     void* user_data)
{
    if (s_callback_count >= NVS_CONFIG_MAX_CALLBACKS) return ESP_ERR_NO_MEM;
    s_callbacks[s_callback_count].param_name = param_name;
    s_callbacks[s_callback_count].cb = cb;
    s_callbacks[s_callback_count].user_data = user_data;
    s_callback_count++;
    return ESP_OK;
}

esp_err_t NvsConfig_RegisterGlobalOnChange(NvsConfigOnChange_t cb,
                                           void* user_data)
{
    return NvsConfig_RegisterOnChange(NULL, cb, user_data);
}

void NvsConfig_ClearCallbacks(void)
{
    s_callback_count = 0;
    memset(s_callbacks, 0, sizeof(s_callbacks));
}

/**
 * @brief Notify registered callbacks that a parameter changed.
 *
 * Called AFTER the mutex is released to prevent deadlocks
 * (callbacks may read other params).
 */
static void _nvsconfig_notify_change(const char* param_name)
{
    for (size_t i = 0; i < s_callback_count; i++) {
        if (s_callbacks[i].param_name == NULL ||
            strcmp(s_callbacks[i].param_name, param_name) == 0) {
            s_callbacks[i].cb(param_name, s_callbacks[i].user_data);
        }
    }
}

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
 * @brief Parameter index enum for write-count tracking.
 */
typedef enum {
#define PARAM(s, t, name, d, desc)      PARAM_INDEX_##name,
#define ARRAY(s, t, sz, name, d, desc)  PARAM_INDEX_##name,
#include "param_table.inc"
#undef PARAM
#undef ARRAY
    PARAM_INDEX_COUNT
} NvsConfigParamIndex_t;

static uint32_t s_write_counts[PARAM_INDEX_COUNT] = {0};

uint32_t NvsConfig_GetWriteCount(const char* name)
{
    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        if (strcmp(g_nvsconfig_params[i].name, name) == 0) {
            return s_write_counts[i];
        }
    }
    return 0;
}

uint32_t NvsConfig_GetTotalWriteCount(void)
{
    uint32_t total = 0;
    for (size_t i = 0; i < PARAM_INDEX_COUNT; i++) {
        total += s_write_counts[i];
    }
    return total;
}

void NvsConfig_ResetWriteCounts(void)
{
    memset(s_write_counts, 0, sizeof(s_write_counts));
}

/**
 * @brief Schema versioning support.
 */
static NvsConfigMigrationCb_t s_migration_cb = NULL;
static uint32_t s_schema_version = NVS_CONFIG_SCHEMA_VERSION;

#define NVS_SCHEMA_KEY "schema_ver"

esp_err_t NvsConfig_RegisterMigration(NvsConfigMigrationCb_t cb)
{
    s_migration_cb = cb;
    return ESP_OK;
}

uint32_t NvsConfig_GetSchemaVersion(void)
{
    return s_schema_version;
}

/**
 * @brief Getters and Setters ( and Reset and Print functions )
 *
 * All functions acquire s_nvs_mutex for thread-safe access to the controller.
 */
#define PARAM(secure_lvl_, type_, name_, default_value_, description_)                          \
    esp_err_t Param_Set##name_(const type_ value)                                               \
    {                                                                                           \
        if (NvsConfig_SecureLevel() > secure_lvl_) {                                            \
            return ESP_ERR_INVALID_STATE;                                                       \
        }                                                                                       \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                             \
        esp_err_t _ret;                                                                         \
        if (g_nvsconfig_controller.name_.value != value) {                                      \
            g_nvsconfig_controller.name_.value = value;                                         \
            g_nvsconfig_controller.name_.is_default = false;                                    \
            g_nvsconfig_controller.name_.is_dirty = true;                                       \
            s_write_counts[PARAM_INDEX_##name_]++;                                              \
            _ret = ESP_OK;                                                                      \
        } else {                                                                                \
            _ret = ESP_FAIL;                                                                    \
        }                                                                                       \
        xSemaphoreGive(s_nvs_mutex);                                                            \
        if (_ret == ESP_OK) _nvsconfig_notify_change(#name_);                                   \
        return _ret;                                                                            \
    }                                                                                           \
    type_ Param_Get##name_(void)                                                                \
    {                                                                                           \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                             \
        type_ _val = g_nvsconfig_controller.name_.value;                                        \
        xSemaphoreGive(s_nvs_mutex);                                                            \
        return _val;                                                                            \
    }                                                                                           \
    esp_err_t Param_Reset##name_(void)                                                          \
    {                                                                                           \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                             \
        esp_err_t _ret;                                                                         \
        if (g_nvsconfig_controller.name_.value != g_nvsconfig_controller.name_.default_value) { \
            g_nvsconfig_controller.name_.value = g_nvsconfig_controller.name_.default_value;    \
            g_nvsconfig_controller.name_.is_default = true;                                     \
            g_nvsconfig_controller.name_.is_dirty = true;                                       \
            _ret = ESP_OK;                                                                      \
        } else {                                                                                \
            _ret = ESP_FAIL;                                                                    \
        }                                                                                       \
        xSemaphoreGive(s_nvs_mutex);                                                            \
        return _ret;                                                                            \
    }                                                                                           \
    int Param_Print##name_(char* buf, size_t buf_size)                                          \
    {                                                                                           \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                             \
        int _n = snprintf(buf, buf_size, GetPrintFormat_##type_(),                              \
                          g_nvsconfig_controller.name_.value);                                  \
        xSemaphoreGive(s_nvs_mutex);                                                            \
        return _n;                                                                              \
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
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                                                               \
        esp_err_t _ret;                                                                                                           \
        if (memcmp(&g_nvsconfig_controller.name_.value, value, size_ * sizeof(type_)) != 0) {                                     \
            memcpy(&g_nvsconfig_controller.name_.value, value, size_ * sizeof(type_));                                            \
            g_nvsconfig_controller.name_.is_default = false;                                                                      \
            g_nvsconfig_controller.name_.is_dirty = true;                                                                         \
            s_write_counts[PARAM_INDEX_##name_]++;                                                                                \
            _ret = ESP_OK;                                                                                                        \
        } else {                                                                                                                  \
            _ret = ESP_ERR_INVALID_ARG;                                                                                           \
        }                                                                                                                         \
        xSemaphoreGive(s_nvs_mutex);                                                                                              \
        if (_ret == ESP_OK) _nvsconfig_notify_change(#name_);                                                                     \
        return _ret;                                                                                                              \
    }                                                                                                                             \
    const type_* Param_Get##name_(size_t* out_array_length)                                                                       \
    {                                                                                                                             \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                                                               \
        if (out_array_length) *out_array_length = g_nvsconfig_controller.name_.size;                                              \
        const type_* _ptr = g_nvsconfig_controller.name_.value;                                                                   \
        xSemaphoreGive(s_nvs_mutex);                                                                                              \
        return _ptr;                                                                                                              \
    }                                                                                                                             \
    esp_err_t Param_Copy##name_(type_* buffer, size_t buffer_size)                                                                \
    {                                                                                                                             \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                                                               \
        const size_t required_size = g_nvsconfig_controller.name_.size * sizeof(type_);                                           \
        if (buffer_size < required_size) {                                                                                        \
            xSemaphoreGive(s_nvs_mutex);                                                                                          \
            return ESP_ERR_INVALID_SIZE;                                                                                          \
        }                                                                                                                         \
        memcpy(buffer, g_nvsconfig_controller.name_.value, required_size);                                                        \
        xSemaphoreGive(s_nvs_mutex);                                                                                              \
        return ESP_OK;                                                                                                            \
    }                                                                                                                             \
    esp_err_t Param_Reset##name_(void)                                                                                            \
    {                                                                                                                             \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                                                               \
        esp_err_t _ret;                                                                                                           \
        if (memcmp(g_nvsconfig_controller.name_.value, g_nvsconfig_controller.name_.default_value, size_ * sizeof(type_)) != 0) { \
            memcpy(g_nvsconfig_controller.name_.value, g_nvsconfig_controller.name_.default_value, size_ * sizeof(type_));        \
            g_nvsconfig_controller.name_.is_default = true;                                                                       \
            g_nvsconfig_controller.name_.is_dirty = true;                                                                         \
            _ret = ESP_OK;                                                                                                        \
        } else {                                                                                                                  \
            _ret = ESP_FAIL;                                                                                                      \
        }                                                                                                                         \
        xSemaphoreGive(s_nvs_mutex);                                                                                              \
        return _ret;                                                                                                              \
    }                                                                                                                             \
    int Param_Print##name_(char* buf, size_t buf_size)                                                                            \
    {                                                                                                                             \
        xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);                                                                               \
        int _result;                                                                                                              \
        /* Special handling for char arrays (strings) */                                                                          \
        if (strcmp(GetPrintFormat_##type_(), "%s") == 0) {                                                                        \
            _result = snprintf(buf, buf_size, "%s", (char*)g_nvsconfig_controller.name_.value);                                   \
        }                                                                                                                         \
        else {                                                                                                                    \
            int offset = 0;                                                                                                       \
            size_t remaining_size = buf_size;                                                                                     \
            offset += snprintf(buf + offset, remaining_size, "[");                                                                \
            if (offset >= (int)buf_size) { xSemaphoreGive(s_nvs_mutex); return offset; }                                          \
            remaining_size = buf_size - offset;                                                                                   \
            for (size_t i = 0; i < g_nvsconfig_controller.name_.size; ++i) {                                                      \
                int written = snprintf(buf + offset, remaining_size,                                                              \
                                       GetPrintFormat_##type_(),                                                                  \
                                       g_nvsconfig_controller.name_.value[i]);                                                    \
                if (written < 0 || (size_t)written >= remaining_size) {                                                           \
                    buf[buf_size - 1] = '\0';                                                                                     \
                    xSemaphoreGive(s_nvs_mutex);                                                                                  \
                    return buf_size;                                                                                              \
                }                                                                                                                 \
                offset += written;                                                                                                \
                remaining_size -= written;                                                                                        \
                if (i < g_nvsconfig_controller.name_.size - 1) {                                                                  \
                    if (remaining_size > 1) {                                                                                     \
                        buf[offset++] = ',';                                                                                      \
                        buf[offset] = '\0';                                                                                       \
                        remaining_size--;                                                                                         \
                    } else {                                                                                                      \
                        buf[buf_size - 1] = '\0';                                                                                 \
                        xSemaphoreGive(s_nvs_mutex);                                                                              \
                        return buf_size;                                                                                          \
                    }                                                                                                             \
                }                                                                                                                 \
            }                                                                                                                     \
            if (remaining_size > 1) {                                                                                             \
                buf[offset++] = ']';                                                                                              \
                buf[offset] = '\0';                                                                                               \
            } else {                                                                                                              \
                buf[buf_size - 1] = '\0';                                                                                         \
                xSemaphoreGive(s_nvs_mutex);                                                                                      \
                return buf_size;                                                                                                  \
            }                                                                                                                     \
            _result = offset;                                                                                                     \
        }                                                                                                                         \
        xSemaphoreGive(s_nvs_mutex);                                                                                              \
        return _result;                                                                                                           \
    }
#include "param_table.inc"
#undef PARAM
#undef ARRAY

/**
 * @brief Registry wrapper functions.
 *
 * Generate static wrappers with uniform signatures so they can be stored
 * as function pointers in the NvsConfigParamEntry_t registry.
 */
#define PARAM(secure_lvl_, type_, name_, default_value_, description_)                 \
    static bool _registry_is_dirty_##name_(void) {                                     \
        return g_nvsconfig_controller.name_.is_dirty;                                  \
    }                                                                                  \
    static bool _registry_is_default_##name_(void) {                                   \
        return g_nvsconfig_controller.name_.is_default;                                \
    }                                                                                  \
    static esp_err_t _registry_reset_##name_(void) {                                   \
        return Param_Reset##name_();                                                   \
    }                                                                                  \
    static int _registry_print_##name_(char* buf, size_t buf_size) {                   \
        return Param_Print##name_(buf, buf_size);                                      \
    }                                                                                  \
    static esp_err_t _registry_set_##name_(const void* data, size_t data_size) {       \
        if (data_size != sizeof(type_)) return ESP_ERR_INVALID_SIZE;                   \
        type_ val;                                                                     \
        memcpy(&val, data, sizeof(type_));                                             \
        return Param_Set##name_(val);                                                  \
    }

#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_)           \
    static bool _registry_is_dirty_##name_(void) {                                     \
        return g_nvsconfig_controller.name_.is_dirty;                                  \
    }                                                                                  \
    static bool _registry_is_default_##name_(void) {                                   \
        return g_nvsconfig_controller.name_.is_default;                                \
    }                                                                                  \
    static esp_err_t _registry_reset_##name_(void) {                                   \
        return Param_Reset##name_();                                                   \
    }                                                                                  \
    static int _registry_print_##name_(char* buf, size_t buf_size) {                   \
        return Param_Print##name_(buf, buf_size);                                      \
    }                                                                                  \
    static esp_err_t _registry_set_##name_(const void* data, size_t data_size) {       \
        const size_t full_size = size_ * sizeof(type_);                                \
        if (data_size > full_size) return ESP_ERR_INVALID_SIZE;                         \
        if (data_size == full_size) {                                                   \
            return Param_Set##name_((const type_*)data, size_);                         \
        }                                                                               \
        /* Partial write: zero-fill remaining elements */                               \
        type_ tmp[size_];                                                               \
        memset(tmp, 0, full_size);                                                      \
        memcpy(tmp, data, data_size);                                                   \
        Param_Set##name_(tmp, size_);                                                   \
        return ESP_ERR_INVALID_SIZE; /* warning: partial write */                       \
    }
#include "param_table.inc"
#undef PARAM
#undef ARRAY

/**
 * @brief Parameter registry: const array of entries with function pointers.
 */
#define PARAM(secure_lvl_, type_, name_, default_value_, description_)  \
    {                                                                    \
        .name = #name_,                                                 \
        .description = description_,                                    \
        .secure_level = secure_lvl_,                                    \
        .is_array = false,                                              \
        .element_size = sizeof(type_),                                  \
        .element_count = 1,                                             \
        .is_dirty = _registry_is_dirty_##name_,                         \
        .is_default = _registry_is_default_##name_,                     \
        .reset = _registry_reset_##name_,                               \
        .print = _registry_print_##name_,                               \
        .set = _registry_set_##name_,                                   \
    },
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_) \
    {                                                                          \
        .name = #name_,                                                       \
        .description = description_,                                          \
        .secure_level = secure_lvl_,                                          \
        .is_array = true,                                                     \
        .element_size = sizeof(type_),                                        \
        .element_count = size_,                                               \
        .is_dirty = _registry_is_dirty_##name_,                               \
        .is_default = _registry_is_default_##name_,                           \
        .reset = _registry_reset_##name_,                                     \
        .print = _registry_print_##name_,                                     \
        .set = _registry_set_##name_,                                         \
    },
const NvsConfigParamEntry_t g_nvsconfig_params[] = {
#include "param_table.inc"
};
#undef PARAM
#undef ARRAY

const size_t g_nvsconfig_param_count =
    sizeof(g_nvsconfig_params) / sizeof(g_nvsconfig_params[0]);

const NvsConfigParamEntry_t* NvsConfig_FindParam(const char* name)
{
    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        if (strcmp(g_nvsconfig_params[i].name, name) == 0) {
            return &g_nvsconfig_params[i];
        }
    }
    return NULL;
}

void NvsConfig_ResetAll(void)
{
    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        g_nvsconfig_params[i].reset();
    }
}

void NvsConfig_PrintAll(void)
{
    char buf[128];
    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        g_nvsconfig_params[i].print(buf, sizeof(buf));
        ESP_LOGI(TAG, "%-16s = %s", g_nvsconfig_params[i].name, buf);
    }
}

void NvsConfig_SaveDirtyParameters(void)
{
    xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);

    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace '%s' (Error: 0x%x %s)",
                 NVS_NAMESPACE, err, esp_err_to_name(err));
        xSemaphoreGive(s_nvs_mutex);
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
    xSemaphoreGive(s_nvs_mutex);
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
    // Create mutex for thread-safe parameter access
    if (s_nvs_mutex == NULL) {
        s_nvs_mutex = xSemaphoreCreateMutex();
        if (s_nvs_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create NVS config mutex");
            return ESP_FAIL;
        }
    }

    // NVS initialization
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {

        /* Schema version check */
        uint32_t stored_version = 0;
        size_t ver_size = sizeof(stored_version);
        bool version_mismatch = false;

        if (nvs_get_blob(handle, NVS_SCHEMA_KEY, &stored_version, &ver_size) == ESP_OK) {
            if (stored_version != NVS_CONFIG_SCHEMA_VERSION) {
                ESP_LOGW(TAG, "Schema version mismatch: NVS=%lu, current=%lu",
                         (unsigned long)stored_version, (unsigned long)NVS_CONFIG_SCHEMA_VERSION);
                version_mismatch = true;
                if (s_migration_cb) {
                    esp_err_t migration_result = s_migration_cb(stored_version, NVS_CONFIG_SCHEMA_VERSION);
                    if (migration_result != ESP_OK) {
                        ESP_LOGW(TAG, "Migration failed, resetting all parameters to defaults");
                        nvs_erase_all(handle);
                        nvs_commit(handle);
                    }
                } else {
                    ESP_LOGW(TAG, "No migration callback, resetting all parameters to defaults");
                    nvs_erase_all(handle);
                    nvs_commit(handle);
                }
            }
        } else {
            ESP_LOGI(TAG, "First boot: no schema version in NVS");
        }

        /* Write current schema version to NVS */
        nvs_set_blob(handle, NVS_SCHEMA_KEY, &(uint32_t){NVS_CONFIG_SCHEMA_VERSION}, sizeof(uint32_t));
        s_schema_version = NVS_CONFIG_SCHEMA_VERSION;

        (void)version_mismatch;

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