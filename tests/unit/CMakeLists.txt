cmake_minimum_required(VERSION 3.16)
project(nvs_config_unit_tests CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD   11)

# ── Locate CppUTest (installed via: brew install cpputest) ──────────────────
find_package(CppUTest REQUIRED)

# ── Source paths ────────────────────────────────────────────────────────────
set(NVS_CONFIG_ROOT ${CMAKE_SOURCE_DIR}/../..)
set(MOCK_DIR        ${CMAKE_SOURCE_DIR}/mocks)

# ── Test executable ─────────────────────────────────────────────────────────
add_executable(unit_tests
    test_main.cpp
    test_groups.cpp
    test_scalar.cpp
    test_array.cpp
    test_security.cpp
    test_print.cpp
    test_edge_cases.cpp
    test_registry.cpp
    test_callbacks.cpp
    test_wear_level.cpp
    test_versioning.cpp
    test_console.cpp
    test_init_and_save.cpp
    ${NVS_CONFIG_ROOT}/src/nvs_config.c
    ${NVS_CONFIG_ROOT}/src/secure_level.c
    mocks/mock_impl.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}           # test_helpers.hpp, cpputest_compat.hpp, param_table.inc
    ${MOCK_DIR}                   # replaces all ESP-IDF headers
    ${NVS_CONFIG_ROOT}/include    # nvs_config.h
    ${CppUTest_INCLUDE_DIRS}
)

# Define IDF version so nvs_config.c uses the esp_timer path (not FreeRTOS timers)
target_compile_definitions(unit_tests PRIVATE
    ESP_IDF_VERSION_MAJOR=5
)

# ── Compile flags ───────────────────────────────────────────────────────────
target_compile_options(unit_tests PRIVATE
    --coverage
    -g
    -O0
    -Wall
    -Wno-unused-parameter
)

target_link_options(unit_tests PRIVATE
    --coverage
)

target_link_libraries(unit_tests
    ${CppUTest_LIBRARIES}
)

# ── Coverage target ──────────────────────────────────────────────────────────
# Run: cd build && make coverage
# Then: open coverage_html/index.html
add_custom_target(coverage
    COMMAND ${CMAKE_BINARY_DIR}/unit_tests
    COMMAND lcov
            --capture
            --directory ${CMAKE_BINARY_DIR}
            --output-file coverage.info
            --branch-coverage
            --ignore-errors inconsistent
            --ignore-errors unsupported
            --ignore-errors deprecated
            --ignore-errors format
            --ignore-errors gcov
    COMMAND lcov
            --remove coverage.info
            "${MOCK_DIR}/*"
            "/usr/*"
            "${CppUTest_INCLUDE_DIRS}/*"
            "${CMAKE_SOURCE_DIR}/*.cpp"
            "${CMAKE_SOURCE_DIR}/*.hpp"
            "${CMAKE_SOURCE_DIR}/*.inc"
            "format.inc"
            --output-file coverage_filtered.info
            --branch-coverage
            --ignore-errors inconsistent
            --ignore-errors deprecated
            --ignore-errors format
            --ignore-errors unused
    COMMAND genhtml
            coverage_filtered.info
            --output-directory coverage_html
            --branch-coverage
            --ignore-errors inconsistent
            --ignore-errors corrupt
            --ignore-errors category
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating HTML coverage report in build/coverage_html/"
)
