#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "bmx_280.h" 
#include <WiFi.h>
#include <time.h>

const char* ssid = "adelin";      
const char* password = "password"; 

const int BME_ADDR_ON_BUS = 0x77;
BMx280 envSensor(BME_ADDR_ON_BUS);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long lastOledUpdate = 0;
const unsigned long oledUpdateInterval = 2000;

float currentTemperature = NAN;
float currentPressure = NAN;
float currentHumidity = NAN;

unsigned long lastLogTime = 0;             
const unsigned long logInterval = 5000;      

struct SensorReading {
  char timestamp[20]; 
  float temperature;
  float pressure;
  float humidity;
};

const int bufferSize = 15; 
SensorReading readingsBuffer[bufferSize];
int currentReadingIndex = 0;
int readingsCount = 0;

WiFiServer server(80);

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;     
const int   daylightOffset_sec = 0; 

int currentDisplay = 0; 

bool readSensors();
void handleClientRequest(WiFiClient client);
void logDataToSerial();
void addManualReading();
bool initializeTime();

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- ESP32 Sensor and OLED Display Initialization ---");

  Wire.begin(21, 22); 
  Serial.println("I2C initialized on SDA: GPIO21, SCL: GPIO22");
  delay(250);

  Serial.println("Initializing OLED display...");
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("OLED Initialization Failed!");
    while (1); 
  }
  Serial.println("OLED Initialized Successfully.");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Init...");
  delay(2000);

  Serial.println("Initializing BME280 sensor...");
  if (!envSensor.init()) {
    Serial.println("BME280 Init Failed!");

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Sensor Init");
    display.setCursor(0, 16);
    display.print("Failed!");
    display.display();
    Serial.println("Displayed 'Sensor Init Failed!' on OLED.");
    while (1);
  Serial.println("BME280 Sensor Initialized Successfully.");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Init OK");
  display.display();
  Serial.println("Displayed 'Init OK' on OLED.");
  delay(1000);

  Serial.println("Setting up Wi-Fi Access Point...");
  if (WiFi.softAP(ssid, password)) {
    Serial.println("Wi-Fi Access Point Started Successfully.");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to Start Wi-Fi Access Point!");
    while (1);
  }

  if (!initializeTime()) {
    Serial.println("Failed to initialize time.");
  }

  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {

  WiFiClient client = server.available(); 

  if (client) {
    Serial.println("New client connected.");
    handleClientRequest(client);
    client.stop();
    Serial.println("Client disconnected.");
  }

  unsigned long now = millis();
  if (now - lastOledUpdate >= oledUpdateInterval) {
    lastOledUpdate = now; 
    if (readSensors()) {
      // check if humidity exceeds 70%
      if (!isnan(currentHumidity) && currentHumidity > 70.0) {

        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("WARNING");
        display.setCursor(0, 16);
        display.print("Hum:");
        display.print(currentHumidity, 1);
        display.print("%");
        display.display(); 
        Serial.println("Displayed WARNING on OLED due to high humidity.");
      }
      else {

        display.clearDisplay();
        display.setTextSize(2); 
        display.setTextColor(WHITE);

        switch (currentDisplay) {
          case 0: 
            display.setCursor(0, 16); 
            display.print("Temp:");
            if (!isnan(currentTemperature)) {
              display.print(currentTemperature, 1);
              display.print("C");
            } else {
              display.print("N/A");
            }
            break;
          case 1: 
            display.setCursor(0, 16);
            display.print("Pres:");
            if (!isnan(currentPressure)) {
              display.print(currentPressure, 1);
              display.print("hPa");
            } else {
              display.print("N/A");
            }
            break;
          case 2: 
            display.setCursor(0, 16);
            display.print("Hum:");
            if (!isnan(currentHumidity)) {
              display.print(currentHumidity, 1);
              display.print("%");
            } else {
              display.print("N/A");
            }
            break;
        }

        display.display(); 
        Serial.println("OLED updated with " + String(currentDisplay == 0 ? "Temperature" : (currentDisplay == 1 ? "Pressure" : "Humidity")) + " data.");

        currentDisplay = (currentDisplay + 1) % 3; 
      }
    } else {
      Serial.println("Failed to read sensors for OLED display.");
    }
  }

  if (now - lastLogTime >= logInterval) {
    lastLogTime = now;

    if (readingsCount > 0) {
      logDataToSerial(); 
    } else {
      Serial.println("No sensor data available to log.");
    }
  }

}

bool initializeTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Initializing NTP Time...");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return false;
  }
  Serial.println(&timeinfo, "Current Time: %Y-%m-%d %H:%M:%S");
  return true;
}

void handleClientRequest(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();
  Serial.println("Received request: " + request);

  if (request.indexOf("GET /data") >= 0) {
    String json = "[\n";
    for (int i = 0; i < readingsCount; i++) {

      int index = (currentReadingIndex + bufferSize - readingsCount + i) % bufferSize;
      json += "  {\n";
      json += "    \"timestamp\": \"" + String(readingsBuffer[index].timestamp) + "\",\n";
      json += "    \"temperature\": " + String(readingsBuffer[index].temperature, 2) + ",\n";
      json += "    \"pressure\": " + String(readingsBuffer[index].pressure, 2) + ",\n";
      json += "    \"humidity\": " + String(readingsBuffer[index].humidity, 2) + "\n";
      json += "  }";
      if (i < readingsCount -1) {
        json += ",";
      }
      json += "\n";
    }
    json += "]";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print(json);
    Serial.println("Sent JSON data with " + String(readingsCount) + " readings.");
  }
  else if (request.indexOf("GET /add") >= 0) {
    addManualReading();

    String response = "Added Reading: Temp=" + String(currentTemperature, 2) + "C, Pres=" + String(currentPressure, 2) + "hPa, Hum=" + String(currentHumidity, 2) + "%";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.print(response);
    Serial.println("Added manual sensor reading via /add.");
  }
  else {
    String html = "<!DOCTYPE html><html><head><title>ESP32 Sensor Data</title></head><body>";
    html += "<h1>ESP32 Sensor Data</h1>";
    html += "<p>Choose between endpoints:</p>";
    html += "<ul>";
    html += "<li><a href=\"/data\">/data</a> - Get sensor data in JSON format.</li>";
    html += "<li><a href=\"/add\">/add</a> - Add current sensor readings to data.</li>";
    html += "</ul>";
    html += "</body></html>";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.print(html);
    Serial.println("Sent HTML page.");
  }
}

bool readSensors() {
  float temp = envSensor.readTemperature();
  float pres = envSensor.readPressure();
  float hum = envSensor.readHumidity();

  bool success = false;

  if (!isnan(temp)) {
    currentTemperature = temp;
    success = true;
    Serial.printf("Temp: %.1f°C ", currentTemperature);
  } else {
    Serial.print("Temp: N/A ");
  }

  if (!isnan(pres)) {
    currentPressure = pres;
    success = true;
    Serial.printf("Pres: %.1f hPa ", currentPressure);
  } else {
    Serial.print("Pres: N/A ");
  }

  if (!isnan(hum)) {
    currentHumidity = hum;
    success = true;
    Serial.printf("Hum: %.1f%%\n", currentHumidity);
  } else {
    Serial.println("Hum: N/A");
  }

  return success;
}

void logDataToSerial() {
  Serial.println("--- Logging Sensor Readings ---");
  for (int i = 0; i < readingsCount; i++) {
    int index = (currentReadingIndex + bufferSize - readingsCount + i) % bufferSize;
    Serial.printf("Time: %s, Temp: %.2f°C, Press: %.2fhPa, Hum: %.2f%%\n", 
                  readingsBuffer[index].timestamp,
                  readingsBuffer[index].temperature, 
                  readingsBuffer[index].pressure, 
                  readingsBuffer[index].humidity);
  }
  Serial.println("--- End of Log ---");
}

void addManualReading() {
  if (readSensors()) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeString[20];
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
      strncpy(readingsBuffer[currentReadingIndex].timestamp, timeString, sizeof(readingsBuffer[currentReadingIndex].timestamp));
    }
    else {
      snprintf(readingsBuffer[currentReadingIndex].timestamp, sizeof(readingsBuffer[currentReadingIndex].timestamp), "ms:%lu", millis());
    }

    readingsBuffer[currentReadingIndex].temperature = currentTemperature;
    readingsBuffer[currentReadingIndex].pressure = currentPressure;
    readingsBuffer[currentReadingIndex].humidity = currentHumidity;

    currentReadingIndex = (currentReadingIndex + 1) % bufferSize;
    if (readingsCount < bufferSize) readingsCount++;

    Serial.println("Added a new sensor reading via /add.");
  }
  else {
    Serial.println("Failed to read sensors. Manual reading not added.");
  }
}
