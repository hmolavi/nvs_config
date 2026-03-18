// param_table.inc for registry_iterator example
// Demonstrates iterating over all parameters generically via the registry.

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

// A mix of types to demonstrate generic iteration
PARAM(0, uint8_t,  Volume,      75,         "Audio volume 0-100")
PARAM(0, bool,     Muted,       false,      "Audio mute state")
PARAM(0, float,    Balance,     0.0,        "L/R balance -1.0 to 1.0")
PARAM(0, int32_t,  BassBoost,   0,          "Bass EQ in dB")
PARAM(0, uint32_t, SampleRate,  44100,      "Audio sample rate Hz")

ARRAY(0, uint8_t, 3, RGBColor, ARRAY_INIT(255, 128, 0), "Status LED color")
ARRAY(0, float, 5, EqBands, ARRAY_INIT(0.0, 0.0, 0.0, 0.0, 0.0), "5-band EQ gains")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL
