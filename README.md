# Description
ESP32 Environmental Monitoring with OLED Display and Wi-Fi Server

This project utilizes an ESP32 to monitor environmental conditions using a BME280 sensor for temperature, humidity, and pressure readings. The data is displayed on an OLED screen and can be accessed via a simple Wi-Fi web server. The system supports manual data logging, real-time sensor readings, and data retrieval through a RESTful interface.

Features:
Real-time display of temperature, pressure, and humidity on an OLED screen.

Wi-Fi access point for remote sensor data access.

JSON endpoint to retrieve logged sensor data.

Ability to add manual sensor readings.

NTP synchronization for accurate timestamps.

High humidity warning display when humidity exceeds 70%.

Technologies:
ESP32 microcontroller

BME280 sensor (temperature, pressure, humidity)

Adafruit SSD1306 OLED display

Wi-Fi server for remote data access

NTP for time synchronization

Ideal for building environmental monitoring systems and IoT-based applications.
