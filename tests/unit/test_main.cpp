/**
 * @file test_main.cpp
 * @brief CppUTest entry point for the nvs_config unit test suite.
 *
 * Initialises the nvs_config library (using mock ESP-IDF stubs so all
 * parameters load from their compiled-in defaults) then hands control to
 * the CppUTest runner.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/unit_tests           # run all 149 tests
 *   ./build/unit_tests -ojunit   # also emit JUnit XML
 *
 * Coverage:
 *   cd build && make coverage
 *   open coverage_html/index.html
 */

#include <CppUTest/CommandLineTestRunner.h>
#include "nvs_config.h"

int main(int argc, char** argv)
{
    /* Initialise with mock stubs — nvs_get_blob returns NOT_FOUND so every
     * parameter loads from its compiled-in default value.               */
    NvsConfig_Init();

    /* Elevate to admin so fixture setup() can call Param_Reset* freely. */
    NvsConfig_SecureLevelChange(0);

    return CommandLineTestRunner::RunAllTests(argc, argv);
}
