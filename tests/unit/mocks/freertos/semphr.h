#pragma once

#include "FreeRTOS.h"
#include <stdlib.h>

typedef void* SemaphoreHandle_t;
typedef void* QueueHandle_t;

#ifdef __cplusplus
extern "C" {
#endif

SemaphoreHandle_t xSemaphoreCreateMutex(void);
BaseType_t        xSemaphoreTake(SemaphoreHandle_t sem, TickType_t ticks);
BaseType_t        xSemaphoreGive(SemaphoreHandle_t sem);
void              vSemaphoreDelete(SemaphoreHandle_t sem);

/* Counting semaphore - not used in unit tests but present in freertos API */
SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t max, UBaseType_t initial);

#ifdef __cplusplus
}
#endif
