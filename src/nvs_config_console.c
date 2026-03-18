/**
 * @file nvs_config_console.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief ESP-IDF console commands for interactive parameter management.
 *
 * @copyright Copyright (c) 2025
 */

#include "nvs_config_console.h"
#include "nvs_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_console.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"

static const char *TAG = "NVS_CONSOLE";

/* ── param list ── */

static int cmd_param_list(int argc, char **argv)
{
    (void)argc; (void)argv;
    char buf[128];

    printf("%-16s %-6s %-5s %-5s %-7s  %s\n",
           "NAME", "LEVEL", "DIRTY", "DFLT", "TYPE", "VALUE");
    printf("--------------------------------------------------------------\n");

    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        const NvsConfigParamEntry_t *e = &g_nvsconfig_params[i];
        e->print(buf, sizeof(buf));
        printf("%-16s %-6u %-5s %-5s %-7s  %s\n",
               e->name,
               e->secure_level,
               e->is_dirty()   ? "yes" : "no",
               e->is_default() ? "yes" : "no",
               e->is_array     ? "array" : "scalar",
               buf);
    }
    return 0;
}

/* ── param get <name> ── */

static struct {
    struct arg_str *name;
    struct arg_end *end;
} s_get_args;

static int cmd_param_get(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&s_get_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_get_args.end, argv[0]);
        return 1;
    }

    const NvsConfigParamEntry_t *e = NvsConfig_FindParam(s_get_args.name->sval[0]);
    if (!e) {
        printf("Parameter '%s' not found\n", s_get_args.name->sval[0]);
        return 1;
    }

    char buf[128];
    e->print(buf, sizeof(buf));
    printf("%s = %s\n", e->name, buf);
    return 0;
}

/* ── param set <name> <value> ── */

/**
 * Parse a string value into a buffer based on element size.
 * Supports scalars only (console limitation).
 */
static esp_err_t _console_parse_scalar(const char* str, void* out, size_t elem_size)
{
    switch (elem_size) {
    case 1: {
        /* Could be char, bool, int8_t, or uint8_t */
        long v = strtol(str, NULL, 0);
        uint8_t byte = (uint8_t)v;
        /* If input is a single non-digit char, treat as char literal */
        if (str[0] != '\0' && str[1] == '\0' && (str[0] < '0' || str[0] > '9') && str[0] != '-') {
            byte = (uint8_t)str[0];
        }
        memcpy(out, &byte, 1);
        return ESP_OK;
    }
    case 2: { uint16_t v = (uint16_t)strtoul(str, NULL, 0); memcpy(out, &v, 2); return ESP_OK; }
    case 4: {
        /* Could be int32_t, uint32_t, or float */
        char *end;
        /* Try as integer first; if string contains '.', parse as float */
        if (strchr(str, '.') != NULL) {
            float f = strtof(str, NULL);
            memcpy(out, &f, 4);
        } else {
            uint32_t v = (uint32_t)strtoul(str, &end, 0);
            memcpy(out, &v, 4);
        }
        return ESP_OK;
    }
    case 8: {
        if (strchr(str, '.') != NULL) {
            double d = strtod(str, NULL);
            memcpy(out, &d, 8);
        } else {
            uint64_t v = (uint64_t)strtoull(str, NULL, 0);
            memcpy(out, &v, 8);
        }
        return ESP_OK;
    }
    default:
        return ESP_ERR_NOT_SUPPORTED;
    }
}

static struct {
    struct arg_str *name;
    struct arg_str *value;
    struct arg_end *end;
} s_set_args;

static int cmd_param_set(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&s_set_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_set_args.end, argv[0]);
        return 1;
    }

    const NvsConfigParamEntry_t *e = NvsConfig_FindParam(s_set_args.name->sval[0]);
    if (!e) {
        printf("Parameter '%s' not found\n", s_set_args.name->sval[0]);
        return 1;
    }

    if (e->is_array) {
        printf("Array parameters cannot be set from the console\n");
        return 1;
    }

    uint8_t val_buf[8]; /* largest scalar is 8 bytes (double/int64) */
    esp_err_t parse_rc = _console_parse_scalar(s_set_args.value->sval[0], val_buf, e->element_size);
    if (parse_rc != ESP_OK) {
        printf("Failed to parse value for '%s'\n", e->name);
        return 1;
    }

    esp_err_t rc = e->set(val_buf, e->element_size);
    if (rc == ESP_OK) {
        char buf[128];
        e->print(buf, sizeof(buf));
        printf("%s = %s\n", e->name, buf);
    } else {
        printf("Failed to set '%s': %s (0x%x)\n", e->name, esp_err_to_name(rc), rc);
    }
    return (rc == ESP_OK) ? 0 : 1;
}

/* ── param reset <name> ── */

static struct {
    struct arg_str *name;
    struct arg_end *end;
} s_reset_args;

static int cmd_param_reset(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&s_reset_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_reset_args.end, argv[0]);
        return 1;
    }

    const char *name = s_reset_args.name->sval[0];

    if (strcmp(name, "all") == 0) {
        NvsConfig_ResetAll();
        printf("All parameters reset to defaults\n");
        return 0;
    }

    const NvsConfigParamEntry_t *e = NvsConfig_FindParam(name);
    if (!e) {
        printf("Parameter '%s' not found\n", name);
        return 1;
    }

    esp_err_t rc = e->reset();
    if (rc == ESP_OK) {
        char buf[128];
        e->print(buf, sizeof(buf));
        printf("%s reset to %s\n", e->name, buf);
    } else {
        printf("%s already at default\n", e->name);
    }
    return 0;
}

/* ── param save ── */

static int cmd_param_save(int argc, char **argv)
{
    (void)argc; (void)argv;
    NvsConfig_SaveDirtyParameters();
    printf("Dirty parameters saved to flash\n");
    return 0;
}

/* ── param level [N] ── */

static struct {
    struct arg_int *level;
    struct arg_end *end;
} s_level_args;

static int cmd_param_level(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&s_level_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, s_level_args.end, argv[0]);
        return 1;
    }

    if (s_level_args.level->count == 0) {
        printf("Security level: %u\n", NvsConfig_SecureLevel());
        return 0;
    }

    uint8_t new_level = (uint8_t)s_level_args.level->ival[0];
    esp_err_t rc = NvsConfig_SecureLevelChange(new_level);
    if (rc == ESP_OK) {
        printf("Security level changed to %u\n", new_level);
    } else {
        printf("Failed: %s\n", esp_err_to_name(rc));
    }
    return (rc == ESP_OK) ? 0 : 1;
}

/* ── Registration ── */

esp_err_t NvsConfig_ConsoleInit(void)
{
    /* param list */
    const esp_console_cmd_t list_cmd = {
        .command = "param-list",
        .help = "List all parameters with current values and flags",
        .hint = NULL,
        .func = cmd_param_list,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&list_cmd));

    /* param get */
    s_get_args.name = arg_str1(NULL, NULL, "<name>", "Parameter name");
    s_get_args.end  = arg_end(1);
    const esp_console_cmd_t get_cmd = {
        .command = "param-get",
        .help = "Get a parameter's current value",
        .hint = NULL,
        .func = cmd_param_get,
        .argtable = &s_get_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&get_cmd));

    /* param set */
    s_set_args.name  = arg_str1(NULL, NULL, "<name>", "Parameter name");
    s_set_args.value = arg_str1(NULL, NULL, "<value>", "New value");
    s_set_args.end   = arg_end(2);
    const esp_console_cmd_t set_cmd = {
        .command = "param-set",
        .help = "Set a parameter value (scalars only)",
        .hint = NULL,
        .func = cmd_param_set,
        .argtable = &s_set_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_cmd));

    /* param reset */
    s_reset_args.name = arg_str1(NULL, NULL, "<name|all>", "Parameter name or 'all'");
    s_reset_args.end  = arg_end(1);
    const esp_console_cmd_t reset_cmd = {
        .command = "param-reset",
        .help = "Reset parameter to default (use 'all' for all params)",
        .hint = NULL,
        .func = cmd_param_reset,
        .argtable = &s_reset_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&reset_cmd));

    /* param save */
    const esp_console_cmd_t save_cmd = {
        .command = "param-save",
        .help = "Save dirty parameters to NVS flash",
        .hint = NULL,
        .func = cmd_param_save,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&save_cmd));

    /* param level */
    s_level_args.level = arg_int0(NULL, NULL, "[level]", "New security level (omit to query)");
    s_level_args.end   = arg_end(1);
    const esp_console_cmd_t level_cmd = {
        .command = "param-level",
        .help = "Get or set the security level",
        .hint = NULL,
        .func = cmd_param_level,
        .argtable = &s_level_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&level_cmd));

    ESP_LOGI(TAG, "NVS config console commands registered");
    return ESP_OK;
}
