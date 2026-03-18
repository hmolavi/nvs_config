# Security Levels

Role-based access control for parameters.

## What It Does

Parameters are assigned a security level in `param_table.inc`. At runtime, a global security level restricts which parameters can be written:

| Runtime Level | Role | Can Write |
|---|---|---|
| 0 | Admin | All parameters |
| 1 | Technician | Level 1 + 2 params only |
| 2 | End User | Level 2 params only |

The example walks through each level, showing which writes succeed and which are denied with `ESP_ERR_INVALID_STATE`.

## Build & Run

```bash
cd examples/security_levels
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

## Expected Output

```
I (XXX) SECURITY_DEMO: --- Level 0 (Admin): full access ---
I (XXX) SECURITY_DEMO:   Set SerialNumber=12345: ESP_OK
I (XXX) SECURITY_DEMO:   Set CalibOffset=42: ESP_OK
I (XXX) SECURITY_DEMO:   Set Brightness=200: ESP_OK

I (XXX) SECURITY_DEMO: --- Level 1 (Technician): admin params blocked ---
I (XXX) SECURITY_DEMO:   Set SerialNumber=99999: ESP_ERR_INVALID_STATE
I (XXX) SECURITY_DEMO:   Set CalibOffset=-10: ESP_OK
I (XXX) SECURITY_DEMO:   Set Brightness=100: ESP_OK

I (XXX) SECURITY_DEMO: --- Level 2 (End User): admin + technician params blocked ---
I (XXX) SECURITY_DEMO:   Set SerialNumber=99999: ESP_ERR_INVALID_STATE
I (XXX) SECURITY_DEMO:   Set CalibOffset=100: ESP_ERR_INVALID_STATE
I (XXX) SECURITY_DEMO:   Set Brightness=50: ESP_OK
```
