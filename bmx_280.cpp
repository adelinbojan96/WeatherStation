#include "bmx_280.h"
#include <Wire.h>

// Initialize the BME280 sensor
bool BMx280::init() {
  // Reset the device
  wakeUpDevice(deviceAddress, BME_REG_RESET, BME_VAL_RESET_SOFT);
  delay(200);

  // Read and set compensation values
  setCompensationValues();

  // Configure Humidity Oversampling
  uint8_t humCtrlRegVal = 0x05;  // humidity oversampling x16
  i2cWriteToRegister(deviceAddress, BME_REG_CONTROLHUMID, humCtrlRegVal);

  // Configure Sensor Settings
  uint8_t confRegVal = 0x00;     // Config Register (standby time, filter, etc.)
  uint8_t ctrlRegVal = 0xB7;     // temp & press oversample x16, normal mode
  i2cWriteToRegister(deviceAddress, BME_REG_CONFIG, confRegVal);
  i2cWriteToRegister(deviceAddress, BME_REG_CONTROL, ctrlRegVal);

  delay(200);

  // Verify Sensor ID
  Wire.beginTransmission(deviceAddress);
  Wire.write(BME_REG_ID);
  Wire.endTransmission(false);
  Wire.requestFrom(deviceAddress, (uint8_t)1);
  if (Wire.available()) {
    uint8_t id = Wire.read();
    Serial.print("BME280 ID: ");
    Serial.println(id, HEX);
    if (id != 0x60) { // 0x60 is a common ID for BME280
      Serial.println("BME280 sensor not found!");
      return false;
    }
  } else {
    Serial.println("No response from BME280 sensor!");
    return false;
  }

  // Optionally, you can add more verification steps here

  return true; // Initialization successful
}

void BMx280::setCompensationValues() {
  populateTemperatureCompensationValues();
  populatePressureCompensationValues();
  populateHumidityCompensationValues();
}

void BMx280::populateTemperatureCompensationValues() {
  compVals.T1 = i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_T1);
  compVals.T2 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_T2);
  compVals.T3 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_T3);
}

void BMx280::populatePressureCompensationValues() {
  compVals.P1 = i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P1);
  compVals.P2 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P2);
  compVals.P3 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P3);
  compVals.P4 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P4);
  compVals.P5 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P5);
  compVals.P6 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P6);
  compVals.P7 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P7);
  compVals.P8 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P8);
  compVals.P9 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_P9);
}

void BMx280::populateHumidityCompensationValues() {
  compVals.H1 = i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H1);
  compVals.H2 = (int16_t)i2cReadWordFromRegisterLE(deviceAddress, BME_REG_COMP_H2);
  compVals.H3 = i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H3);
  compVals.H4 = (int16_t)(((int8_t)i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H4) << 4) |
               (i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H4 + 1) & 0xF));
  compVals.H5 = (int16_t)(((int8_t)i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H5 + 1) << 4) |
               (i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H5) >> 4));
  compVals.H6 = (int8_t)i2cReadByteFromRegister(deviceAddress, BME_REG_COMP_H6);
}

int32_t BMx280::getTFine() {
  int32_t adcT = i2cReadThreeBytesFromRegister(deviceAddress, BME_REG_TEMPDATA);
  if (adcT == 0x800000) {
    return 0;
  }
  adcT >>= 4;

  int32_t var1, var2;
  var1 = (int32_t)((adcT / 8) - ((int32_t)compVals.T1 * 2));
  var1 = (var1 * ((int32_t)compVals.T2)) / 2048;
  var2 = (int32_t)((adcT / 16) - ((int32_t)compVals.T1));
  var2 = (((var2 * var2) / 4096) * ((int32_t)compVals.T3)) / 16384;

  return var1 + var2;
}

float BMx280::getTemperature() {
  int32_t tempFine = getTFine();
  if (tempFine == 0) {
    return NAN; // Indicate failure
  }
  int32_t T = (tempFine * 5 + 128) / 256;
  return (float)T / 100.0;
}

float BMx280::getPressure() {
  int32_t tFine = getTFine();
  if (tFine == 0) {
    return NAN; // Indicate failure
  }
  int32_t adcP = i2cReadThreeBytesFromRegister(deviceAddress, BME_REG_PRESSUREDATA);
  if (adcP == 0x800000) {
    return NAN; // Indicate failure
  }
  adcP >>= 4;

  int64_t var1, var2, P;
  var1 = ((int64_t)tFine) - 128000;
  var2 = var1 * var1 * (int64_t)compVals.P6;
  var2 = var2 + ((var1 * (int64_t)compVals.P5) << 17);
  var2 = var2 + (((int64_t)compVals.P4) << 35);
  var1 = ((var1 * var1 * (int64_t)compVals.P3) >> 8) + ((var1 * (int64_t)compVals.P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)compVals.P1) >> 33;

  if (var1 == 0) {
    return NAN; // Avoid division by zero
  }

  P = 1048576 - adcP;
  P = (((P << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)compVals.P9) * (P >> 13) * (P >> 13)) >> 25;
  var2 = (((int64_t)compVals.P8) * P) >> 19;

  P = ((P + var1 + var2) >> 8) + (((int64_t)compVals.P7) << 4);

  return (float)P / 256.0;
}

float BMx280::getHumidity() {
  int32_t tFine = getTFine();
  if (tFine == 0) {
    return NAN; // Indicate failure
  }
  int32_t adcH = i2cReadWordFromRegister(deviceAddress, BME_REG_HUMIDDATA);

  if (adcH == 0x8000) {
    return NAN; // Indicate failure
  }

  int32_t var1 = tFine - ((int32_t)76800);
  int32_t var2 = (int32_t)(adcH * 16384);
  int32_t var3 = (int32_t)(((int32_t)compVals.H4) * 1048576);
  int32_t var4 = ((int32_t)compVals.H5) * var1;
  int32_t var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
  var2 = (var1 * ((int32_t)compVals.H6)) / 1024;
  var3 = (var1 * ((int32_t)compVals.H3)) / 2048;
  var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
  var2 = ((var4 * ((int32_t)compVals.H2)) + 8192) / 16384;
  var3 = var5 * var2;
  var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
  var5 = var3 - ((var4 * ((int32_t)compVals.H1)) / 16);
  var5 = (var5 < 0 ? 0 : var5);
  var5 = (var5 > 419430400 ? 419430400 : var5);
  uint32_t H = (uint32_t)(var5 / 4096);
  return (float)H / 1024.0;
}

void BMx280::printBMEData() {
  Serial.println("|--------= BME280 =--------|");
  Serial.printf("| Temperature: %.2f ˚C    |\n", getTemperature());
  Serial.printf("|    Pressure: %.2f hPa  |\n", getPressure());
  Serial.printf("|    Humidity: %.2f %%     |\n", getHumidity());
}
