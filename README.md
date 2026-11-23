# ESP32 temperature and LED control

My ESP32 project for reading temperature and humidity, sending readings to Ubidots, and controlling LEDs over MQTT.

It includes manual LED control and a temperature-based mode with hysteresis to avoid switching repeatedly around the same threshold.

## Files

- `espp32_mqtt_1700331.ino` — main sketch
- `PubSubClientExt.h` — MQTT helper

## Setup

1. Install Arduino IDE and ESP32 board support.
2. Install the libraries included by the sketch.
3. Check the sensor and LED pin assignments against your wiring.
4. Set your own Wi-Fi and Ubidots configuration in the sketch.
5. Upload it to the ESP32.
