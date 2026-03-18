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

/* ── Security Levels (0 = most privileged) ── */
SECURE_LEVEL(0, "Admin - full access")
SECURE_LEVEL(1, "Operator")
SECURE_LEVEL(2, "User - restricted")

/* ── Scalar Parameters: every type from format.inc ── */

/* Admin (Level 0) */
PARAM(0, char,     Letter,         'A',            "char type")
PARAM(0, bool,     AdminLock,      false,          "bool type")
PARAM(0, int8_t,   TinyOffset,     -127,           "int8_t near min")
PARAM(0, uint8_t,  Brightness,     255,            "uint8_t max")
PARAM(0, int64_t,  BigTimestamp,   1700000000LL,   "int64_t large value")
PARAM(0, uint64_t, DeviceUID,      0ULL,           "uint64_t zero")

/* Operator (Level 1) */
PARAM(1, int16_t,  Altitude,       -32000,         "int16_t negative")
PARAM(1, uint16_t, SampleRate,     60000,          "uint16_t large")
PARAM(1, int32_t,  CalibOffset,    -2000000000,    "int32_t large neg")
PARAM(1, uint32_t, SerialNum,      4000000000U,    "uint32_t large")

/* User (Level 2) */
PARAM(2, float,    TempReading,    -40.5f,         "float negative")
PARAM(2, double,   GpsLongitude,   -122.419418,    "double precision")

/* WARNING: Parameter names must be <= 15 characters (NVS key limit) */
/* 15-char name: M-a-x-N-a-m-e-T-e-s-t-X-X-X-1 */
PARAM(0, uint32_t, MaxNameTestXXX1, 42U,           "15-char name limit test")

/* ── Array Parameters ── */

/* Admin (Level 0) */
ARRAY(0, char,     16, DeviceName,   ARRAY_INIT('S','t','r','e','s','s','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'), "char array string")
ARRAY(0, uint8_t,   8, BytePattern,  ARRAY_INIT(0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE), "raw byte pattern")

/* Operator (Level 1) */
ARRAY(1, int32_t,   6, CalibPoints,  ARRAY_INIT(-1000, -500, 0, 500, 1000, 2000), "calibration curve")
ARRAY(1, float,     4, Thresholds,   ARRAY_INIT(0.001f, 1.0f, 100.0f, 9999.99f), "alarm thresholds")

/* User (Level 2) */
ARRAY(2, bool,      8, FeatureFlags, ARRAY_INIT(true, false, true, false, true, false, true, false), "feature toggle bits")
ARRAY(2, uint16_t,  3, RGBColor,    ARRAY_INIT(255, 128, 0), "display color RGB")

#undef PARAM
#undef ARRAY
#undef SECURE_LEVEL
