# Change Callbacks

Real-time notifications when parameter values change.

## What It Does

1. Registers a **global callback** that fires on any parameter change (e.g., audit logging)
2. Registers **per-parameter callbacks** for `Brightness` and `TempTarget` (e.g., adjust hardware)
3. Passes `user_data` context to callbacks
4. Shows that callbacks do NOT fire when a value is set to its current value

## Build & Run

```bash
cd examples/change_callbacks
idf.py set-target <your-esp32-model>
idf.py build flash monitor
```

## Expected Output

```
I (XXX) CALLBACK_DEMO: >> Setting Brightness to 200
W (XXX) CALLBACK_DEMO: [MyApp] Parameter 'Brightness' changed!
I (XXX) CALLBACK_DEMO: LED brightness updated to 200 — applying to hardware PWM

I (XXX) CALLBACK_DEMO: >> Setting TempTarget to 24.5
W (XXX) CALLBACK_DEMO: [MyApp] Parameter 'TempTarget' changed!
I (XXX) CALLBACK_DEMO: Temperature target changed to 24.5 C — updating PID controller

I (XXX) CALLBACK_DEMO: >> Setting LedEnabled to false
W (XXX) CALLBACK_DEMO: [MyApp] Parameter 'LedEnabled' changed!

I (XXX) CALLBACK_DEMO: >> Setting LedEnabled to false again (same value)
I (XXX) CALLBACK_DEMO:    Value unchanged — no callbacks fired (as expected)
```
