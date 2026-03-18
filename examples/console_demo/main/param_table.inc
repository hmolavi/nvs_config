// param_table.inc for console_demo example
// Demonstrates interactive parameter management over UART.

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

// Security levels
SECURE_LEVEL(0, "Admin")
SECURE_LEVEL(1, "User")

// Device configuration parameters
PARAM(0, uint8_t,  Brightness,  128,        "Display brightness 0-255")
PARAM(0, bool,     LedEnabled,  true,       "LED on/off")
PARAM(1, int16_t,  TempOffset,  0,          "Temperature calibration offset")
PARAM(1, float,    Threshold,   25.0,       "Alert temperature threshold")

// Network settings
ARRAY(0, char, 32, DeviceName, "ESP32-Device", "Device hostname")
ARRAY(1, uint8_t, 4, IpAddress, ARRAY_INIT(192, 168, 1, 100), "Static IP address")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL
