# Console Demo

Interactive parameter management over UART using the ESP-IDF console REPL.

## What It Does

Starts a command-line shell on the serial port. You can inspect and modify NVS parameters in real time without reflashing.

## Build & Run

```bash
cd examples/console_demo
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

> This example requires `CONFIG_NVS_CONFIG_CONSOLE_ENABLED=y`, which is already set in `sdkconfig.defaults`.

## Available Commands

| Command | Description |
|---|---|
| `param-list` | List all parameters with values and flags |
| `param-get <name>` | Print a parameter's current value |
| `param-set <name> <value>` | Set a scalar parameter |
| `param-reset <name\|all>` | Reset one or all parameters to defaults |
| `param-save` | Force-save dirty parameters to NVS flash |
| `param-level [N]` | Get or set the security level |

## Example Session

```
nvs> param-list
NAME             LEVEL  DIRTY DFLT  TYPE     VALUE
--------------------------------------------------------------
Brightness       0      no    yes   scalar   128
LedEnabled       0      no    yes   scalar   1
TempOffset       1      no    yes   scalar   0
Threshold        1      no    yes   scalar   25
DeviceName       0      no    yes   array    ESP32-Device
IpAddress        0      no    yes   array    [192,168,1,100]

nvs> param-set Brightness 200
Brightness = 200

nvs> param-get Brightness
Brightness = 200

nvs> param-reset Brightness
Brightness reset to 128

nvs> param-save
Dirty parameters saved to flash
```
