#pragma once

#include "esp_err.h"

typedef void* esp_timer_handle_t;

typedef enum {
    ESP_TIMER_TASK,
    ESP_TIMER_ISR,
} esp_timer_dispatch_t;

typedef struct {
    void (*callback)(void* arg);
    void* arg;
    esp_timer_dispatch_t dispatch_method;
    const char* name;
    bool skip_unhandled_events;
} esp_timer_create_args_t;

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t esp_timer_create(const esp_timer_create_args_t* create_args,
                           esp_timer_handle_t* out_handle);
esp_err_t esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period_us);

#ifdef __cplusplus
}
#endif
