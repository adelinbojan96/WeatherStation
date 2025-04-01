#ifndef I2C_UTILS_H
#define I2C_UTILS_H

#include <Arduino.h>
#include <Wire.h>

#include <cstdlib>

// -=| I2C |=-
const int I2C_SDA = 9;
const int I2C_SCL = 10;

void i2cWriteToRegister(const uint8_t deviceAddress,
                        const uint8_t registerAddress, const uint8_t value);

uint8_t i2cReadByteFromRegister(const uint8_t deviceAddress,
                                const uint8_t registerAddress);

uint16_t i2cReadWordFromRegister(const uint8_t deviceAddress,
                                 const uint8_t registerAddress);

uint16_t i2cReadWordFromRegisterLE(const uint8_t deviceAddress,
                                   const uint8_t registerAddress);

uint32_t i2cReadThreeBytesFromRegister(const uint8_t deviceAddress,
                                       const uint8_t registerAddress);

void wakeUpDevice(const uint8_t deviceAddress, const uint8_t resetAddress,
                  const uint8_t resetValue);

void identifyDevice(const uint8_t deviceAddress,
                    const uint8_t whoAmIRegAddress);

void printAllI2CDevicesOnBus();

#endif  // I2C_UTILS_H
