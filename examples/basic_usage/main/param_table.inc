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
SECURE_LEVEL(2, "User level")

// Define parameters
PARAM(2, uint8_t, ExampleCounter, 0, "A simple counter for the example")
ARRAY(2, char, 16, ExampleString, "Hello", "A test string for the example")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL