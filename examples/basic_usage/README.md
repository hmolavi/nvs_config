# Basic Usage

The simplest possible use of nvs_config — a single parameter that persists across reboots.

## What It Does

1. Initializes NVS config with `NvsConfig_Init()`
2. Reads a counter, increments it, writes it back
3. The value survives power cycles (stored in NVS flash)

## Build & Run

```bash
cd examples/basic_usage
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

## Expected Output

```
I (XXX) EXAMPLE_MAIN: NvsConfig Initialized.
I (XXX) EXAMPLE_MAIN: Initial ExampleCounter value: 0
I (XXX) EXAMPLE_MAIN: Set ExampleCounter to: 1
I (XXX) EXAMPLE_MAIN: Example finished.
```

On subsequent boots the counter increments (1, 2, 3, ...).
