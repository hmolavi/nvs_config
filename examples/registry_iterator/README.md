# Registry Iterator

Generic, type-agnostic parameter operations via the parameter registry vtable.

## What It Does

1. Iterates `g_nvsconfig_params[]` to print metadata for every parameter (name, type, element size/count, description)
2. Looks up a parameter by name with `NvsConfig_FindParam()`
3. Sets values via the generic `entry->set(void*, size)` interface — works for both scalars and arrays without knowing the type
4. Inspects dirty/default flags through the vtable
5. Resets all parameters in one call with `NvsConfig_ResetAll()`

## Build & Run

```bash
cd examples/registry_iterator
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

## Expected Output

```
I (XXX) REGISTRY_DEMO: --- All 7 registered parameters ---
I (XXX) REGISTRY_DEMO: [0] Volume       = 75       type=scalar  elem_size=1  count=1  desc="Audio volume 0-100"
I (XXX) REGISTRY_DEMO: [1] Muted        = 0        type=scalar  elem_size=1  count=1  desc="Audio mute state"
...
I (XXX) REGISTRY_DEMO: --- Find by name ---
I (XXX) REGISTRY_DEMO: Found 'Volume': value=75, is_default=yes, is_dirty=no
I (XXX) REGISTRY_DEMO: --- Generic set() via void* ---
I (XXX) REGISTRY_DEMO: Set Volume to 90: ESP_OK
I (XXX) REGISTRY_DEMO: Set RGBColor to {0, 255, 128}: ESP_OK
...
```
