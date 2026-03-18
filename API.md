# API Reference

## Header Files

- [nvs_config.h](#file-nvs_configh)
- [nvs_config_console.h](#file-nvs_config_consoleh)

## File: nvs_config.h

This header provides the interface for the NVS Storage Config Library component.

## Core Functions

|      Type | Name                                                                                                                                                                  |
| --------: | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| esp_err_t | [**NvsConfig_Init**](#function-nvsconfig_init)(void) <br>_Initializes NVS flash storage and configuration parameters._                                                |
|      void | [**NvsConfig_SaveDirtyParameters**](#function-nvsconfig_savedirtyparameters)(void) <br>_Saves modified parameters to NVS flash._                                      |
|   uint8_t | [**NvsConfig_SecureLevel**](#function-nvsconfig_securelevel)(void) <br>_Retrieves the current security level._                                                        |
| esp_err_t | [**NvsConfig_SecureLevelChange**](#function-nvsconfig_securelevelchange)(uint8_t new_secure_level) <br>_Changes the current security level and logs the transition._   |

---

### function `NvsConfig_Init`

Initializes NVS flash storage and loads configuration parameters from flash (or sets defaults). Creates the thread-safety mutex and starts a periodic FreeRTOS timer (every 30 seconds) to commit any changes. Also checks the schema version and invokes the migration callback if a mismatch is detected.

```c
esp_err_t NvsConfig_Init(void);
```

**Returns:**
ESP_OK if initialization is successful; otherwise, ESP_FAIL.

---

### function `NvsConfig_SaveDirtyParameters`

Iterates through all parameters, saving modified ("dirty") parameters to NVS flash and committing the changes. Thread-safe — acquires the internal mutex.

```c
void NvsConfig_SaveDirtyParameters(void);
```

---

### function `NvsConfig_SecureLevel`

Retrieves the current security level for parameter access. The security level is atomic and safe to read from any task.

```c
uint8_t NvsConfig_SecureLevel(void);
```

---

### function `NvsConfig_SecureLevelChange`

Changes the current security level. Logs the transition between security levels, affecting restrictions on parameter modifications. At level N, only parameters with `secure_level >= N` are writable.

```c
esp_err_t NvsConfig_SecureLevelChange(uint8_t new_secure_level);
```

**Returns:**
ESP_OK on success, ESP_ERR_INVALID_ARG if the level is out of range.

---

## Parameter Functions (Generated via Macros)

For each parameter declared in the external `param_table.inc`, several functions are automatically generated.

### Scalar Parameters

- **Set a Parameter:**

  ```c
  esp_err_t Param_Set<name>(const type value);
  ```

  _Updates the parameter value if allowed by the current security level and marks it as dirty if changed. Thread-safe._

  **Returns:** ESP_OK if the value changed, ESP_FAIL if it was the same, ESP_ERR_INVALID_STATE if the security level is insufficient.

- **Get a Parameter:**

  ```c
  type Param_Get<name>(void);
  ```

  _Returns the current value of the parameter. Thread-safe._

- **Reset a Parameter:**

  ```c
  esp_err_t Param_Reset<name>(void);
  ```

  _Resets the parameter to its default value and marks it as dirty. Thread-safe._

- **Print a Parameter:**
  ```c
  int Param_Print<name>(char *buffer, size_t buffer_size);
  ```
  _Generates a formatted string representation of the parameter and writes it into `buffer` using a defined format. The function returns the total number of characters that were written (excluding the null terminator) in the normal case. If the function detects that there isn't enough space in the provided buffer (for example, while printing array elements or adding a separator), it returns a value equal to `buffer_size` to indicate truncation/error._

---

### Array Parameters

- **Set an Array Parameter:**

  ```c
  esp_err_t Param_Set<name>(const type *value, size_t length);
  ```

  _Updates the array parameter, ensuring the length does not exceed the defined maximum. Thread-safe._

- **Get an Array Parameter:**

  ```c
  const type* Param_Get<name>(size_t *out_array_length);
  ```

  _Retrieves a pointer to the array along with its current length. Thread-safe._

- **Copy an Array Parameter:**

  ```c
  esp_err_t Param_Copy<name>(type *buffer, size_t buffer_size);
  ```

  _Copies the array's contents into the provided buffer. Returns ESP_OK on success or ESP_ERR_INVALID_SIZE if the buffer is too small. Thread-safe._

- **Reset an Array Parameter:**

  ```c
  esp_err_t Param_Reset<name>(void);
  ```

  _Resets the array parameter to its default values and marks it as dirty. Thread-safe._

- **Print an Array Parameter:**
  ```c
  int Param_Print<name>(char *buffer, size_t buffer_size);
  ```
  _Generates a formatted string representation of the array parameter. The resulting string is written into `buffer`. In normal operation, the function returns the total number of characters written (excluding the null terminator). If the provided buffer is not large enough to hold the complete output (for example, while adding separators between array elements), the function returns a value equal to `buffer_size` as an indicator of truncation/error._

---

## Parameter Registry

The registry provides runtime introspection over all parameters via a vtable pattern. Each parameter gets one entry in the global `g_nvsconfig_params[]` array.

### struct `NvsConfigParamEntry_t`

|       Type | Field                                                                                                  |
| ---------: | :----------------------------------------------------------------------------------------------------- |
| const char* | **name** <br>_Parameter name as defined in param_table.inc._                                         |
| const char* | **description** <br>_Human-readable description string._                                             |
|    uint8_t | **secure_level** <br>_Security level required to write this parameter._                                |
|       bool | **is_array** <br>_True for array parameters, false for scalars._                                       |
|     size_t | **element_size** <br>_sizeof(type) for one element._                                                   |
|     size_t | **element_count** <br>_1 for scalars, array size for arrays._                                          |
|   bool (\*)() | **is_dirty** <br>_Returns true if the parameter has been modified since last save._                 |
|   bool (\*)() | **is_default** <br>_Returns true if the parameter is at its default value._                         |
| esp_err_t (\*)() | **reset** <br>_Resets the parameter to its default value._                                      |
| int (\*)(char\*, size_t) | **print** <br>_Prints the value into a buffer. Returns characters written._              |
| esp_err_t (\*)(const void\*, size_t) | **set** <br>_Sets the value from a raw pointer + size. See below._ |

**`set(const void* data, size_t data_size)` behavior:**

| Case | Scalar | Array |
|---|---|---|
| Exact size match | Sets value, returns ESP_OK | Sets all elements, returns ESP_OK |
| Too small | Returns ESP_ERR_INVALID_SIZE (no write) | Zero-fills remaining, writes, returns ESP_ERR_INVALID_SIZE (warning) |
| Too large | Returns ESP_ERR_INVALID_SIZE (no write) | Returns ESP_ERR_INVALID_SIZE (no write) |

### Registry Functions

|                              Type | Name                                                                                                                          |
| --------------------------------: | :---------------------------------------------------------------------------------------------------------------------------- |
| const NvsConfigParamEntry_t\* | [**NvsConfig_FindParam**](#function-nvsconfig_findparam)(const char\* name) <br>_Finds a parameter entry by name._                |
|                              void | [**NvsConfig_ResetAll**](#function-nvsconfig_resetall)(void) <br>_Resets all parameters to their default values._              |
|                              void | [**NvsConfig_PrintAll**](#function-nvsconfig_printall)(void) <br>_Logs all parameter names and values._                        |

---

### function `NvsConfig_FindParam`

Looks up a parameter registry entry by name. Performs a linear search over `g_nvsconfig_params[]`.

```c
const NvsConfigParamEntry_t* NvsConfig_FindParam(const char* name);
```

**Parameters:**
- `name` — The parameter name (case-sensitive, must match the name in param_table.inc).

**Returns:**
Pointer to the entry, or NULL if not found.

---

### function `NvsConfig_ResetAll`

Resets every parameter to its default value by calling each entry's `reset()` function pointer.

```c
void NvsConfig_ResetAll(void);
```

---

### function `NvsConfig_PrintAll`

Logs all parameters to the ESP-IDF log output in the format `name = value`.

```c
void NvsConfig_PrintAll(void);
```

---

## Change Callbacks

Register callbacks that fire when parameter values change. Callbacks are invoked outside the mutex to prevent deadlocks.

|      Type | Name                                                                                                                                                                                              |
| --------: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| esp_err_t | [**NvsConfig_RegisterOnChange**](#function-nvsconfig_registeronchange)(const char\* param_name, NvsConfigOnChange_t cb, void\* user_data) <br>_Registers a per-parameter change callback._        |
| esp_err_t | [**NvsConfig_RegisterGlobalOnChange**](#function-nvsconfig_registerglobalonchange)(NvsConfigOnChange_t cb, void\* user_data) <br>_Registers a callback that fires for any parameter change._      |
|      void | [**NvsConfig_ClearCallbacks**](#function-nvsconfig_clearcallbacks)(void) <br>_Removes all registered callbacks._                                                                                  |

### typedef `NvsConfigOnChange_t`

```c
typedef void (*NvsConfigOnChange_t)(const char* param_name, void* user_data);
```

---

### function `NvsConfig_RegisterOnChange`

Registers a callback that fires when a specific parameter changes.

```c
esp_err_t NvsConfig_RegisterOnChange(const char* param_name,
                                     NvsConfigOnChange_t cb,
                                     void* user_data);
```

**Parameters:**
- `param_name` — Parameter name to watch (case-sensitive).
- `cb` — Callback function.
- `user_data` — Passed to the callback on invocation.

**Returns:**
ESP_OK on success, ESP_ERR_NO_MEM if the maximum number of callback slots (16) is reached.

---

### function `NvsConfig_RegisterGlobalOnChange`

Registers a callback that fires when any parameter changes.

```c
esp_err_t NvsConfig_RegisterGlobalOnChange(NvsConfigOnChange_t cb,
                                           void* user_data);
```

**Returns:**
ESP_OK on success, ESP_ERR_NO_MEM if the maximum number of callback slots is reached.

---

### function `NvsConfig_ClearCallbacks`

Removes all registered callbacks. Primarily useful for testing.

```c
void NvsConfig_ClearCallbacks(void);
```

---

## Wear-Level Tracking

Per-parameter write counters to monitor flash wear. Counters are in-memory and reset on reboot.

|      Type | Name                                                                                                                                      |
| --------: | :---------------------------------------------------------------------------------------------------------------------------------------- |
|  uint32_t | [**NvsConfig_GetWriteCount**](#function-nvsconfig_getwritecount)(const char\* name) <br>_Returns the write count for a parameter._        |
|  uint32_t | [**NvsConfig_GetTotalWriteCount**](#function-nvsconfig_gettotalwritecount)(void) <br>_Returns the total write count across all params._    |
|      void | [**NvsConfig_ResetWriteCounts**](#function-nvsconfig_resetwritecounts)(void) <br>_Resets all write counters to zero._                      |

---

### function `NvsConfig_GetWriteCount`

Returns the number of successful writes to a parameter since initialization.

```c
uint32_t NvsConfig_GetWriteCount(const char* name);
```

**Returns:**
Write count, or 0 if the parameter name is not found.

---

### function `NvsConfig_GetTotalWriteCount`

Returns the sum of all per-parameter write counts.

```c
uint32_t NvsConfig_GetTotalWriteCount(void);
```

---

### function `NvsConfig_ResetWriteCounts`

Resets all write counters to zero. Primarily useful for testing.

```c
void NvsConfig_ResetWriteCounts(void);
```

---

## Schema Versioning

Detects parameter table changes across firmware updates. When `NvsConfig_Init()` finds a version mismatch in NVS, it invokes the registered migration callback. If no callback is registered (or it returns an error), all parameters are reset to defaults.

|      Type | Name                                                                                                                                                              |
| --------: | :---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| esp_err_t | [**NvsConfig_RegisterMigration**](#function-nvsconfig_registermigration)(NvsConfigMigrationCb_t cb) <br>_Registers a migration callback. Call before Init._        |
|  uint32_t | [**NvsConfig_GetSchemaVersion**](#function-nvsconfig_getschemaversion)(void) <br>_Returns the current schema version._                                            |

### define `NVS_CONFIG_SCHEMA_VERSION`

Define this macro before including `nvs_config.h` to set your schema version. Defaults to 1.

```c
#define NVS_CONFIG_SCHEMA_VERSION 2
#include "nvs_config.h"
```

### typedef `NvsConfigMigrationCb_t`

```c
typedef esp_err_t (*NvsConfigMigrationCb_t)(uint32_t old_version, uint32_t new_version);
```

---

### function `NvsConfig_RegisterMigration`

Registers a migration callback. Must be called **before** `NvsConfig_Init()`. When a version mismatch is detected, the callback is invoked with the old and new version numbers. If the callback returns ESP_OK, parameters are loaded normally. Any other return value causes a full reset to defaults.

```c
esp_err_t NvsConfig_RegisterMigration(NvsConfigMigrationCb_t cb);
```

**Returns:**
ESP_OK on success.

---

### function `NvsConfig_GetSchemaVersion`

Returns the current schema version stored in NVS.

```c
uint32_t NvsConfig_GetSchemaVersion(void);
```

---

## File: nvs_config_console.h

Optional header for interactive UART console commands. Enable by setting `CONFIG_NVS_CONFIG_CONSOLE_ENABLED=y` in your sdkconfig or sdkconfig.defaults.

|      Type | Name                                                                                                                          |
| --------: | :---------------------------------------------------------------------------------------------------------------------------- |
| esp_err_t | [**NvsConfig_ConsoleInit**](#function-nvsconfig_consoleinit)(void) <br>_Registers console commands for parameter management._ |

---

### function `NvsConfig_ConsoleInit`

Registers the following ESP-IDF console commands:

| Command | Description |
|---|---|
| `param-list` | List all parameters with current values and flags |
| `param-get <name>` | Print a single parameter's value |
| `param-set <name> <value>` | Set a scalar parameter from a string |
| `param-reset <name\|all>` | Reset one parameter or all parameters to defaults |
| `param-save` | Force-save dirty parameters to NVS flash |
| `param-level [N]` | Get or set the current security level |

Call this after `esp_console_init()` (or before starting a REPL) and after `NvsConfig_Init()`.

```c
esp_err_t NvsConfig_ConsoleInit(void);
```

**Returns:**
ESP_OK on success.

---

## Additional Notes

- **Thread Safety:**
  All generated parameter functions and core functions are protected by a FreeRTOS mutex. Change callbacks are invoked outside the mutex to prevent deadlocks.

- **Parameter Declarations:**
  Refer to the [parameter table example file](param_table_example.inc) for guidelines on defining parameters using the `PARAM` and `ARRAY` macros.

- **Logging:**
  The library utilizes ESP-IDF's logging system (`esp_log.h`) to output debug and error messages to assist with troubleshooting.

- **Error Handling:**
  All functions (except the print functions) return standard ESP error codes, enabling seamless integration into your application's error handling routines.
  Print functions follow the `snprintf` convention and return an integer indicating the number of characters that would have been written if sufficient space were available; if truncation or an error occurs, they return `buf_size`.

For further examples and usage scenarios, please refer to the [README.md](README.md) or visit the [GitHub repository](https://github.com/hmolavi/nvs_config).

Happy coding!
