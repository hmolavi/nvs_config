#pragma once

#include <stddef.h>
#include "esp_err.h"

typedef uint32_t nvs_handle_t;

typedef enum {
    NVS_READONLY,
    NVS_READWRITE,
} nvs_open_mode_t;

/* Error codes used by NVS */
#define ESP_ERR_NVS_BASE            ((esp_err_t) 0x1100)
#define ESP_ERR_NVS_NOT_FOUND       ((esp_err_t)(ESP_ERR_NVS_BASE + 0x0e))
#define ESP_ERR_NVS_NO_FREE_PAGES   ((esp_err_t)(ESP_ERR_NVS_BASE + 0x0d))
#define ESP_ERR_NVS_NEW_VERSION_FOUND ((esp_err_t)(ESP_ERR_NVS_BASE + 0x10))

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t nvs_open(const char* name, nvs_open_mode_t open_mode, nvs_handle_t* out_handle);
esp_err_t nvs_get_blob(nvs_handle_t c_handle, const char* key, void* out_value, size_t* length);
esp_err_t nvs_set_blob(nvs_handle_t c_handle, const char* key, const void* value, size_t length);
esp_err_t nvs_commit(nvs_handle_t c_handle);
esp_err_t nvs_erase_all(nvs_handle_t c_handle);
void      nvs_close(nvs_handle_t c_handle);

#ifdef __cplusplus
}
#endif
