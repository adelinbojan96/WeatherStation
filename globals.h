// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <Adafruit_SSD1306.h>
#include "bmx_280.h"

// OLED Display
extern Adafruit_SSD1306 display;

// BME280 Sensor
extern BMx280 envSensor;

// Timing Variables
extern unsigned long lastUpdate;
extern const unsigned long frameInterval;

// Frame Control
extern int currentFrame;

// Sensor Data
extern float lastTemperature;
extern float lastPressure;
extern float lastHumidity;

// Logging Variables
extern unsigned long lastLogTime;
extern const unsigned long logInterval;

// Function Prototypes
bool readSensors();
void logDataToSerial();

#endif
