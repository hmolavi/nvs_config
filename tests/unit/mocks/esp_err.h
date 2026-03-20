#pragma once

#include <stdint.h>

typedef int32_t esp_err_t;

#define ESP_OK                  ((esp_err_t)  0)
#define ESP_FAIL                ((esp_err_t) -1)
#define ESP_ERR_NO_MEM          ((esp_err_t) 0x101)
#define ESP_ERR_INVALID_ARG     ((esp_err_t) 0x102)
#define ESP_ERR_INVALID_STATE   ((esp_err_t) 0x103)
#define ESP_ERR_INVALID_SIZE    ((esp_err_t) 0x104)
#define ESP_ERR_NOT_FOUND       ((esp_err_t) 0x105)

#ifdef __cplusplus
extern "C" {
#endif

const char* esp_err_to_name(esp_err_t code);

#ifdef __cplusplus
}
#endif
