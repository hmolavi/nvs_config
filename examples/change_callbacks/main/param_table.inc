// param_table.inc for change_callbacks example
// Demonstrates parameter change notifications.

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

SECURE_LEVEL(0, "Full access")

PARAM(0, uint8_t,  Brightness, 128, "LED brightness 0-255")
PARAM(0, bool,     LedEnabled, true, "LED on/off switch")
PARAM(0, float,    TempTarget, 22.0, "Target temperature in C")
PARAM(0, uint32_t, Interval,   1000, "Sampling interval in ms")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL
