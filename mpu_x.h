#ifndef MPU_UTILS_HH
#define MPU_UTILS_HH

#include "i2c_utils.h"

class MPUx {
 public:
  struct floatThreeVals {
    float x;
    float y;
    float z;
  };

 private:
  static uint8_t constexpr IMU_REG_GEN_CFG = 0x1A;
  static uint8_t constexpr IMU_REG_GYRO_CFG = 0x1B;
  static uint8_t constexpr IMU_REG_ACL1_CFG = 0x1C;
  static uint8_t constexpr IMU_REG_ACL2_CFG = 0x1D;
  static uint8_t constexpr IMU_REG_PWR_MGMT_1 = 0x6B;
  static uint8_t constexpr IMU_REG_WHO_AM_I = 0x75;
  static uint8_t constexpr IMU_REG_SMP_RAT_CFG = 0x19;
  static uint8_t constexpr IMU_REG_INT_BYP_CFG = 0x37;
  static uint8_t constexpr IMU_REG_ACCL_VALS_X = 0x3B;
  static uint8_t constexpr IMU_REG_ACCL_VALS_Y = 0x3D;
  static uint8_t constexpr IMU_REG_ACCL_VALS_Z = 0x3F;
  static uint8_t constexpr IMU_REG_GYRO_VALS_X = 0x43;
  static uint8_t constexpr IMU_REG_GYRO_VALS_Y = 0x45;
  static uint8_t constexpr IMU_REG_GYRO_VALS_Z = 0x47;

  static uint8_t constexpr IMU_VAL_PWR_MGMT_1_RESET = 0x80;
  static uint8_t constexpr IMU_VAL_REG_INT_BYP_CFG_BYP_EN = 0x02;

  const uint8_t deviceAddress;

  floatThreeVals acclOffset = {0.0, 0.0, 0.0};
  floatThreeVals gyroOffset = {0.0, 0.0, 0.0};

  void selfCalibrate();
  floatThreeVals getThreeValsRaw(uint8_t xAddr, uint8_t yAddr, uint8_t zAddr);
  floatThreeVals getThreeValsComp(uint8_t xAddr, uint8_t yAddr, uint8_t zAddr,
                                  floatThreeVals offset);

 public:
  MPUx(const uint8_t deviceAddress) : deviceAddress(deviceAddress) {}
  void init();
  floatThreeVals getAcclVals();
  floatThreeVals getGyroVals();
  void printMPUData();
};

#endif
