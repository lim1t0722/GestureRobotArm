/**
  * @file    mpu6050.h
  * @brief   GY-521 MPU6050 6-axis motion sensor driver
  *          Enhanced complementary filter with adaptive alpha
  */
#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"
#include "i2c.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MPU6050_ADDR            0xD0

#define MPU6050_REG_PWR_MGMT_1  0x6B
#define MPU6050_REG_PWR_MGMT_2  0x6C
#define MPU6050_REG_SMPLRT_DIV  0x19
#define MPU6050_REG_CONFIG      0x1A
#define MPU6050_REG_GYRO_CONFIG 0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_INT_ENABLE  0x38
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_GYRO_XOUT_H  0x43
#define MPU6050_REG_USER_CTRL   0x6A
#define MPU6050_REG_FIFO_EN     0x23
#define MPU6050_REG_WHO_AM_I    0x75
#define MPU6050_REG_TEMP_OUT_H  0x41

#define MPU6050_GYRO_FS_250     0x00
#define MPU6050_GYRO_FS_500     0x08
#define MPU6050_GYRO_FS_1000    0x10
#define MPU6050_GYRO_FS_2000    0x18

#define MPU6050_ACCEL_FS_2      0x00
#define MPU6050_ACCEL_FS_4      0x08
#define MPU6050_ACCEL_FS_8      0x10
#define MPU6050_ACCEL_FS_16     0x18

#define MPU6050_GYRO_SENS_250   131.0f
#define MPU6050_GYRO_SENS_500   65.5f
#define MPU6050_GYRO_SENS_1000  32.8f
#define MPU6050_GYRO_SENS_2000  16.4f

#define MPU6050_ACCEL_SENS_2    16384.0f
#define MPU6050_ACCEL_SENS_4    8192.0f
#define MPU6050_ACCEL_SENS_8    4096.0f
#define MPU6050_ACCEL_SENS_16   2048.0f

#define MPU6050_DT              0.02f

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    float   temp;
} MPU6050_RawData_t;

typedef struct {
    float roll;
    float pitch;
    float yaw;
} MPU6050_Angle_t;

uint8_t MPU6050_Init(void);
uint8_t MPU6050_ReInit(void);
uint8_t MPU6050_ReadRawData(MPU6050_RawData_t *raw);
uint8_t MPU6050_CalculateAngle(MPU6050_RawData_t *raw, MPU6050_Angle_t *angle);
uint8_t MPU6050_GetAngle(MPU6050_Angle_t *angle);
void MPU6050_Calibrate(void);
void MPU6050_ResetYaw(void);
void MPU6050_ResetAll(void);
void MPU6050_PrintDebugInfo(void);
uint8_t MPU6050_IsDataValid(void);
uint16_t MPU6050_GetErrorCount(void);
uint16_t MPU6050_GetReadCount(void);
float MPU6050_GetTemp(void);
uint8_t MPU6050_GetFailStep(void);
uint16_t MPU6050_GetI2CStatus(void);

#endif /* __MPU6050_H */
