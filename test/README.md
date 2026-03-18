# Stress Test

Comprehensive validation of all supported types, boundary values, and persistence.

## What It Does

- Tests all 12 scalar types (char, bool, int8..int64, uint8..uint64, float, double)
- Tests array parameters across multiple types
- Validates print formatting for all types
- Exercises security level enforcement across 3 tiers
- Rapid-fire write stress testing (256+ writes in tight loops)
- Cross-reboot persistence verification

## Build & Run

```bash
cd examples/stress_test
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

## Expected Output

A long series of pass/fail results followed by a summary:

```
I (XXX) STRESS: ========================================
I (XXX) STRESS:   RESULTS: XX/XX tests passed
I (XXX) STRESS: ========================================
```
