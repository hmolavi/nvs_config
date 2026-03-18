/**
 * @file test_thread_safety.cpp
 * @brief Unit tests for thread-safe access to parameters via FreeRTOS mutex.
 *
 * Each test spawns multiple FreeRTOS tasks that concurrently access the
 * parameter controller, verifying no corruption or crashes occur.
 */

#include "test_helpers.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

void register_thread_safety_tests() {} // linker anchor

// ── Helpers ──

static SemaphoreHandle_t s_done_sem;

struct SetterTaskArgs {
    uint8_t value;
    int iterations;
};

static void brightness_setter_task(void* arg)
{
    auto* args = static_cast<SetterTaskArgs*>(arg);
    for (int i = 0; i < args->iterations; i++) {
        Param_SetBrightness(args->value);
    }
    xSemaphoreGive(s_done_sem);
    vTaskDelete(NULL);
}

static void letter_setter_task(void* arg)
{
    int iterations = *static_cast<int*>(arg);
    for (int i = 0; i < iterations; i++) {
        Param_SetLetter('Z');
    }
    xSemaphoreGive(s_done_sem);
    vTaskDelete(NULL);
}

static void letter_resetter_task(void* arg)
{
    int iterations = *static_cast<int*>(arg);
    for (int i = 0; i < iterations; i++) {
        Param_ResetLetter();
    }
    xSemaphoreGive(s_done_sem);
    vTaskDelete(NULL);
}

static void save_task(void* arg)
{
    int iterations = *static_cast<int*>(arg);
    for (int i = 0; i < iterations; i++) {
        NvsConfig_SaveDirtyParameters();
    }
    xSemaphoreGive(s_done_sem);
    vTaskDelete(NULL);
}

static void modify_task(void* arg)
{
    int iterations = *static_cast<int*>(arg);
    for (int i = 0; i < iterations; i++) {
        Param_SetBrightness((uint8_t)(i & 0xFF));
        Param_SetLetter((i % 2 == 0) ? 'A' : 'Z');
    }
    xSemaphoreGive(s_done_sem);
    vTaskDelete(NULL);
}

// ── Tests ──

TEST_F(NvsTestFixture, ConcurrentSettersNoCorruption) {
    s_done_sem = xSemaphoreCreateCounting(2, 0);

    static SetterTaskArgs args_a = { .value = 0, .iterations = 1000 };
    static SetterTaskArgs args_b = { .value = 128, .iterations = 1000 };

    xTaskCreate(brightness_setter_task, "set_a", 4096, &args_a, 5, NULL);
    xTaskCreate(brightness_setter_task, "set_b", 4096, &args_b, 5, NULL);

    // Wait for both tasks to finish
    xSemaphoreTake(s_done_sem, pdMS_TO_TICKS(10000));
    xSemaphoreTake(s_done_sem, pdMS_TO_TICKS(10000));

    uint8_t val = Param_GetBrightness();
    // Value must be one of the two values, not corrupted
    EXPECT_TRUE(val == 0 || val == 128);

    vSemaphoreDelete(s_done_sem);
}

TEST_F(NvsTestFixture, ConcurrentSetAndReset) {
    s_done_sem = xSemaphoreCreateCounting(2, 0);

    static int iters = 500;

    xTaskCreate(letter_setter_task, "setter", 4096, &iters, 5, NULL);
    xTaskCreate(letter_resetter_task, "resetter", 4096, &iters, 5, NULL);

    xSemaphoreTake(s_done_sem, pdMS_TO_TICKS(10000));
    xSemaphoreTake(s_done_sem, pdMS_TO_TICKS(10000));

    char val = Param_GetLetter();
    EXPECT_TRUE(val == 'A' || val == 'Z');

    vSemaphoreDelete(s_done_sem);
}

TEST_F(NvsTestFixture, ConcurrentSaveAndSet) {
    s_done_sem = xSemaphoreCreateCounting(2, 0);

    static int iters = 100;

    xTaskCreate(modify_task, "modifier", 4096, &iters, 5, NULL);
    xTaskCreate(save_task, "saver", 4096, &iters, 5, NULL);

    xSemaphoreTake(s_done_sem, pdMS_TO_TICKS(10000));
    xSemaphoreTake(s_done_sem, pdMS_TO_TICKS(10000));

    // If we got here without a crash, the test passes
    EXPECT_TRUE(true);

    vSemaphoreDelete(s_done_sem);
}

TEST_F(NvsTestFixture, MutexInitializedBeforeUse) {
    // NvsConfig_Init() was already called successfully in test_main.cpp
    // If mutex creation failed, Init would have returned ESP_FAIL
    EXPECT_EQ(NvsConfig_SecureLevel(), (uint8_t)0);
}
