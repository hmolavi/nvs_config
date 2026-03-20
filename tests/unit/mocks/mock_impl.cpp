/**
 * @file mock_impl.cpp
 * @brief Stub implementations of all ESP-IDF APIs used by nvs_config.
 *
 * Default behaviour (unchanged from original):
 *  - nvs_flash_init / nvs_open / nvs_set_blob / nvs_commit → ESP_OK
 *  - nvs_get_blob                                          → ESP_ERR_NVS_NOT_FOUND
 *    (forces NvsConfig_Init to load every parameter from its compiled-in default)
 *  - Mutex stubs are single-threaded no-ops (unit tests never spawn tasks)
 *  - Timer stubs are no-ops (no periodic saves needed in unit tests)
 *
 * Tests that need to exercise error/alternate paths can set the knobs in
 * mock_control.h and call mock_reset_controls() in teardown().
 */

#include "mock_control.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_timer/esp_timer.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

// ── Mock control globals (defaults mirror original fixed-stub behaviour) ──

esp_err_t g_mock_nvs_open_ret           = ESP_OK;
int       g_mock_nvs_get_blob_ok_calls  = 0;
uint8_t   g_mock_nvs_get_blob_data[64]  = {};
esp_err_t g_mock_nvs_set_blob_ret       = ESP_OK;
esp_err_t g_mock_nvs_commit_ret         = ESP_OK;
esp_err_t g_mock_nvs_flash_init_ret     = ESP_OK;
int       g_mock_mutex_fail             = 0;
esp_err_t g_mock_esp_timer_create_ret   = ESP_OK;
esp_err_t g_mock_esp_timer_start_ret    = ESP_OK;

void mock_reset_controls(void)
{
    g_mock_nvs_open_ret          = ESP_OK;
    g_mock_nvs_get_blob_ok_calls = 0;
    memset(g_mock_nvs_get_blob_data, 0, sizeof(g_mock_nvs_get_blob_data));
    g_mock_nvs_set_blob_ret      = ESP_OK;
    g_mock_nvs_commit_ret        = ESP_OK;
    g_mock_nvs_flash_init_ret    = ESP_OK;
    g_mock_mutex_fail            = 0;
    g_mock_esp_timer_create_ret  = ESP_OK;
    g_mock_esp_timer_start_ret   = ESP_OK;
}

// ── esp_err ──

const char* esp_err_to_name(esp_err_t code)
{
    switch (code) {
        case ESP_OK:                    return "ESP_OK";
        case ESP_FAIL:                  return "ESP_FAIL";
        case ESP_ERR_NO_MEM:            return "ESP_ERR_NO_MEM";
        case ESP_ERR_INVALID_ARG:       return "ESP_ERR_INVALID_ARG";
        case ESP_ERR_INVALID_STATE:     return "ESP_ERR_INVALID_STATE";
        case ESP_ERR_INVALID_SIZE:      return "ESP_ERR_INVALID_SIZE";
        case ESP_ERR_NVS_NOT_FOUND:     return "ESP_ERR_NVS_NOT_FOUND";
        case ESP_ERR_NVS_NO_FREE_PAGES: return "ESP_ERR_NVS_NO_FREE_PAGES";
        default:                        return "UNKNOWN_ERROR";
    }
}

// ── NVS flash ──

esp_err_t nvs_flash_init(void)
{
    esp_err_t ret = g_mock_nvs_flash_init_ret;
    g_mock_nvs_flash_init_ret = ESP_OK;  // one-shot; resets after use
    return ret;
}

esp_err_t nvs_flash_erase(void) { return ESP_OK; }

// ── NVS handle operations ──

esp_err_t nvs_open(const char* /*name*/, nvs_open_mode_t /*mode*/, nvs_handle_t* out_handle)
{
    *out_handle = 1;
    return g_mock_nvs_open_ret;
}

esp_err_t nvs_get_blob(nvs_handle_t /*handle*/, const char* /*key*/,
                       void* out, size_t* length)
{
    if (g_mock_nvs_get_blob_ok_calls > 0) {
        g_mock_nvs_get_blob_ok_calls--;
        size_t copy_len = (*length < sizeof(g_mock_nvs_get_blob_data))
                          ? *length : sizeof(g_mock_nvs_get_blob_data);
        memcpy(out, g_mock_nvs_get_blob_data, copy_len);
        return ESP_OK;
    }
    return ESP_ERR_NVS_NOT_FOUND;
}

esp_err_t nvs_set_blob(nvs_handle_t /*handle*/, const char* /*key*/,
                       const void* /*value*/, size_t /*length*/)
{
    return g_mock_nvs_set_blob_ret;
}

esp_err_t nvs_commit(nvs_handle_t /*handle*/) { return g_mock_nvs_commit_ret; }
esp_err_t nvs_erase_all(nvs_handle_t /*handle*/) { return ESP_OK; }
void      nvs_close(nvs_handle_t /*handle*/) {}

// ── FreeRTOS mutex (single-threaded stubs) ──

SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    if (g_mock_mutex_fail) return nullptr;
    return std::malloc(1);
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t /*sem*/, TickType_t /*ticks*/)
{
    return pdTRUE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t /*sem*/)
{
    return pdTRUE;
}

void vSemaphoreDelete(SemaphoreHandle_t sem)
{
    std::free(sem);
}

SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t /*max*/, UBaseType_t /*initial*/)
{
    return std::malloc(1);
}

// ── FreeRTOS timers (v4 path is never called when IDF_VERSION_MAJOR >= 5) ──

TimerHandle_t xTimerCreate(const char* /*name*/, TickType_t /*period*/,
                           UBaseType_t /*auto_reload*/, void* /*id*/,
                           TimerCallbackFunction_t /*cb*/)
{
    return nullptr;
}

BaseType_t xTimerStart(TimerHandle_t /*timer*/, TickType_t /*ticks*/)
{
    return pdTRUE;
}

// ── esp_timer (v5+ path) ──

esp_err_t esp_timer_create(const esp_timer_create_args_t* /*args*/,
                           esp_timer_handle_t* out_handle)
{
    if (g_mock_esp_timer_create_ret != ESP_OK) return g_mock_esp_timer_create_ret;
    *out_handle = std::malloc(1);
    return ESP_OK;
}

esp_err_t esp_timer_start_periodic(esp_timer_handle_t /*timer*/, uint64_t /*period_us*/)
{
    return g_mock_esp_timer_start_ret;
}
