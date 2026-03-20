# nvs_config Test Suites

Two independent test suites, each with a distinct purpose, toolchain, and location.

| Suite        | Location          | Runs on      | Tests | Coverage        |
| ------------ | ----------------- | ------------ | ----- | --------------- |
| **Unit**     | `tests/unit/`     | local (host) | 163   | Yes (gcov/lcov) |
| **Hardware** | `tests/hardware/` | ESP32        | 4     | No              |

---

## Unit Tests (`tests/unit/`)

Tests all parameter logic (get/set/reset/print, security, callbacks, wear tracking, registry, versioning) using CppUTest on your host machine. ESP-IDF APIs are replaced by thin stubs in `mocks/`, so no hardware is required.

### Prerequisites

Install once:

```bash
brew install cpputest lcov
```

### Build and Run

```bash
cd tests/unit
./run_unit_tests.sh
```

---

## Hardware Tests (`tests/hardware/`)

Tests that require a real FreeRTOS scheduler - concurrent task access via `xTaskCreate`. These exercise the mutex implementation under actual preemption, which cannot be replicated on the host.

### Prerequisites

- ESP-IDF v5.x installed and sourced (`source $IDF_PATH/export.sh`)
- ESP32-S3 connected via USB

### Build, Flash, Monitor

```bash
cd tests/hardware
idf.py set-target esp32s3   # first time only
idf.py build flash monitor
```

Expected serial output:

```
NVS Config - Hardware Test Suite (4 tests)

--- NvsTestFixture ---
  [PASS] ConcurrentSettersNoCorruption (1 assertions)
  [PASS] ConcurrentSetAndReset (1 assertions)
  [PASS] ConcurrentSaveAndSet (1 assertions)
  [PASS] MutexInitializedBeforeUse (1 assertions)

========================================
  4/4 tests passed (4 assertions)
  ALL TESTS PASSED
========================================
```

### Changing the target chip

```bash
idf.py set-target esp32   # or esp32c3, esp32s2, etc.
idf.py build flash monitor
```

---

## Test File Ownership

| File                     | Suite    | What it tests                                         |
| ------------------------ | -------- | ----------------------------------------------------- |
| `test_scalar.cpp`        | Unit     | All scalar types: set/get/reset                       |
| `test_array.cpp`         | Unit     | All array types: set/get/copy/reset                   |
| `test_security.cpp`      | Unit     | Security level enforcement                            |
| `test_print.cpp`         | Unit     | Print formatting for every type                       |
| `test_edge_cases.cpp`    | Unit     | Boundary values, rapid writes, dirty flags            |
| `test_registry.cpp`      | Unit     | Registry vtable, FindParam, bulk ops                  |
| `test_callbacks.cpp`     | Unit     | Per-param and global change callbacks                 |
| `test_wear_level.cpp`    | Unit     | Per-parameter write count tracking                    |
| `test_versioning.cpp`    | Unit     | Schema version read-back                              |
| `test_init_and_save.cpp` | Unit     | Init/save paths, NVS errors, migration callback paths |
| `test_console.cpp`       | Unit     | Generic `set(void*, size)` API                        |
| `test_groups.cpp`        | Unit     | Shared CppUTest group symbol definition               |
| `test_main.cpp`          | Unit     | Unit test runner entry point                          |
| `test_thread_safety.cpp` | Hardware | Concurrent task access under real RTOS                |
