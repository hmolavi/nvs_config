#pragma once

#include <stdint.h>

/* Minimal FreeRTOS type and constant stubs for host-side compilation. */

typedef uint32_t TickType_t;
typedef int32_t  BaseType_t;
typedef uint32_t UBaseType_t;

#define pdTRUE   ((BaseType_t) 1)
#define pdFALSE  ((BaseType_t) 0)
#define pdPASS   pdTRUE
#define pdFAIL   pdFALSE

#define portMAX_DELAY ((TickType_t) 0xFFFFFFFFUL)

#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
