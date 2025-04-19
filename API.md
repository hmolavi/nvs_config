# API Reference

## Header files

- [nvs_config.h](#file-nvs_configh)

## File nvs_config.h

This header provides the interface for the NVS Storage Config Library component.

## Functions

|      Type      | Name |
| -------------: | :--- |
| esp_err_t      | [**NvsConfig_Init**](#function-nvsconfig_init)(void) <br>_Initializes NVS flash storage and config parameters._ |
| void           | [**NvsConfig_SaveDirtyParameters**](#function-nvsconfig_savedirtyparameters)(void) <br>_Saves modified parameters to NVS flash._ |
| uint8_t        | [**NvsConfig_SecureLevel**](#function-nvsconfig_securelevel)(void) <br>_Retrieves the current security level._ |
| esp_err_t      | [**NvsConfig_SecureLevelChange**](#function-nvsconfig_securelevelchange)(uint8_t new_secure_level) <br>_Changes the current security level and logs the transition._ |

## Parameter Functions (Generated via Macros)

For each parameter declared in the external `param_table.inc`, the following functions are automatically generated:

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

### Array Parameters

- **Set an Array Parameter:**  
    ```c
    esp_err_t Param_Set<name>(const type *value, size_t length);
    ```
    _Updates the array parameter ensuring the length does not exceed the maximum defined._

- **Get an Array Parameter:**  
    ```c
    const type* Param_Get<name>(size_t *out_array_length);
    ```
    _Retrieves the array and its current length._

- **Copy an Array Parameter:**  
    ```c
    esp_err_t Param_Copy<name>(type *buffer, size_t buffer_size);
    ```
    _Copies the array’s contents into the provided buffer._

- **Reset an Array Parameter:**  
    ```c
    esp_err_t Param_Reset<name>(void);
    ```
    _Resets the array parameter to its default values and marks it as dirty._

- **Print a Parameter:**  
    Both scalar and array parameters include a print function (e.g., `Param_Print<name>`) to generate a formatted string representation.

## Functions Documentation

### function `NvsConfig_Init`

Initializes NVS flash storage and loads configuration parameters from flash or sets defaults. Also starts a periodic FreeRTOS timer (every 30 seconds) to commit any changes.

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

