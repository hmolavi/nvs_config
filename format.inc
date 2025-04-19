/**
 * @file format.inc
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief Print formatting for print functions
 * 
 * @copyright Copyright (c) 2025
 */


#ifndef PRINT_FORMAT
#define PRINT_FORMAT(...)
#endif

PRINT_FORMAT(char, "%c")
PRINT_FORMAT(bool, "%d")
PRINT_FORMAT(int8_t, "%" PRId8)
PRINT_FORMAT(uint8_t, "%" PRIu8)
PRINT_FORMAT(int16_t, "%" PRId16)
PRINT_FORMAT(uint16_t, "%" PRIu16)
PRINT_FORMAT(int32_t, "%" PRId32)
PRINT_FORMAT(uint32_t, "%" PRIu32)
PRINT_FORMAT(int64_t, "%" PRId64)
PRINT_FORMAT(uint64_t, "%" PRIu64)
PRINT_FORMAT(float, "%.6g")
PRINT_FORMAT(double, "%.6g")

#undef PRINT_FORMAT
