# Homie ESP8266 BME280 sensor

This simple code uses
[homie-esp8266](https://github.com/marvinroger/homie-esp8266) to create a wifi
sensors that will talk to my [Home Assistant](https://home-assistant.io/) via
MQTT, following the [homie](https://github.com/marvinroger/homie/tree/master)
convention for IoT.

# Diagram

![diagram](https://raw.githubusercontent.com/bleader/homie-esp8266-bme280/master/diagram/bme280.jpg)

# Notes

- Pressure is adjusted to match the offset I observed compared to weather
  station nearby
- Battery/grid powered is now an option
- For battery operation, disabling debug will save a tiny bit of battery life

