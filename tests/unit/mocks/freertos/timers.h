#pragma once

/* Minimal FreeRTOS timer stubs.
 * These symbols must exist for compilation but are never called in the
 * unit test build because ESP_IDF_VERSION_MAJOR >= 5 selects esp_timer. */

#include "FreeRTOS.h"

typedef void* TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

#ifdef __cplusplus
extern "C" {
#endif

TimerHandle_t xTimerCreate(const char* name, TickType_t period,
                           UBaseType_t auto_reload, void* id,
                           TimerCallbackFunction_t cb);
BaseType_t    xTimerStart(TimerHandle_t timer, TickType_t ticks);

#ifdef __cplusplus
}
#endif
