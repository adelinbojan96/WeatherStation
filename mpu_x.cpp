#include "mpu_x.h"

#include <stdint.h>

void MPUx::init() {
  // Perform soft reset
  wakeUpDevice(deviceAddress, IMU_REG_PWR_MGMT_1, IMU_VAL_PWR_MGMT_1_RESET);

  // Enable bypass mode
  i2cWriteToRegister(deviceAddress, IMU_REG_INT_BYP_CFG,
                     IMU_VAL_REG_INT_BYP_CFG_BYP_EN);
  delay(100);

  // Enable measurements on device
  uint8_t currPwrMgmt =
      i2cReadByteFromRegister(deviceAddress, IMU_REG_PWR_MGMT_1);
  uint8_t newPwrMgmt = currPwrMgmt & ~0x40;
  i2cWriteToRegister(deviceAddress, IMU_REG_PWR_MGMT_1, newPwrMgmt);

  delay(200);

  selfCalibrate();
  delay(100);

  // Set the sample rate divider to 5
  i2cWriteToRegister(deviceAddress, IMU_REG_SMP_RAT_CFG, 0x05);
  delay(100);
}

void MPUx::selfCalibrate() {
  // Enable digital low pass filter for the gyroscope
  uint8_t currGyroCfg =
      i2cReadByteFromRegister(deviceAddress, IMU_REG_GYRO_CFG);
  uint8_t newGyroCfg = currGyroCfg & 0xFC;
  i2cWriteToRegister(deviceAddress, IMU_REG_GYRO_CFG, newGyroCfg);
  delay(100);

  // Set gyroscope low pass filter to the lowest noise setting
  uint8_t currConfReg = i2cReadByteFromRegister(deviceAddress, IMU_REG_GEN_CFG);
  uint8_t newConfReg = currConfReg & 0xF8;  // Set the last 3 bits to 0
  newConfReg |= 0x06;                       // Set the last 3 bits to 110
  i2cWriteToRegister(deviceAddress, IMU_REG_GEN_CFG, newConfReg);
  delay(100);

  // Set Accl & Gyro range
  uint8_t currAclCfg = i2cReadByteFromRegister(deviceAddress, IMU_REG_ACL1_CFG);
  uint8_t newAclCfg = currAclCfg & 0xE7;
  newAclCfg |= (0 << 3);
  i2cWriteToRegister(deviceAddress, IMU_REG_ACL1_CFG, newAclCfg);
  delay(100);

  currGyroCfg = i2cReadByteFromRegister(deviceAddress, IMU_REG_GYRO_CFG);
  newGyroCfg = currGyroCfg & 0xE7;
  newGyroCfg |= (0 << 3);
  i2cWriteToRegister(deviceAddress, IMU_REG_GYRO_CFG, newGyroCfg);
  delay(100);

  // Enable accelerometer low pass filter & set to lowest noise
  uint8_t currAcl2Cfg =
      i2cReadByteFromRegister(deviceAddress, IMU_REG_ACL2_CFG);
  uint8_t newAcl2Cfg = currAcl2Cfg & ~8;
  i2cWriteToRegister(deviceAddress, IMU_REG_ACL2_CFG, newAcl2Cfg);
  delay(100);

  currAcl2Cfg = i2cReadByteFromRegister(deviceAddress, IMU_REG_ACL2_CFG);
  newAcl2Cfg = currAclCfg & 0xF8;
  newAcl2Cfg |= 0x06;
  i2cWriteToRegister(deviceAddress, IMU_REG_ACL2_CFG, newAcl2Cfg);

  delay(200);

  // Get an average from 100 samples to determine a minimal offset
  floatThreeVals initialOffsetAccl{0.0, 0.0, 0.0};
  floatThreeVals initialOffsetGyro{0.0, 0.0, 0.0};

  for (int i = 0; i < 100; i++) {
    floatThreeVals acclCurr = getThreeValsRaw(
        IMU_REG_ACCL_VALS_X, IMU_REG_ACCL_VALS_Y, IMU_REG_ACCL_VALS_Z);
    floatThreeVals gyroCurr = getThreeValsRaw(
        IMU_REG_GYRO_VALS_X, IMU_REG_GYRO_VALS_Y, IMU_REG_GYRO_VALS_Z);

    initialOffsetAccl.x += acclCurr.x;
    initialOffsetAccl.y += acclCurr.y;
    initialOffsetAccl.z += acclCurr.z;

    initialOffsetGyro.x += gyroCurr.x;
    initialOffsetGyro.y += gyroCurr.y;
    initialOffsetGyro.z += gyroCurr.z;
    delay(5);
  }

  acclOffset.x = initialOffsetAccl.x / 100.0f;
  acclOffset.y = initialOffsetAccl.y / 100.0f;
  acclOffset.z = initialOffsetAccl.z / 100.0f - 16384.0f;

  gyroOffset.x = initialOffsetGyro.x / 100.0f;
  gyroOffset.y = initialOffsetGyro.y / 100.0f;
  gyroOffset.z = initialOffsetGyro.z / 100.0f;
}

MPUx::floatThreeVals MPUx::getThreeValsRaw(uint8_t xAddr, uint8_t yAddr,
                                           uint8_t zAddr) {
  // Read raw data
  int16_t xRaw = (int16_t)i2cReadWordFromRegister(deviceAddress, xAddr);
  int16_t yRaw = (int16_t)i2cReadWordFromRegister(deviceAddress, yAddr);
  int16_t zRaw = (int16_t)i2cReadWordFromRegister(deviceAddress, zAddr);

  return {(float)xRaw, (float)yRaw, (float)zRaw};
}

MPUx::floatThreeVals MPUx::getThreeValsComp(uint8_t xAddr, uint8_t yAddr,
                                            uint8_t zAddr,
                                            floatThreeVals offset) {
  floatThreeVals retVal{0.0f, 0.0f, 0.0f};
  const floatThreeVals adc = getThreeValsRaw(xAddr, yAddr, zAddr);

  retVal.x = adc.x - offset.x;
  retVal.y = adc.y - offset.y;
  retVal.z = adc.z - offset.z;

  return retVal;
}

MPUx::floatThreeVals MPUx::getAcclVals() {
  floatThreeVals retVal{0.0f, 0.0f, 0.0f};
  const floatThreeVals acclComp =
      getThreeValsComp(IMU_REG_ACCL_VALS_X, IMU_REG_ACCL_VALS_Y,
                       IMU_REG_ACCL_VALS_Z, acclOffset);

  retVal.x = acclComp.x / 16384.0f;
  retVal.y = acclComp.y / 16384.0f;
  retVal.z = acclComp.z / 16384.0f;

  return retVal;
}

MPUx::floatThreeVals MPUx::getGyroVals() {
  floatThreeVals retVal{0.0f, 0.0f, 0.0f};
  const floatThreeVals gyroComp =
      getThreeValsComp(IMU_REG_GYRO_VALS_X, IMU_REG_GYRO_VALS_Y,
                       IMU_REG_GYRO_VALS_Z, gyroOffset);

  retVal.x = gyroComp.x * 250.0f / 16384.0f;
  retVal.y = gyroComp.y * 250.0f / 16384.0f;
  retVal.z = gyroComp.z * 250.0f / 16384.0f;

  return retVal;
}

void MPUx::printMPUData() {
  floatThreeVals accl = getAcclVals();
  floatThreeVals gyro = getGyroVals();
  Serial.println("|----------------------= MPU-6500 =----------------------|");
  Serial.printf("| Accelerometer (g) >>> x: %+07.2f y: %+07.2f z: %+07.2f |\n",
                accl.x, accl.y, accl.z);
  Serial.printf("| Gyroscope (°/sec) >>> x: %+07.2f y: %+07.2f z: %+07.2f |\n",
                gyro.x, gyro.y, gyro.z);
}