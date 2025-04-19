[![Component Registry](https://components.espressif.com/components/hmolavi/nvs_config/badge.svg)](https://components.espressif.com/components/hmolavi/nvs_config) [![Espressif](https://img.shields.io/badge/Espressif-Components-blue.svg?style=flat-square)](https://components.espressif.com/components/hmolavi/nvs_config) [![GitHub](https://img.shields.io/badge/GitHub-hmolavi/nvs_config-blue.svg?style=flat-square)](https://github.com/hmolavi/nvs_config)

# NVS Config Library

The **NVS Storage Config Library** is an ESP-IDF component designed to simplify the creation, management, and persistence of application parameters using Non-Volatile Storage (NVS). It supports both scalar values and array parameters, provides automatic verification against default values, and enables periodic saving of modified parameters using a FreeRTOS timer.

## Features

- **Easy Parameter Declaration:** Use the `PARAM` and `ARRAY` macros (via an external parameter table file) to define parameters and their default values.
- **Secure Parameter Management:** Configure security levels to protect sensitive parameters.
- **Automatic Persistence:** Modified parameters are marked as “dirty” and are safely saved to flash.
- **Runtime Updates:** Change security levels and update parameters at runtime without needing to rebuild the firmware.
- **Cross-Platform Compatibility:** Built with ESP-IDF for seamless integration in ESP32-based projects.

## How It Works

1. **Initialization:**  
   The `NvsConfig_Init()` function initializes NVS flash storage, loads parameter values from flash (or sets defaults if not present), and creates a periodic timer to check and save modified parameters.
2. **Setting and Getting Parameters:**  
   Each parameter has generated API functions, such as `Param_Set<name>`, `Param_Get<name>`, and `Param_Reset<name>`, to manage its value.
3. **Security Levels:**  
   Access to parameters is controlled via security levels. Use `NvsConfig_SecureLevel()` and `NvsConfig_SecureLevelChange()` to check and update the current security level.

## Getting Started

1. **Add the Component:**  
   Include the component in your ESP-IDF project by referencing it in your project's `CMakeLists.txt` and adding it as a dependency:

   ```bash
   idf.py add-dependency "hmolavi/nvs-config"
   ```

2. **Define Your NVS Parameters:**  
   Create an `param_table.inc` file in your project's `main` directory. Use either the provided example file, [param_table_example.inc](param_table_example.inc), or one here:

   ```c
   /* param_table.inc */
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

   SECURE_LEVEL(0, "Secure Level 0")

   PARAM(0, char, Letter, 'A', "example char")
   /* Add other parameters... */

   #undef PARAM
   #undef ARRAY
   #undef SECURE_LEVEL
   ```
3. **Include Header:**  
   Add the following include statement to your source file:

   ```c
   #include "nvs_config.h"
   ```

4. **Initialize in User Code:**  
   Call `NvsConfig_Init()` at startup to load and manage parameters.

5. **Accessing Your Parameters:**  
   To interact with your parameters, simply prefix the action (Get, Set, Reset, Print) with `Param_` followed by the name of the parameter as defined in your "param_table.inc" file. This naming convention automatically provides you with the corresponding function for that parameter.

   ```c
   char letter;
   char buf[128];
   int buffer_write;
   esp_err_t rc;

   // Get (default is A)
   letter = Param_GetLetter();
   
   // Set
   rc = Param_SetLetter('Z');
   
   // Reset (back to A)
   rc = Param_ResetLetter();
   
   // Print
   buffer_write = Param_PrintLetter(buf, sizeof(buf));
   printf("Our letter is: %s", buf);
   ```

## Examples

For a complete demonstration, please refer to the examples provided in the examples folder of this repository.  
- [basic_usage](/examples/basic_usage/)
- [array_example](/examples/array_example/)

## Build & Integration

- **idf_component.yml**  
  Ensure the component’s version and dependencies are correctly specified.
- **CMakeLists.txt**  
  The component is registered with ESP-IDF using `idf_component_register`.

## License

This project is released under the [MIT License](LICENSE).

## Support

For support and contributions, please visit the [GitHub repository](https://github.com/hmolavi/nvs_config).
