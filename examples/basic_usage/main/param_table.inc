// Example param_table.inc for basic_usage example

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

// Define security levels
SECURE_LEVEL(0, "Full access")

// Define parameters
// @warning The name is used as hashkey for nvs_blob so your parameter name cannot exceed 15 characters.
//          For reference this is a 15 character name "someverylongvar"
PARAM(0, uint8_t, ExampleCounter, 0, "A simple counter for the example")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL