// param_table.inc for security_levels example
// Demonstrates role-based access control for parameters.

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

// Three security tiers: 0 = most privileged, 2 = most restricted
SECURE_LEVEL(0, "Admin - factory/debug access")
SECURE_LEVEL(1, "Technician - field maintenance")
SECURE_LEVEL(2, "End user - basic operation")

// Admin-only parameters (level 0)
PARAM(0, uint32_t, SerialNumber, 0,       "Factory-assigned serial number")
PARAM(0, bool,     DebugMode,    false,    "Enable verbose debug logging")

// Technician parameters (level 1)
PARAM(1, int16_t,  CalibOffset,  0,        "Sensor calibration offset")
PARAM(1, uint16_t, SampleRate,   1000,     "ADC sample rate in Hz")

// User parameters (level 2)
PARAM(2, uint8_t,  Brightness,   128,      "Display brightness")
PARAM(2, float,    TempUnit,     1.0,      "1.0 = Celsius, 1.8 = Fahrenheit scale")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL
