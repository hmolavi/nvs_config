/**
 * @file test_framework.hpp
 * @brief Lightweight C++ unit test framework for ESP-IDF targets.
 *
 * Provides GoogleTest-style TEST() and TEST_F() macros with automatic
 * registration, assertion macros, and a test runner that prints results
 * to the serial monitor.
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <functional>
#include <vector>

#include "esp_log.h"
#include "esp_err.h"

namespace nvs_test {

struct TestCase {
    const char* suite;
    const char* name;
    std::function<void()> body;
};

class TestRunner {
public:
    static TestRunner& get() {
        static TestRunner instance;
        return instance;
    }

    void add(const char* suite, const char* name, std::function<void()> body) {
        tests_.push_back({suite, name, std::move(body)});
    }

    int run_all() {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "========================================");
        ESP_LOGI(TAG, "  Running %d tests", (int)tests_.size());
        ESP_LOGI(TAG, "========================================");

        const char* current_suite = "";

        for (auto& tc : tests_) {
            if (strcmp(current_suite, tc.suite) != 0) {
                current_suite = tc.suite;
                ESP_LOGI(TAG, "");
                ESP_LOGI(TAG, "--- %s ---", current_suite);
            }

            current_failed_ = false;
            current_assertions_ = 0;

            tc.body();

            if (!current_failed_) {
                ESP_LOGI(TAG, "  [PASS] %s (%d assertions)", tc.name, current_assertions_);
                pass_count_++;
            } else {
                ESP_LOGE(TAG, "  [FAIL] %s", tc.name);
                fail_count_++;
            }
        }

        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "========================================");
        ESP_LOGI(TAG, "  %d/%d tests passed (%d assertions)",
                 pass_count_, (int)tests_.size(), total_assertions_);
        if (fail_count_ == 0) {
            ESP_LOGI(TAG, "  ALL TESTS PASSED");
        } else {
            ESP_LOGE(TAG, "  %d TESTS FAILED", fail_count_);
        }
        ESP_LOGI(TAG, "========================================");
        return fail_count_;
    }

    void assert_pass() {
        current_assertions_++;
        total_assertions_++;
    }

    void assert_fail(const char* expr, const char* file, int line) {
        current_assertions_++;
        total_assertions_++;
        current_failed_ = true;
        // Strip path to just filename for cleaner output
        const char* fname = strrchr(file, '/');
        fname = fname ? fname + 1 : file;
        ESP_LOGE(TAG, "    ASSERT FAILED: %s (%s:%d)", expr, fname, line);
    }

private:
    TestRunner() = default;
    static constexpr const char* TAG = "TEST";
    std::vector<TestCase> tests_;
    bool current_failed_ = false;
    int current_assertions_ = 0;
    int pass_count_ = 0;
    int fail_count_ = 0;
    int total_assertions_ = 0;
};

// Auto-registration helper — constructor adds test to runner at static init time
struct Registrar {
    Registrar(const char* suite, const char* name, std::function<void()> body) {
        TestRunner::get().add(suite, name, std::move(body));
    }
};

} // namespace nvs_test

// ── Test declaration macros ──

// Standalone test (no fixture)
#define TEST(suite, name) \
    static void _test_body_##suite##_##name(); \
    static nvs_test::Registrar _reg_##suite##_##name( \
        #suite, #name, _test_body_##suite##_##name); \
    static void _test_body_##suite##_##name()

// Test with fixture class (must define SetUp() and TearDown())
#define TEST_F(fixture, name) \
    struct _test_##fixture##_##name : fixture { void Run(); }; \
    static nvs_test::Registrar _reg_##fixture##_##name( \
        #fixture, #name, []() { \
            _test_##fixture##_##name t; t.SetUp(); t.Run(); t.TearDown(); \
        }); \
    void _test_##fixture##_##name::Run()

// ── Assertion macros ──

#define EXPECT_TRUE(expr) do { \
    if (expr) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#expr, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_FALSE(expr) do { \
    if (!(expr)) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail("!" #expr, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_EQ(a, b) do { \
    auto _va = (a); auto _vb = (b); \
    if (_va == _vb) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#a " == " #b, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_NE(a, b) do { \
    auto _va = (a); auto _vb = (b); \
    if (_va != _vb) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#a " != " #b, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_GT(a, b) do { \
    auto _va = (a); auto _vb = (b); \
    if (_va > _vb) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#a " > " #b, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_GE(a, b) do { \
    auto _va = (a); auto _vb = (b); \
    if (_va >= _vb) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#a " >= " #b, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_OK(expr) do { \
    esp_err_t _rc = (expr); \
    if (_rc == ESP_OK) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#expr " == ESP_OK", __FILE__, __LINE__); } \
} while(0)

#define EXPECT_ERR(expr, err) do { \
    esp_err_t _rc = (expr); \
    esp_err_t _expected = (err); \
    if (_rc == _expected) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#expr " == " #err, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_STREQ(a, b) do { \
    const char* _sa = (a); const char* _sb = (b); \
    if (_sa && _sb && strcmp(_sa, _sb) == 0) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#a " streq " #b, __FILE__, __LINE__); } \
} while(0)

#define EXPECT_MEMEQ(a, b, n) do { \
    if (memcmp((a), (b), (n)) == 0) { nvs_test::TestRunner::get().assert_pass(); } \
    else { nvs_test::TestRunner::get().assert_fail(#a " memeq " #b, __FILE__, __LINE__); } \
} while(0)
