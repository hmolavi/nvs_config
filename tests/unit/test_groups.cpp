/**
 * @file test_groups.cpp
 * @brief Provides the single definition of externTestGroupNvsTestFixture.
 *
 * CppUTest's TEST_GROUP macro emits `int externTestGroup<Name> = 0;` which
 * must appear in exactly one translation unit.  The matching struct definition
 * lives in test_helpers.hpp (shared header) where it is ODR-safe.
 */

#include "test_helpers.hpp"

int externTestGroupNvsTestFixture = 0;
