/**
 * @file param_table_example.inc
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief Example param_table.inc file
 * 
 * Each parameter is defined using the PARAM or the ARRAY macros, 
 * specifying its:
 *  - security level
 *  - data type
 *  - variable name
 *  - default value
 *  - description
 * 
 * For ARRAY parameters, the maximum length must be specified.
 * 
 * @warning The name is used as hashkey for nvs_blob so your parameter name 
 *          cannot exceed 15 characters.
 */

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

/* Can have [1,255] security levels (i.e. the secure_level must fit in uint8_t) */
SECURE_LEVEL(0, "Full access")
SECURE_LEVEL(1, "Maintenance")
SECURE_LEVEL(2, "User level")


PARAM(0, char, ExChar, 'A', "example char")
PARAM(1, uint8_t, ExUint8, 255, "example uint8_t")
PARAM(2, uint16_t, ExUint16, 65535, "example uint16_t")
PARAM(2, uint32_t, ExUint32, 4294967295, "example uint32_t")
PARAM(2, int32_t, ExInt32, -2147483648, "example int32_t")
PARAM(2, float, ExFloat, 3.14, "example float")

ARRAY(2, char, 32, ExCharArr, "example char array", "example char array")
ARRAY(2, uint8_t, 5, ExUint8Arr, ARRAY_INIT(1, 2, 3, 4, 5), "example uint8_t array")
ARRAY(2, uint16_t, 3, ExUint16Arr, ARRAY_INIT(100, 200, 300), "example uint16_t array")
ARRAY(2, int32_t, 4, ExUint32Arr, ARRAY_INIT(1, 0, 0, 0), "example int array")
ARRAY(2, float, 3, ExFloatArr, ARRAY_INIT(1.1, 2.2, 3.3), "example float array")
ARRAY(2, bool, 2, ExBoolArr, ARRAY_INIT(true, false), "example bool array")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL