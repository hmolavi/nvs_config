# Array Example

Working with fixed-size typed arrays stored in NVS.

## What It Does

1. Retrieves the default `int32_t[4]` array and prints it
2. Sets new values with `Param_SetExampleIntArray()`
3. Copies the array to a local buffer with `Param_CopyExampleIntArray()`
4. Resets to defaults with `Param_ResetExampleIntArray()`

## Build & Run

```bash
cd examples/array_example
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

## Expected Output

```
I (XXX) ARRAY_EXAMPLE: Initial array (length 4): [10,20,30,40]
I (XXX) ARRAY_EXAMPLE: Updated array value: [99,88,77,66]
I (XXX) ARRAY_EXAMPLE: Copy verified successfully.
I (XXX) ARRAY_EXAMPLE: Current array value after reset: [10,20,30,40]
```
