[![Component Registry](https://components.espressif.com/components/hmolavi/nvs_config/badge.svg)](https://components.espressif.com/components/hmolavi/nvs_config) [![Espressif](https://img.shields.io/badge/Espressif-Components-blue.svg?style=flat-square)](https://components.espressif.com/components/hmolavi/nvs_config) [![GitHub](https://img.shields.io/badge/GitHub-hmolavi/nvs_config-blue.svg?style=flat-square)](https://github.com/hmolavi/nvs_config)

# NVS Config Library

An ESP-IDF component that simplifies the creation, management, and persistence of application parameters using Non-Volatile Storage (NVS). Define your parameters once in a declarative table and the library generates type-safe getters, setters, reset, and print functions at compile time via C macro expansion.

![block_diagram](/assets/nvs_config_block_diagram.png)

## Features

- **Declarative Parameter Table**  
  &nbsp;&nbsp;&nbsp;Define parameters with `PARAM` & `ARRAY` macros and you nvs_config generates all boilerplate code at compile time
- **Thread-Safe Access**  
  &nbsp;&nbsp;&nbsp;All NVS operations are protected by a FreeRTOS mutex
- **Parameter Registry**  
  &nbsp;&nbsp;&nbsp;Runtime vtable (`g_nvsconfig_params[]`) enables generic iteration, lookup by name, and polymorphic operations without knowing concrete types
- **Interactive UART Console**  
  &nbsp;&nbsp;&nbsp;Optional ESP-IDF console commands (`param-list`, `param-get`, `param-set`, `param-reset`, `param-save`, `param-level`)
- **Change Callbacks**  
  &nbsp;&nbsp;&nbsp;Register per-parameter or global callbacks that fire when values change
- **Role-based Security Levels**  
  &nbsp;&nbsp;&nbsp;Assign a security level to each parameter and restrict writes depending on access control
- **Wear-Level Tracking**  
  &nbsp;&nbsp;&nbsp;Per-parameter write counters to monitor flash wear
- **Schema Versioning**  
  &nbsp;&nbsp;&nbsp;Detects parameter table changes across firmware updates & registers migration callbacks

## Getting Started

### 1. Add the Component

```bash
idf.py add-dependency "hmolavi/nvs-config"
```

### 2. Create `param_table.inc`

Place this file in your project's `main/` directory:

```c
#define ARRAY_INIT(...) {__VA_ARGS__}

#ifndef SECURE_LEVEL
#define SECURE_LEVEL(secure_level, description)
#endif
#ifndef PARAM
#define PARAM(secure_level, type, name, default, description)
#endif
#ifndef ARRAY
#define ARRAY(secure_level, type, size, name, default, description)
#endif

SECURE_LEVEL(0, "Admin")
SECURE_LEVEL(1, "User")

PARAM(0, uint8_t, Brightness, 128, "Display brightness 0-255")
PARAM(1, float,   Threshold,  25.0, "Alert temperature threshold")
ARRAY(0, uint8_t, 4, IpAddr, ARRAY_INIT(192, 168, 1, 100), "Static IP")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL
```

> Parameter names must be **15 characters or fewer** (NVS key size limit).

### 3. Use in Code

```c
#include "nvs_config.h"

void app_main(void)
{
    NvsConfig_Init();

    /* Type-safe generated API */
    uint8_t brightness = Param_GetBrightness();
    Param_SetBrightness(200);
    Param_ResetBrightness();

    /* Generic registry API */
    const NvsConfigParamEntry_t *entry = NvsConfig_FindParam("Brightness");
    char buf[32];
    entry->print(buf, sizeof(buf));

    /* Iterate all parameters */
    for (size_t i = 0; i < g_nvsconfig_param_count; i++) {
        g_nvsconfig_params[i].print(buf, sizeof(buf));
        printf("%s = %s\n", g_nvsconfig_params[i].name, buf);
    }
}
```

## Supported Types

| Scalar Types          | Array Types                 |
| --------------------- | --------------------------- |
| `char`, `bool`        | `char[N]` (strings)         |
| `int8_t`, `uint8_t`   | `uint8_t[N]`, `int8_t[N]`   |
| `int16_t`, `uint16_t` | `uint16_t[N]`, `int16_t[N]` |
| `int32_t`, `uint32_t` | `int32_t[N]`, `uint32_t[N]` |
| `int64_t`, `uint64_t` | `float[N]`, `double[N]`     |
| `float`, `double`     | `bool[N]`                   |

## Examples

Each example is a complete buildable ESP-IDF project. See the README in each folder for details.

| Example                                          | Description                                                            |
| ------------------------------------------------ | ---------------------------------------------------------------------- |
| [basic_usage](examples/basic_usage/)             | A single counter that persists across reboots                          |
| [array_example](examples/array_example/)         | Array parameter get, set, copy, and reset                              |
| [console_demo](examples/console_demo/)           | Interactive UART shell for parameter management                        |
| [registry_iterator](examples/registry_iterator/) | Generic iteration and type-agnostic operations via the registry vtable |
| [change_callbacks](examples/change_callbacks/)   | Per-parameter and global change notification callbacks                 |
| [security_levels](examples/security_levels/)     | Role-based access control (admin / technician / user)                  |

```bash
cd examples/console_demo
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

## Architecture

The core pattern is that `param_table.inc` is `#include`-ed multiple times with different macro definitions to generate different artifacts:

| Pass | Purpose                                                                          |
| ---- | -------------------------------------------------------------------------------- |
| 1    | Struct members inside `NvsConfigMasterController_t`                              |
| 2    | Function prototypes (`Param_Set*`, `Param_Get*`, `Param_Reset*`, `Param_Print*`) |
| 3    | Compile-time `_Static_assert` on array initializer counts                        |
| 4    | Default value initialization                                                     |
| 5    | Thread-safe getter/setter/reset/print function bodies                            |
| 6    | Registry wrapper functions (vtable delegates)                                    |
| 7    | `const NvsConfigParamEntry_t g_nvsconfig_params[]` array                         |
| 8    | NVS load/save logic                                                              |

## API Reference

See [API.md](API.md) for the full API reference, including all function signatures, struct definitions, and detailed parameter descriptions.

## Testing

Tests are in the `tests/` folder:

- `tests/unit/`: 163 host-based unit tests (fast, no board needed)
- `tests/hardware/`: 4 on-device tests (real FreeRTOS concurrency)

The unit tests use CppUTest with gcov/lcov coverage.

Current unit test code coverage:  
![code_coverage](/assets/2026_3_20_code_coverage.png)

To run the unit tests:

```bash
# From the project directory:
tests/unit/run_unit_tests.sh
```

To run the hardware tests:

```bash
# From the project directory & from ESP-IDF terminal:
cd tests/hardware/
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

For more details, see [tests/README.md](tests/README.md).

## Compatibility

Compatible with ESP-IDF v4.4+ and v5.x. Tested on ESP32 and ESP32-S3.

## License

[MIT License](LICENSE)

## Support

For issues and contributions: [github.com/hmolavi/nvs_config](https://github.com/hmolavi/nvs_config)
