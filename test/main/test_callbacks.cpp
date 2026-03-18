/**
 * @file test_callbacks.cpp
 * @brief Unit tests for parameter change callback system.
 */

#include "test_helpers.hpp"
#include <cstring>

void register_callback_tests() {} // linker anchor

// ── Test state ──

static int s_global_cb_count = 0;
static const char* s_last_cb_name = nullptr;
static void* s_last_cb_userdata = nullptr;

static void global_callback(const char* name, void* user_data) {
    s_global_cb_count++;
    s_last_cb_name = name;
    s_last_cb_userdata = user_data;
}

static int s_per_param_count = 0;

static void per_param_callback(const char* name, void* user_data) {
    s_per_param_count++;
    s_last_cb_name = name;
    s_last_cb_userdata = user_data;
}

static int s_second_cb_count = 0;

static void second_callback(const char* name, void* user_data) {
    (void)name; (void)user_data;
    s_second_cb_count++;
}

// ── Fixture that resets callback state ──

struct CallbackFixture : NvsTestFixture {
    void SetUp() {
        NvsTestFixture::SetUp();
        NvsConfig_ClearCallbacks();
        s_global_cb_count = 0;
        s_per_param_count = 0;
        s_second_cb_count = 0;
        s_last_cb_name = nullptr;
        s_last_cb_userdata = nullptr;
    }
};

// ── Tests ──

TEST_F(CallbackFixture, GlobalCallbackFires) {
    EXPECT_OK(NvsConfig_RegisterGlobalOnChange(global_callback, nullptr));
    Param_SetLetter('Z');
    EXPECT_EQ(s_global_cb_count, 1);
    EXPECT_STREQ(s_last_cb_name, "Letter");
}

TEST_F(CallbackFixture, PerParamCallbackFires) {
    EXPECT_OK(NvsConfig_RegisterOnChange("Brightness", per_param_callback, nullptr));
    Param_SetBrightness(42);
    EXPECT_EQ(s_per_param_count, 1);
    EXPECT_STREQ(s_last_cb_name, "Brightness");
}

TEST_F(CallbackFixture, PerParamCallbackIgnoresOther) {
    EXPECT_OK(NvsConfig_RegisterOnChange("Brightness", per_param_callback, nullptr));
    Param_SetLetter('Z');
    EXPECT_EQ(s_per_param_count, 0);
}

TEST_F(CallbackFixture, CallbackReceivesUserData) {
    int marker = 42;
    EXPECT_OK(NvsConfig_RegisterGlobalOnChange(global_callback, &marker));
    Param_SetLetter('Z');
    EXPECT_EQ(s_last_cb_userdata, (void*)&marker);
}

TEST_F(CallbackFixture, NoCallbackOnSameValue) {
    EXPECT_OK(NvsConfig_RegisterGlobalOnChange(global_callback, nullptr));
    // Letter is already 'A' (default), setting same value returns ESP_FAIL
    Param_SetLetter('A');
    EXPECT_EQ(s_global_cb_count, 0);
}

TEST_F(CallbackFixture, MultipleCallbacks) {
    EXPECT_OK(NvsConfig_RegisterOnChange("Brightness", per_param_callback, nullptr));
    EXPECT_OK(NvsConfig_RegisterOnChange("Brightness", second_callback, nullptr));
    Param_SetBrightness(42);
    EXPECT_EQ(s_per_param_count, 1);
    EXPECT_EQ(s_second_cb_count, 1);
}

TEST_F(CallbackFixture, GlobalAndPerParamBothFire) {
    EXPECT_OK(NvsConfig_RegisterGlobalOnChange(global_callback, nullptr));
    EXPECT_OK(NvsConfig_RegisterOnChange("Brightness", per_param_callback, nullptr));
    Param_SetBrightness(42);
    EXPECT_EQ(s_global_cb_count, 1);
    EXPECT_EQ(s_per_param_count, 1);
}
