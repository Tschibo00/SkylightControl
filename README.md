# SkylightControl

ESP32 project to automatically open/close skylight in conservatory.
Temperature is monitored, skylight is closed below configurable temperature or opened above configurable temperature.
If humidity is too high, skylight is openend as well.
If rain starts, skylight is immediately closed, no matter what the other values indicate.
Rain amount per hour is calculated and pushed (with temperature, humidity and pressure) to thinger.io
Also includes a minimal webpage showing the current values and allowing to configure the parameters.
