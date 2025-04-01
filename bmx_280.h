#ifndef BMX_280_H
#define BMX_280_H

#include "i2c_utils.h"
#include <math.h> // Required for NAN

class BMx280 {
 private:
  // Register Addresses
  static constexpr uint8_t BME_REG_ID = 0xD0;
  static constexpr uint8_t BME_REG_RESET = 0xE0;
  static constexpr uint8_t BME_REG_CONTROLHUMID = 0xF2;
  static constexpr uint8_t BME_REG_STATUS = 0xF3;
  static constexpr uint8_t BME_REG_CONTROL = 0xF4;
  static constexpr uint8_t BME_REG_CONFIG = 0xF5;
  static constexpr uint8_t BME_REG_PRESSUREDATA = 0xF7;
  static constexpr uint8_t BME_REG_TEMPDATA = 0xFA;
  static constexpr uint8_t BME_REG_HUMIDDATA = 0xFD;

  static constexpr uint8_t BME_VAL_RESET_SOFT = 0xB6;

  // Compensation Registers
  static constexpr uint8_t BME_REG_COMP_T1 = 0x88;
  static constexpr uint8_t BME_REG_COMP_T2 = 0x8A;
  static constexpr uint8_t BME_REG_COMP_T3 = 0x8C;

  static constexpr uint8_t BME_REG_COMP_P1 = 0x8E;
  static constexpr uint8_t BME_REG_COMP_P2 = 0x90;
  static constexpr uint8_t BME_REG_COMP_P3 = 0x92;
  static constexpr uint8_t BME_REG_COMP_P4 = 0x94;
  static constexpr uint8_t BME_REG_COMP_P5 = 0x96;
  static constexpr uint8_t BME_REG_COMP_P6 = 0x98;
  static constexpr uint8_t BME_REG_COMP_P7 = 0x9A;
  static constexpr uint8_t BME_REG_COMP_P8 = 0x9C;
  static constexpr uint8_t BME_REG_COMP_P9 = 0x9E;

  static constexpr uint8_t BME_REG_COMP_H1 = 0xA1;
  static constexpr uint8_t BME_REG_COMP_H2 = 0xE1;
  static constexpr uint8_t BME_REG_COMP_H3 = 0xE3;
  static constexpr uint8_t BME_REG_COMP_H4 = 0xE4;
  static constexpr uint8_t BME_REG_COMP_H5 = 0xE5;
  static constexpr uint8_t BME_REG_COMP_H6 = 0xE7;

  // Compensation Values Structure
  struct BMx280CompVals {
    uint16_t T1;
    int16_t T2;
    int16_t T3;

    uint16_t P1;
    int16_t P2;
    int16_t P3;
    int16_t P4;
    int16_t P5;
    int16_t P6;
    int16_t P7;
    int16_t P8;
    int16_t P9;

    uint8_t H1;
    int16_t H2;
    uint8_t H3;
    int16_t H4;
    int16_t H5;
    int8_t H6;
  } compVals;

  uint8_t deviceAddress;

  // Private Methods
  void setCompensationValues();
  void populateTemperatureCompensationValues();
  void populatePressureCompensationValues();
  void populateHumidityCompensationValues();

  int32_t getTFine();
  float getTemperature();
  float getPressure(); 
  float getHumidity();

 public:
  BMx280(const uint8_t deviceAddress) : deviceAddress(deviceAddress) {}
  
  /**
   * Initializes the BME280 sensor.
   * @return true if initialization is successful, false otherwise.
   */
  bool init();
  
  void printBMEData();

  // Public Getters
  float readTemperature() { return getTemperature(); }
  float readPressure()    { return getPressure() / 100.0F; } // Convert to hPa
  float readHumidity()    { return getHumidity(); }
};

#endif
