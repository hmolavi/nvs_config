/**
 * @file test_registry.cpp
 * @brief Unit tests for the parameter registry (vtable array).
 */

#include "test_helpers.hpp"
#include <cstring>

void register_registry_tests() {} // linker anchor

// ── Registry metadata ──

TEST_F(NvsTestFixture, RegistryCountMatchesExpected) {
    // 13 scalars + 6 arrays = 19 params in test param_table.inc
    EXPECT_EQ(g_nvsconfig_param_count, (size_t)19);
}

TEST_F(NvsTestFixture, RegistryFirstParamIsLetter) {
    EXPECT_STREQ(g_nvsconfig_params[0].name, "Letter");
}

TEST_F(NvsTestFixture, RegistryScalarIsArrayFalse) {
    EXPECT_FALSE(g_nvsconfig_params[0].is_array);
}

TEST_F(NvsTestFixture, RegistryArrayIsArrayTrue) {
    // DeviceName is the first array param (index 13)
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("DeviceName");
    EXPECT_TRUE(e != nullptr);
    EXPECT_TRUE(e->is_array);
}

TEST_F(NvsTestFixture, RegistrySecureLevelCorrect) {
    EXPECT_EQ(g_nvsconfig_params[0].secure_level, (uint8_t)0);
}

TEST_F(NvsTestFixture, RegistryDescriptionPopulated) {
    EXPECT_STREQ(g_nvsconfig_params[0].description, "char type");
}

// ── FindParam ──

TEST_F(NvsTestFixture, FindParamByName) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Brightness");
    EXPECT_TRUE(e != nullptr);
    EXPECT_STREQ(e->name, "Brightness");
}

TEST_F(NvsTestFixture, FindParamNotFound) {
    EXPECT_TRUE(NvsConfig_FindParam("NonExistent") == nullptr);
}

// ── Vtable accessors ──

TEST_F(NvsTestFixture, RegistryIsDirtyReflectsState) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Letter");
    EXPECT_TRUE(e != nullptr);
    // After fixture reset, save clears dirty
    NvsConfig_SaveDirtyParameters();
    EXPECT_FALSE(e->is_dirty());
    Param_SetLetter('Z');
    EXPECT_TRUE(e->is_dirty());
}

TEST_F(NvsTestFixture, RegistryIsDefaultReflectsState) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Letter");
    EXPECT_TRUE(e != nullptr);
    EXPECT_TRUE(e->is_default());
    Param_SetLetter('Z');
    EXPECT_FALSE(e->is_default());
    Param_ResetLetter();
    EXPECT_TRUE(e->is_default());
}

// ── Vtable operations ──

TEST_F(NvsTestFixture, RegistryResetViaVtable) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Brightness");
    EXPECT_TRUE(e != nullptr);
    Param_SetBrightness(42);
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)42);
    EXPECT_OK(e->reset());
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)255);
}

TEST_F(NvsTestFixture, RegistryPrintViaVtable) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Brightness");
    EXPECT_TRUE(e != nullptr);
    char buf[32];
    int n = e->print(buf, sizeof(buf));
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "255");
}

TEST_F(NvsTestFixture, RegistrySetScalarViaVtable) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Brightness");
    EXPECT_TRUE(e != nullptr);
    uint8_t val = 42;
    EXPECT_OK(e->set(&val, sizeof(val)));
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)42);
}

TEST_F(NvsTestFixture, RegistrySetScalarWrongSize) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("Brightness");
    EXPECT_TRUE(e != nullptr);
    uint16_t wrong = 42;
    EXPECT_ERR(e->set(&wrong, sizeof(wrong)), ESP_ERR_INVALID_SIZE);
}

TEST_F(NvsTestFixture, RegistrySetArrayViaVtable) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("RGBColor");
    EXPECT_TRUE(e != nullptr);
    const uint16_t rgb[3] = {100, 200, 50};
    EXPECT_OK(e->set(rgb, sizeof(rgb)));
    size_t len;
    const uint16_t* got = Param_GetRGBColor(&len);
    EXPECT_EQ(got[0], (uint16_t)100);
    EXPECT_EQ(got[1], (uint16_t)200);
    EXPECT_EQ(got[2], (uint16_t)50);
}

TEST_F(NvsTestFixture, RegistrySetArrayPartialWrite) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("RGBColor");
    EXPECT_TRUE(e != nullptr);
    // Send only 2 of 3 elements — remaining should be zeroed, returns warning
    const uint16_t partial[2] = {111, 222};
    EXPECT_ERR(e->set(partial, sizeof(partial)), ESP_ERR_INVALID_SIZE);
    size_t len;
    const uint16_t* got = Param_GetRGBColor(&len);
    EXPECT_EQ(got[0], (uint16_t)111);
    EXPECT_EQ(got[1], (uint16_t)222);
    EXPECT_EQ(got[2], (uint16_t)0);  // zero-filled
}

TEST_F(NvsTestFixture, RegistrySetArrayOversize) {
    const NvsConfigParamEntry_t* e = NvsConfig_FindParam("RGBColor");
    EXPECT_TRUE(e != nullptr);
    const uint16_t too_big[4] = {1, 2, 3, 4};
    EXPECT_ERR(e->set(too_big, sizeof(too_big)), ESP_ERR_INVALID_SIZE);
}

TEST_F(NvsTestFixture, RegistryElementSizeAndCount) {
    const NvsConfigParamEntry_t* scalar = NvsConfig_FindParam("Brightness");
    EXPECT_TRUE(scalar != nullptr);
    EXPECT_EQ(scalar->element_size, sizeof(uint8_t));
    EXPECT_EQ(scalar->element_count, (size_t)1);

    const NvsConfigParamEntry_t* array = NvsConfig_FindParam("RGBColor");
    EXPECT_TRUE(array != nullptr);
    EXPECT_EQ(array->element_size, sizeof(uint16_t));
    EXPECT_EQ(array->element_count, (size_t)3);
}

// ── Bulk operations ──

TEST_F(NvsTestFixture, ResetAllResetsEverything) {
    Param_SetLetter('Z');
    Param_SetBrightness(42);
    Param_SetTempReading(99.9f);
    NvsConfig_ResetAll();
    EXPECT_EQ(Param_GetLetter(), 'A');
    EXPECT_EQ(Param_GetBrightness(), (uint8_t)255);
    EXPECT_EQ(Param_GetTempReading(), -40.5f);
}

TEST_F(NvsTestFixture, PrintAllDoesNotCrash) {
    // Smoke test: just verify it doesn't crash
    NvsConfig_PrintAll();
    EXPECT_TRUE(true);
}
