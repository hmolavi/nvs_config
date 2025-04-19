# API Reference

## Header Files

- [nvs_config.h](#file-nvs_configh)

## File: nvs_config.h

This header provides the interface for the NVS Storage Config Library component.

## Core Functions

|      Type | Name                                                                                                                                                                  |
| --------: | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| esp_err_t | [**NvsConfig_Init**](#function-nvsconfig_init)(void) <br>_Initializes NVS flash storage and configuration parameters._                                                |
|      void | [**NvsConfig_SaveDirtyParameters**](#function-nvsconfig_savedirtyparameters)(void) <br>_Saves modified parameters to NVS flash._                                      |
|   uint8_t | [**NvsConfig_SecureLevel**](#function-nvsconfig_securelevel)(void) <br>_Retrieves the current security level._                                                        |
| esp_err_t | [**NvsConfig_SecureLevelChange**](#function-nvsconfig_securelevelchange)(uint8*t new_secure_level) <br>\_Changes the current security level and logs the transition.* |

---

### function `NvsConfig_Init`

Initializes NVS flash storage and loads configuration parameters from flash (or sets defaults). Also starts a periodic FreeRTOS timer (every 30 seconds) to commit any changes.

```c
esp_err_t NvsConfig_Init(void);
```

**Returns:**  
ESP_OK if initialization is successful; otherwise, ESP_FAIL.

---

### function `NvsConfig_SaveDirtyParameters`

Iterates through all parameters, saving modified (“dirty”) parameters to NVS flash and committing the changes.

```c
void NvsConfig_SaveDirtyParameters(void);
```

---

### function `NvsConfig_SecureLevel`

Retrieves the current security level for parameter access.

```c
uint8_t NvsConfig_SecureLevel(void);
```

---

### function `NvsConfig_SecureLevelChange`

Changes the current security level. Logs the transition between security levels, affecting restrictions on parameter modifications.

```c
esp_err_t NvsConfig_SecureLevelChange(uint8_t new_secure_level);
```

**Returns:**  
ESP_OK on success.

---

## Parameter Functions (Generated via Macros)

For each parameter declared in the external `param_table.inc`, several functions are automatically generated.

### Scalar Parameters

- **Set a Parameter:**

  ```c
  esp_err_t Param_Set<name>(const type value);
  ```

  _Updates the parameter value if allowed by the current security level and marks it as dirty if changed._

- **Get a Parameter:**

  ```c
  type Param_Get<name>(void);
  ```

  _Returns the current value of the parameter._

- **Reset a Parameter:**

  ```c
  esp_err_t Param_Reset<name>(void);
  ```

  _Resets the parameter to its default value and marks it as dirty._

- **Print a Parameter:**
  ```c
  int Param_Print<name>(char *buffer, size_t buffer_size);
  ```
  _Generates a formatted string representation of the parameter and writes it into `buffer` using a defined format. The function returns the total number of characters that were written (excluding the null terminator) in the normal case. If the function detects that there isn’t enough space in the provided buffer (for example, while printing array elements or adding a separator), it returns a value equal to `buffer_size` to indicate truncation/error._

---

### Array Parameters

- **Set an Array Parameter:**

  ```c
  esp_err_t Param_Set<name>(const type *value, size_t length);
  ```

  _Updates the array parameter, ensuring the length does not exceed the defined maximum._

- **Get an Array Parameter:**

  ```c
  const type* Param_Get<name>(size_t *out_array_length);
  ```

  _Retrieves the array along with its current length._

- **Copy an Array Parameter:**

  ```c
  esp_err_t Param_Copy<name>(type *buffer, size_t buffer_size);
  ```

  _Copies the array’s contents into the provided buffer. Returns ESP_OK on success or an error code if the buffer is too small._

- **Reset an Array Parameter:**

  ```c
  esp_err_t Param_Reset<name>(void);
  ```

  _Resets the array parameter to its default values and marks it as dirty._

- **Print an Array Parameter:**
  ```c
  int Param_Print<name>(char *buffer, size_t buffer_size);
  ```
  _Generates a formatted string representation of the array parameter. The resulting string is written into `buffer`. In normal operation, the function returns the total number of characters written (excluding the null terminator). If the provided buffer is not large enough to hold the complete output (for example, while adding separators between array elements), the function returns a value equal to `buffer_size` as an indicator of truncation/error._

---

## Additional Notes

- **Parameter Declarations:**  
  Refer to the [parameter table example file](param_table_example.inc) for guidelines on defining parameters using the `PARAM` and `ARRAY` macros.

- **Logging:**  
  The library utilizes ESP-IDF’s logging system (`esp_log.h`) to output debug and error messages to assist with troubleshooting.

- **Error Handling:**  
  All functions (except the print functions) return standard ESP error codes, enabling seamless integration into your application’s error handling routines.  
  Print functions follow the `snprintf` convention and return an integer indicating the number of characters that would have been written if sufficient space were available; if truncation or an error occurs, they return `buf_size`.

For further examples and usage scenarios, please refer to the [README.md](README.md) or visit the [GitHub repository](https://github.com/hmolavi/nvs_config).

Happy coding!
