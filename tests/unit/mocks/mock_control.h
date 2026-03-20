/**
 * @file mock_control.h
 * @brief Knobs for controlling mock behaviour from test code.
 *
 * Default values mirror the original fixed-stub behaviour so existing tests
 * are unaffected.  Call mock_reset_controls() in teardown() to restore them.
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── nvs_open ─────────────────────────────────────────────────────────── */
/** Return value for nvs_open().  Default: ESP_OK. */
extern esp_err_t g_mock_nvs_open_ret;

/* ── nvs_get_blob ─────────────────────────────────────────────────────── */
/**
 * The first g_mock_nvs_get_blob_ok_calls calls to nvs_get_blob() return
 * ESP_OK and memcpy g_mock_nvs_get_blob_data (up to *length bytes) into
 * the caller's buffer.  All subsequent calls return ESP_ERR_NVS_NOT_FOUND.
 */
extern int     g_mock_nvs_get_blob_ok_calls;
extern uint8_t g_mock_nvs_get_blob_data[64];

/* ── nvs_set_blob ─────────────────────────────────────────────────────── */
/** Return value for nvs_set_blob().  Default: ESP_OK. */
extern esp_err_t g_mock_nvs_set_blob_ret;

/* ── nvs_commit ───────────────────────────────────────────────────────── */
/** Return value for nvs_commit().  Default: ESP_OK. */
extern esp_err_t g_mock_nvs_commit_ret;

/* ── nvs_flash_init ───────────────────────────────────────────────────── */
/**
 * Return value for the NEXT nvs_flash_init() call, then resets to ESP_OK.
 * Set to ESP_ERR_NVS_NO_FREE_PAGES or ESP_ERR_NVS_NEW_VERSION_FOUND to
 * exercise the erase-and-reinit branch in NvsConfig_Init().
 */
extern esp_err_t g_mock_nvs_flash_init_ret;

/* ── xSemaphoreCreateMutex ────────────────────────────────────────────── */
/** When non-zero, xSemaphoreCreateMutex() returns NULL. Default: 0. */
extern int g_mock_mutex_fail;

/* ── esp_timer ────────────────────────────────────────────────────────── */
/** Return value for esp_timer_create().         Default: ESP_OK. */
extern esp_err_t g_mock_esp_timer_create_ret;
/** Return value for esp_timer_start_periodic(). Default: ESP_OK. */
extern esp_err_t g_mock_esp_timer_start_ret;

/* ── helpers ──────────────────────────────────────────────────────────── */
/** Reset every control to its default value. */
void mock_reset_controls(void);

#ifdef __cplusplus
}
#endif
