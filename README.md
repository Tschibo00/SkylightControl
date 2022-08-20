# SkylightControl
ESP32 project to automatically open/close skylight in conservatory.

## Features
Temperature is monitored, skylight is closed below configurable temperature or opened above configurable temperature.
If humidity is too high, skylight is openend as well.
If rain starts, skylight is immediately closed, no matter what the other values indicate.
Driving of the skylight is only allowed between 9am and 9pm, on 9pm the skylight is closed and monitoring of temperature/humidity/rain is paused until the next morning.

Rain amount per hour is calculated and pushed (with temperature, humidity and pressure) to thinger.io
Also includes a minimal webpage showing the current values and allowing to configure the parameters.

## Hardware
ESP32 with BME280 sensor, Hydreon RG-11 in tipping bucket for sensing rain and a double relais output to trigger motor control of the skylight.
