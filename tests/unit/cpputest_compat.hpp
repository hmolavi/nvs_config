/**
 * @file cpputest_compat.hpp
 * @brief Maps the project's GoogleTest-style assertion macros to CppUTest equivalents.
 *
 * This lets the existing test files compile under CppUTest with minimal changes
 * (only TEST_F → TEST conversions and removal of linker anchors are needed).
 *
 * Macro argument order note:
 *   Original EXPECT_EQ(actual, expected) → CppUTest CHECK_EQUAL(expected, actual)
 */

#pragma once

#include <CppUTest/TestHarness.h>

/* Map TEST_F(fixture, name) → TEST(fixture, name) so test files work unchanged.
 * CppUTest TEST() uses the TEST_GROUP defined for that fixture as its setup/teardown. */
#define TEST_F(fixture, name) TEST(fixture, name)

/* Boolean assertions */
#define EXPECT_TRUE(expr)       CHECK(expr)
#define EXPECT_FALSE(expr)      CHECK(!(expr))

/* Equality / inequality */
#define EXPECT_EQ(a, b)         CHECK_EQUAL((b), (a))
#define EXPECT_NE(a, b)         CHECK((a) != (b))
#define EXPECT_GT(a, b)         CHECK((a) > (b))
#define EXPECT_GE(a, b)         CHECK((a) >= (b))

/* ESP error code assertions */
#define EXPECT_OK(expr)         CHECK_EQUAL(ESP_OK,  (expr))
#define EXPECT_ERR(expr, e)     CHECK_EQUAL((e),     (expr))

/* String and memory assertions */
#define EXPECT_STREQ(a, b)      STRCMP_EQUAL((b), (a))
#define EXPECT_MEMEQ(a, b, n)   MEMCMP_EQUAL((b), (a), (n))
