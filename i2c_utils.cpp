#include "i2c_utils.h"

void i2cWriteToRegister(const uint8_t deviceAddress,
                        const uint8_t registerAddress, const uint8_t value) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t i2cReadByteFromRegister(const uint8_t deviceAddress,
                                const uint8_t registerAddress) {
  uint8_t value = 0;

  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(false);
  Wire.requestFrom(deviceAddress, (uint8_t)1);
  while (Wire.available()) {
    value = Wire.read();
  }

  return value;
}

uint16_t i2cReadWordFromRegister(const uint8_t deviceAddress,
                                 const uint8_t registerAddress) {
  uint16_t value = 0;
  uint8_t buffer[2];
  uint8_t idx = 0;

  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(false);
  Wire.requestFrom(deviceAddress, (uint8_t)2);

  while (Wire.available()) {
    buffer[idx++] = Wire.read();
  }

  value = (buffer[0] << 8) | buffer[1];

  return value;
}

uint16_t i2cReadWordFromRegisterLE(const uint8_t deviceAddress,
                                   const uint8_t registerAddress) {
  uint16_t value = 0;
  uint8_t buffer[2];
  uint8_t idx = 0;

  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(false);
  Wire.requestFrom(deviceAddress, (uint8_t)2);

  while (Wire.available()) {
    buffer[idx++] = Wire.read();
  }

  value = (buffer[0] << 8) | buffer[1];

  return (value >> 8) | (value << 8);
}

uint32_t i2cReadThreeBytesFromRegister(const uint8_t deviceAddress,
                                       const uint8_t registerAddress) {
  int32_t value = 0;
  uint8_t buffer[3];
  uint8_t idx = 0;

  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(false);
  Wire.requestFrom(deviceAddress, (uint8_t)3);

  while (Wire.available()) {
    buffer[idx++] = Wire.read();
  }

  value = (buffer[0] << 16) | buffer[1] << 8 | buffer[2];

  return value;
}

void wakeUpDevice(const uint8_t deviceAddress, const uint8_t resetAddress,
                  const uint8_t resetValue) {
  i2cWriteToRegister(deviceAddress, resetAddress, resetValue);
  delay(10);
}

void identifyDevice(const uint8_t deviceAddress,
                    const uint8_t whoAmIRegAddress) {
  const uint8_t responseWhoAmI =
      i2cReadByteFromRegister(deviceAddress, whoAmIRegAddress);
  Serial.printf("Device address on I²C bus: 0x%X, WHO_AM_I reg val --> 0x%X\n",
                deviceAddress, responseWhoAmI);
}

void printAllI2CDevicesOnBus() {
  uint8_t deviceCounter;

  Serial.println("Scanning for devices");
  Serial.println("--------------------");

  // Scan all addresses from 1 to 126
  for (uint8_t i = 1; i < 127; i++) {
    Wire.beginTransmission(i);

    // Check for error at address i
    const uint8_t error = Wire.endTransmission();

    if (error == 0) {

      Serial.printf("I²C device found at address 0x%02X\n", i);
      deviceCounter++;
    } else if (error == 4) {

      Serial.printf("Unknown error at address 0x%02X\n", i);
    }
  }

  if (deviceCounter == 0)
    Serial.println("No I2C devices found");
  else
    Serial.printf("Found %d device(s) in total\n", deviceCounter);
}
