/**
 * @file secure_level.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Manage security levels
 *
 * @copyright Copyright (c) 2025
 */

#include <stdint.h>
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_config.h"

static const char *TAG = "NVS_CONFIG";

/**
 * Check for accidental duplicate secure_level definitions.
 * The secure_level numbers should be unique, continous and start from 0.
 */
typedef enum SecureLevelDuplicates_e {
#define SECURE_LEVEL(lvl, desc) \
    DUPLICATE_LEVEL_##lvl,
#include "param_table.inc"
    STARTING_LEVEL,
} SecureLevelDuplicates_t;

const char *level_meanings[] = {
#define SECURE_LEVEL(lvl, desc) \
    #desc,
#include "param_table.inc"
};

static uint8_t CurrentSecureLevel = STARTING_LEVEL - 1;

uint8_t NvsConfig_SecureLevel(void) { return CurrentSecureLevel; }

esp_err_t NvsConfig_SecureLevelChange(uint8_t new_secure_level)
{
    ESP_LOGW(TAG, "Secure Level Changing (%u %s) -> (%u %s)",
             CurrentSecureLevel, level_meanings[CurrentSecureLevel],
             new_secure_level, level_meanings[new_secure_level]);
    CurrentSecureLevel = new_secure_level;
    return ESP_OK;
}
