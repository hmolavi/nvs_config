/**
 * @file stress_test_main.c
 * @brief Comprehensive stress test for the nvs_config library.
 *
 * Exercises every supported type (scalar + array), all security levels,
 * boundary values, print formatting, reset behaviour, rapid dirty-flag
 * updates, and cross-reboot persistence.
 *
 * Run once, then reboot to verify NVS persistence.
 */

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_config.h"

static const char *TAG = "STRESS";

/* ── tiny test harness ── */

static int pass_count = 0;
static int fail_count = 0;

static void check(const char *name, bool ok)
{
    if (ok) {
        ESP_LOGI(TAG, "  [PASS] %s", name);
        pass_count++;
    } else {
        ESP_LOGE(TAG, "  [FAIL] %s", name);
        fail_count++;
    }
}

/* ── Test 1: Verify every default value ── */

static void test_defaults(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Test 1: Default Values ---");

    /* scalars */
    check("Letter == 'A'",              Param_GetLetter() == 'A');
    check("AdminLock == false",          Param_GetAdminLock() == false);
    check("TinyOffset == -127",          Param_GetTinyOffset() == -127);
    check("Brightness == 255",           Param_GetBrightness() == 255);
    check("BigTimestamp == 1700000000",  Param_GetBigTimestamp() == 1700000000LL);
    check("DeviceUID == 0",              Param_GetDeviceUID() == 0ULL);
    check("Altitude == -32000",          Param_GetAltitude() == -32000);
    check("SampleRate == 60000",         Param_GetSampleRate() == 60000);
    check("CalibOffset == -2000000000",  Param_GetCalibOffset() == -2000000000);
    check("SerialNum == 4000000000",     Param_GetSerialNum() == 4000000000U);
    check("TempReading == -40.5",        Param_GetTempReading() == -40.5f);
    check("GpsLongitude == -122.419418", Param_GetGpsLongitude() == -122.419418);
    check("MaxNameTestXXX1 == 42",       Param_GetMaxNameTestXXX1() == 42U);

    /* arrays */
    size_t len;

    const char *dn = Param_GetDeviceName(&len);
    check("DeviceName len == 16",        len == 16);
    check("DeviceName == \"Stress\"",    strcmp(dn, "Stress") == 0);

    const uint8_t *bp = Param_GetBytePattern(&len);
    check("BytePattern len == 8",        len == 8);
    check("BytePattern[0] 0xDE",         bp[0] == 0xDE);
    check("BytePattern[7] 0xBE",         bp[7] == 0xBE);

    const int32_t *cp = Param_GetCalibPoints(&len);
    check("CalibPoints len == 6",        len == 6);
    check("CalibPoints[0] -1000",        cp[0] == -1000);
    check("CalibPoints[5] 2000",         cp[5] == 2000);

    const float *th = Param_GetThresholds(&len);
    check("Thresholds len == 4",         len == 4);
    check("Thresholds[0] 0.001",         th[0] == 0.001f);
    check("Thresholds[3] 9999.99",       th[3] == 9999.99f);

    const bool *ff = Param_GetFeatureFlags(&len);
    check("FeatureFlags len == 8",       len == 8);
    check("FeatureFlags[0] true",        ff[0] == true);
    check("FeatureFlags[1] false",       ff[1] == false);

    const uint16_t *rgb = Param_GetRGBColor(&len);
    check("RGBColor len == 3",           len == 3);
    check("RGBColor {255,128,0}",        rgb[0] == 255 && rgb[1] == 128 && rgb[2] == 0);
}

/* ── Test 2: Scalar set/get for every type + boundary values ── */

static void test_scalar_types(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Test 2: Scalar Set/Get (All Types) ---");

    check("Set char 'Z'",        Param_SetLetter('Z') == ESP_OK);
    check("Get char 'Z'",        Param_GetLetter() == 'Z');

    check("Set bool true",       Param_SetAdminLock(true) == ESP_OK);
    check("Get bool true",       Param_GetAdminLock() == true);

    check("Set int8 127",        Param_SetTinyOffset(127) == ESP_OK);
    check("Get int8 127",        Param_GetTinyOffset() == 127);

    check("Set uint8 0",         Param_SetBrightness(0) == ESP_OK);
    check("Get uint8 0",         Param_GetBrightness() == 0);

    check("Set int64 big",       Param_SetBigTimestamp(9999999999LL) == ESP_OK);
    check("Get int64 big",       Param_GetBigTimestamp() == 9999999999LL);

    check("Set uint64 max",      Param_SetDeviceUID(0xFFFFFFFFFFFFFFFFULL) == ESP_OK);
    check("Get uint64 max",      Param_GetDeviceUID() == 0xFFFFFFFFFFFFFFFFULL);

    check("Set int16 32767",     Param_SetAltitude(32767) == ESP_OK);
    check("Get int16 32767",     Param_GetAltitude() == 32767);

    check("Set uint16 0",        Param_SetSampleRate(0) == ESP_OK);
    check("Get uint16 0",        Param_GetSampleRate() == 0);

    check("Set int32 max",       Param_SetCalibOffset(2147483647) == ESP_OK);
    check("Get int32 max",       Param_GetCalibOffset() == 2147483647);

    check("Set uint32 0",        Param_SetSerialNum(0U) == ESP_OK);
    check("Get uint32 0",        Param_GetSerialNum() == 0U);

    check("Set float 99.9",      Param_SetTempReading(99.9f) == ESP_OK);
    check("Get float 99.9",      Param_GetTempReading() == 99.9f);

    check("Set double 179.99",   Param_SetGpsLongitude(179.999999) == ESP_OK);
    check("Get double 179.99",   Param_GetGpsLongitude() == 179.999999);

    check("Set 15-char param",   Param_SetMaxNameTestXXX1(0xDEADBEEFU) == ESP_OK);
    check("Get 15-char param",   Param_GetMaxNameTestXXX1() == 0xDEADBEEFU);

    /* setting the same value should return ESP_FAIL (no change) */
    check("Same val -> ESP_FAIL", Param_SetLetter('Z') == ESP_FAIL);
}

/* ── Test 3: Array operations ── */

static void test_array_ops(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Test 3: Array Operations ---");
    size_t len;

    /* char[16]: set, get, copy */
    char new_name[16] = {0};
    memcpy(new_name, "StressTest!", 11);
    check("Set DeviceName",            Param_SetDeviceName(new_name, 16) == ESP_OK);
    const char *dn = Param_GetDeviceName(&len);
    check("DeviceName matches",        strcmp(dn, "StressTest!") == 0);
    char name_copy[16] = {0};
    check("Copy DeviceName",           Param_CopyDeviceName(name_copy, 16) == ESP_OK);
    check("Copied name matches",       strcmp(name_copy, "StressTest!") == 0);

    /* uint8_t[8] */
    const uint8_t new_bp[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    check("Set BytePattern",           Param_SetBytePattern(new_bp, 8) == ESP_OK);
    const uint8_t *bp = Param_GetBytePattern(&len);
    check("BytePattern ok",            bp[0] == 0x01 && bp[7] == 0xEF);

    /* int32_t[6]: boundary values */
    const int32_t new_cp[6] = {-2147483647, -1, 0, 1, 100, 2147483647};
    check("Set CalibPoints",           Param_SetCalibPoints(new_cp, 6) == ESP_OK);
    const int32_t *cp = Param_GetCalibPoints(&len);
    check("CalibPoints boundaries",    cp[0] == -2147483647 && cp[5] == 2147483647);

    /* float[4] */
    const float new_th[4] = {-999.0f, 0.0f, 0.001f, 999999.0f};
    check("Set Thresholds",            Param_SetThresholds(new_th, 4) == ESP_OK);

    /* bool[8]: flip every bit */
    const bool flipped[8] = {false, true, false, true, false, true, false, true};
    check("Set FeatureFlags",          Param_SetFeatureFlags(flipped, 8) == ESP_OK);
    const bool *ff = Param_GetFeatureFlags(&len);
    check("FeatureFlags flipped",      ff[0] == false && ff[1] == true);

    /* uint16_t[3] */
    const uint16_t new_rgb[3] = {0, 65535, 32768};
    check("Set RGBColor",              Param_SetRGBColor(new_rgb, 3) == ESP_OK);
    const uint16_t *rgb = Param_GetRGBColor(&len);
    check("RGBColor[1] 65535",         rgb[1] == 65535);

    /* bounds: length > declared size */
    const uint8_t big[9] = {0};
    check("Oversized -> INVALID_SIZE", Param_SetBytePattern(big, 9) == ESP_ERR_INVALID_SIZE);

    /* copy into undersized buffer */
    uint8_t tiny[4];
    check("Small copy -> INVALID_SIZE", Param_CopyBytePattern(tiny, sizeof(tiny)) == ESP_ERR_INVALID_SIZE);

    /* reset one array and verify */
    check("Reset CalibPoints",         Param_ResetCalibPoints() == ESP_OK);
    cp = Param_GetCalibPoints(&len);
    check("CalibPoints back to default", cp[0] == -1000 && cp[5] == 2000);

    /* same-value set on array returns ESP_ERR_INVALID_ARG */
    check("Same array -> no change",   Param_SetCalibPoints(cp, 6) == ESP_ERR_INVALID_ARG);
}

/* ── Test 4: Print formatting for every parameter ── */

static void test_print(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Test 4: Print Formatting ---");
    char buf[128];
    int n;

    #define P(param) do {                                               \
        n = Param_Print##param(buf, sizeof(buf));                       \
        ESP_LOGI(TAG, "  %-18s = %s (len=%d)", #param, buf, n);        \
        check("Print " #param, n > 0);                                 \
    } while (0)

    /* scalars */
    P(Letter);
    P(AdminLock);
    P(TinyOffset);
    P(Brightness);
    P(BigTimestamp);
    P(DeviceUID);
    P(Altitude);
    P(SampleRate);
    P(CalibOffset);
    P(SerialNum);
    P(TempReading);
    P(GpsLongitude);
    P(MaxNameTestXXX1);

    /* arrays */
    P(DeviceName);
    P(BytePattern);
    P(CalibPoints);
    P(Thresholds);
    P(FeatureFlags);
    P(RGBColor);

    #undef P

    /* truncation: tiny buffer should not crash */
    n = Param_PrintCalibPoints(buf, 10);
    ESP_LOGI(TAG, "  Truncated (buf=10): '%s' (n=%d)", buf, n);
    check("Print truncation safe", n >= 0);
}

/* ── Test 5: Security level enforcement ── */

static void test_security(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Test 5: Security Levels ---");

    /* level 0 (admin): everything accessible */
    check("Start at level 0",     NvsConfig_SecureLevel() == 0);
    check("L0: set L0 OK",        Param_SetLetter('X') == ESP_OK);
    check("L0: set L1 OK",        Param_SetAltitude(100) == ESP_OK);
    check("L0: set L2 OK",        Param_SetTempReading(50.0f) == ESP_OK);

    /* L0 array across security boundaries */
    const bool all_on[8] = {true, true, true, true, true, true, true, true};
    check("L0: set L2 array OK",  Param_SetFeatureFlags(all_on, 8) == ESP_OK);

    /* level 1 (operator): L0 denied */
    check("Change to level 1",    NvsConfig_SecureLevelChange(1) == ESP_OK);
    check("Level is 1",           NvsConfig_SecureLevel() == 1);
    check("L1: L0 scalar DENIED", Param_SetLetter('Y') == ESP_ERR_INVALID_STATE);
    check("L1: L1 scalar OK",     Param_SetAltitude(200) == ESP_OK);
    check("L1: L2 scalar OK",     Param_SetTempReading(60.0f) == ESP_OK);

    char dummy[16] = {0};
    check("L1: L0 array DENIED",  Param_SetDeviceName(dummy, 16) == ESP_ERR_INVALID_STATE);
    const int32_t cp[6] = {1, 2, 3, 4, 5, 6};
    check("L1: L1 array OK",      Param_SetCalibPoints(cp, 6) == ESP_OK);

    /* level 2 (user): L0+L1 denied */
    check("Change to level 2",    NvsConfig_SecureLevelChange(2) == ESP_OK);
    check("Level is 2",           NvsConfig_SecureLevel() == 2);
    check("L2: L0 DENIED",        Param_SetLetter('Y') == ESP_ERR_INVALID_STATE);
    check("L2: L1 DENIED",        Param_SetAltitude(300) == ESP_ERR_INVALID_STATE);
    check("L2: L2 OK",            Param_SetTempReading(70.0f) == ESP_OK);

    /* invalid level */
    check("Invalid level 99",     NvsConfig_SecureLevelChange(99) != ESP_OK);

    /* restore admin */
    check("Restore level 0",      NvsConfig_SecureLevelChange(0) == ESP_OK);
    check("Confirmed level 0",    NvsConfig_SecureLevel() == 0);
}

/* ── Test 6: Reset and verify ── */

static void test_reset(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Test 6: Reset ---");

    check("Reset Letter",            Param_ResetLetter() == ESP_OK);
    check("Letter back to 'A'",      Param_GetLetter() == 'A');

    check("Reset AdminLock",         Param_ResetAdminLock() == ESP_OK);
    check("AdminLock back to false",  Param_GetAdminLock() == false);

    check("Reset Brightness",        Param_ResetBrightness() == ESP_OK);
    check("Brightness back to 255",   Param_GetBrightness() == 255);

    check("Reset BigTimestamp",       Param_ResetBigTimestamp() == ESP_OK);
    check("BigTimestamp default",     Param_GetBigTimestamp() == 1700000000LL);

    check("Reset DeviceUID",         Param_ResetDeviceUID() == ESP_OK);
    check("DeviceUID back to 0",      Param_GetDeviceUID() == 0ULL);

    /* array resets */
    check("Reset DeviceName",        Param_ResetDeviceName() == ESP_OK);
    size_t len;
    const char *dn = Param_GetDeviceName(&len);
    check("DeviceName back to Stress", strcmp(dn, "Stress") == 0);

    check("Reset BytePattern",       Param_ResetBytePattern() == ESP_OK);
    const uint8_t *bp = Param_GetBytePattern(&len);
    check("BytePattern[0] 0xDE",     bp[0] == 0xDE);

    /* double-reset: already at default → ESP_FAIL */
    check("Double reset scalar FAIL", Param_ResetLetter() == ESP_FAIL);
    check("Double reset array FAIL",  Param_ResetDeviceName() == ESP_FAIL);

    /* reset the remaining params for clean state */
    Param_ResetTinyOffset();
    Param_ResetAltitude();
    Param_ResetSampleRate();
    Param_ResetCalibOffset();
    Param_ResetSerialNum();
    Param_ResetTempReading();
    Param_ResetGpsLongitude();
    Param_ResetMaxNameTestXXX1();
    Param_ResetCalibPoints();
    Param_ResetThresholds();
    Param_ResetFeatureFlags();
    Param_ResetRGBColor();
}

/* ── Test 7: Rapid-fire dirty flag stress ── */

static void test_rapid(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Test 7: Rapid Updates (dirty flag stress) ---");

    /* 256 consecutive Brightness writes (0..255) */
    for (int i = 0; i < 256; i++) {
        Param_SetBrightness((uint8_t)i);
    }
    check("Brightness == 255 after 256 sets", Param_GetBrightness() == 255);

    /* 1000 boolean toggles */
    for (int i = 0; i < 1000; i++) {
        Param_SetAdminLock(i % 2 == 0);
    }
    check("AdminLock after 1000 toggles", Param_GetAdminLock() == false);

    /* 500 float writes */
    for (int i = 0; i < 500; i++) {
        Param_SetTempReading((float)i * 0.1f);
    }
    check("TempReading > 0 after 500 sets", Param_GetTempReading() > 0.0f);

    /* explicit save */
    NvsConfig_SaveDirtyParameters();
    ESP_LOGI(TAG, "  Saved all dirty parameters to NVS");
}

/* ── Reset every parameter back to default ── */

static void reset_all(void)
{
    Param_ResetLetter();
    Param_ResetAdminLock();
    Param_ResetTinyOffset();
    Param_ResetBrightness();
    Param_ResetBigTimestamp();
    Param_ResetDeviceUID();
    Param_ResetAltitude();
    Param_ResetSampleRate();
    Param_ResetCalibOffset();
    Param_ResetSerialNum();
    Param_ResetTempReading();
    Param_ResetGpsLongitude();
    Param_ResetMaxNameTestXXX1();
    Param_ResetDeviceName();
    Param_ResetBytePattern();
    Param_ResetCalibPoints();
    Param_ResetThresholds();
    Param_ResetFeatureFlags();
    Param_ResetRGBColor();
    NvsConfig_SaveDirtyParameters();
}

/* ── Entry point ── */

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "    NVS Config Stress Test");
    ESP_LOGI(TAG, "========================================");

    esp_err_t rc = NvsConfig_Init();
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "NvsConfig_Init FAILED (0x%x) — aborting", rc);
        return;
    }
    ESP_LOGI(TAG, "NvsConfig_Init: OK");

    /* ── Persistence check (from a prior boot) ── */
    bool first_boot = (Param_GetLetter() == 'A' && Param_GetBrightness() == 255);
    if (!first_boot) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "--- Persistence Verification (reboot detected) ---");
        check("Letter persisted as 'Z'",    Param_GetLetter() == 'Z');
        check("Brightness persisted as 42", Param_GetBrightness() == 42);
    } else {
        ESP_LOGI(TAG, "First boot detected (defaults loaded)");
    }

    /* reset to clean state so all tests start from known defaults */
    reset_all();

    /* security level starts at highest (most restricted) by design;
       elevate to admin so tests can modify all params */
    NvsConfig_SecureLevelChange(0);

    /* ── Run test suites ── */
    test_defaults();
    test_scalar_types();
    test_array_ops();
    test_print();
    test_security();
    test_reset();
    test_rapid();

    /* ── Set persistence markers for next boot ── */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- Persistence Setup ---");
    /* reset first so markers are the only dirty params */
    reset_all();
    Param_SetLetter('Z');
    Param_SetBrightness(42);
    NvsConfig_SaveDirtyParameters();
    ESP_LOGI(TAG, "  Markers set: Letter='Z', Brightness=42");
    ESP_LOGI(TAG, "  Reboot device to verify NVS persistence");

    /* ── Summary ── */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Results: %d passed, %d failed", pass_count, fail_count);
    if (fail_count == 0) {
        ESP_LOGI(TAG, "  ALL TESTS PASSED");
    } else {
        ESP_LOGE(TAG, "  SOME TESTS FAILED");
    }
    ESP_LOGI(TAG, "========================================");
}
