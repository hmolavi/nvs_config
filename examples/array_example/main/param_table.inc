// param_table.inc for array_example
// Located at: nvs-config/examples/array_example/main/param_table.inc

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
SECURE_LEVEL(0, "Full access")

// Example Array
// @warning The name is used as hashkey for nvs_blob so your parameter name cannot exceed 15 characters.
//          For reference this is a 15 character name "someverylongvar"
ARRAY(0, int32_t, 4, ExampleIntArray, ARRAY_INIT(10, 20, 30, 40), "An example integer array")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL