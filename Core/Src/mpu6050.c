/**
  * @file    mpu6050.c
  * @brief   MPU6050 driver with enhanced complementary filter
  *          - Adaptive alpha based on gyro magnitude
  *          - Second-order low-pass on accel angles
  *          - Robust error handling and data validation
  */
#include "mpu6050.h"
#include "uart_debug.h"

static float gyro_offset_x = 0.0f;
static float gyro_offset_y = 0.0f;
static float gyro_offset_z = 0.0f;
static float angle_roll  = 0.0f;
static float angle_pitch = 0.0f;
static float angle_yaw   = 0.0f;
static uint32_t last_time = 0;
static uint8_t mpu_consecutive_errors = 0;
static uint8_t mpu_data_valid = 0;
static uint16_t mpu_total_errors = 0;
static uint16_t mpu_total_reads = 0;
static uint8_t mpu_fail_step = 0;
static uint16_t mpu_i2c_sr1 = 0;
static uint16_t mpu_i2c_sr2 = 0;

static float accel_roll_lpf  = 0.0f;
static float accel_pitch_lpf = 0.0f;
static float accel_roll_lpf2  = 0.0f;
static float accel_pitch_lpf2 = 0.0f;

#define MPU_MAX_CONSECUTIVE_ERRORS  5
#define MPU_ACCEL_MAG_MIN          0.5f
#define MPU_ACCEL_MAG_MAX          2.5f

#define MPU_ACCEL_LPF_ALPHA        0.4f
#define MPU_GYRO_HIGH_THRESH       50.0f
#define MPU_ALPHA_LOW              0.90f
#define MPU_ALPHA_HIGH             0.97f

uint16_t MPU6050_GetErrorCount(void)
{
    return mpu_total_errors;
}

uint16_t MPU6050_GetReadCount(void)
{
    return mpu_total_reads;
}

uint8_t MPU6050_IsDataValid(void)
{
    return mpu_data_valid;
}

uint8_t MPU6050_GetFailStep(void)
{
    return mpu_fail_step;
}

uint16_t MPU6050_GetI2CStatus(void)
{
    return (mpu_i2c_sr1 << 8) | (mpu_i2c_sr2 & 0xFF);
}

float MPU6050_GetTemp(void)
{
    MPU6050_RawData_t raw;
    if (MPU6050_ReadRawData(&raw) != 0) return 0.0f;
    return raw.temp;
}

static uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    HAL_StatusTypeDef ret = I2C_SafeMemWrite(MPU6050_ADDR, reg,
                          I2C_MEMADD_SIZE_8BIT, &data, 1, I2C_TIMEOUT_MS);
    if (ret != HAL_OK) {
        mpu_consecutive_errors++;
        mpu_total_errors++;
        if (mpu_consecutive_errors >= MPU_MAX_CONSECUTIVE_ERRORS) {
            I2C_BusRecovery();
            HAL_Delay(5);
            I2C_SafeMemWrite(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x80}, 1, I2C_TIMEOUT_MS);
            HAL_Delay(10);
            mpu_consecutive_errors = 0;
        }
        return 1;
    }
    mpu_consecutive_errors = 0;
    return 0;
}

static uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef ret = I2C_SafeMemRead(MPU6050_ADDR, reg,
                         I2C_MEMADD_SIZE_8BIT, buf, len, I2C_TIMEOUT_MS);
    if (ret != HAL_OK) {
        mpu_consecutive_errors++;
        mpu_total_errors++;
        if (mpu_consecutive_errors >= MPU_MAX_CONSECUTIVE_ERRORS) {
            I2C_BusRecovery();
            HAL_Delay(5);
            I2C_SafeMemWrite(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x80}, 1, I2C_TIMEOUT_MS);
            HAL_Delay(10);
            mpu_consecutive_errors = 0;
        }
        return 1;
    }
    mpu_consecutive_errors = 0;
    return 0;
}

uint8_t MPU6050_Init(void)
{
    uint8_t check;
    HAL_StatusTypeDef ret;

    mpu_fail_step = 0;

    DBG_PRINT("[MPU6050] Init start, addr=0x%02X\r\n", MPU6050_ADDR);

    if (!I2C_IsBusIdle()) {
        mpu_fail_step = 1;
        mpu_i2c_sr1 = I2C1->SR1;
        mpu_i2c_sr2 = I2C1->SR2;
        DBG_PRINT("[MPU6050] Bus not idle, performing recovery (SR1=0x%04X SR2=0x%04X)\r\n",
                  mpu_i2c_sr1, mpu_i2c_sr2);
        I2C_BusRecovery();
        HAL_Delay(10);
    }

    mpu_fail_step = 2;
    ret = I2C_SafeMemRead(MPU6050_ADDR, MPU6050_REG_WHO_AM_I,
                          I2C_MEMADD_SIZE_8BIT, &check, 1, I2C_TIMEOUT_MS);
    if (ret != HAL_OK) {
        mpu_i2c_sr1 = I2C1->SR1;
        mpu_i2c_sr2 = I2C1->SR2;
        DBG_PRINT("[MPU6050] WHO_AM_I read FAILED, ret=%d SR1=0x%04lX SR2=0x%04lX\r\n",
                  ret, (uint32_t)I2C1->SR1, (uint32_t)I2C1->SR2);

        {
            uint8_t scan_addrs[16];
            uint8_t scan_count = I2C_ScanBus(scan_addrs, 16);
            DBG_PRINT("[MPU6050] Bus scan: %u device(s) found:", scan_count);
            for (uint8_t i = 0; i < scan_count; i++) {
                DBG_PRINT(" 0x%02X", scan_addrs[i]);
            }
            DBG_PRINT("\r\n");
        }
        return 1;
    }
    DBG_PRINT("[MPU6050] WHO_AM_I=0x%02X (expect 0x68/0x70/0x72)\r\n", check);
    if (check != 0x68 && check != 0x70 && check != 0x72) {
        mpu_fail_step = 3;
        mpu_i2c_sr1 = check;
        mpu_i2c_sr2 = 0;
        DBG_PRINT("[MPU6050] WHO_AM_I mismatch!\r\n");
        return 1;
    }

    mpu_fail_step = 4;
    if (MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x80) != 0) {
        mpu_i2c_sr1 = I2C1->SR1;
        mpu_i2c_sr2 = I2C1->SR2;
        return 1;
    }
    HAL_Delay(100);

    mpu_fail_step = 5;
    if (MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01) != 0) {
        mpu_i2c_sr1 = I2C1->SR1;
        mpu_i2c_sr2 = I2C1->SR2;
        return 1;
    }
    HAL_Delay(50);

    mpu_fail_step = 6;
    if (MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_2, 0x00) != 0) return 1;

    mpu_fail_step = 7;
    if (MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x02) != 0) return 1;

    mpu_fail_step = 8;
    if (MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x04) != 0) return 1;

    mpu_fail_step = 9;
    if (MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, MPU6050_GYRO_FS_500) != 0) return 1;

    mpu_fail_step = 10;
    if (MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_FS_4) != 0) return 1;

    mpu_fail_step = 11;
    if (MPU6050_WriteReg(MPU6050_REG_INT_ENABLE, 0x00) != 0) return 1;

    mpu_fail_step = 12;
    if (MPU6050_WriteReg(MPU6050_REG_USER_CTRL, 0x00) != 0) return 1;

    mpu_fail_step = 13;
    if (MPU6050_WriteReg(MPU6050_REG_FIFO_EN, 0x00) != 0) return 1;

    HAL_Delay(100);

    mpu_fail_step = 14;
    {
        MPU6050_RawData_t test_raw;
        uint8_t test_ok = 0;
        for (int i = 0; i < 10; i++) {
            if (MPU6050_ReadRawData(&test_raw) == 0) {
                test_ok = 1;
                break;
            }
            HAL_Delay(10);
        }
        if (!test_ok) {
            DBG_PRINT("[MPU6050] Post-init read test FAILED\r\n");
            return 1;
        }
        DBG_PRINT("[MPU6050] Post-init read OK: ax=%d ay=%d az=%d\r\n",
                  test_raw.accel_x, test_raw.accel_y, test_raw.accel_z);
    }

    MPU6050_Calibrate();
    last_time = HAL_GetTick();

    {
        MPU6050_RawData_t init_raw;
        if (MPU6050_ReadRawData(&init_raw) == 0) {
            float ax = init_raw.accel_x / MPU6050_ACCEL_SENS_4;
            float ay = init_raw.accel_y / MPU6050_ACCEL_SENS_4;
            float az = init_raw.accel_z / MPU6050_ACCEL_SENS_4;
            angle_roll = atan2f(ay, az) * 180.0f / (float)M_PI;
            angle_pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / (float)M_PI;
            accel_roll_lpf = angle_roll;
            accel_pitch_lpf = angle_pitch;
            accel_roll_lpf2 = angle_roll;
            accel_pitch_lpf2 = angle_pitch;
            DBG_PRINT("[MPU6050] Init angles: roll=%.1f pitch=%.1f\r\n",
                      angle_roll, angle_pitch);
        }
    }

    angle_yaw = 0.0f;
    mpu_data_valid = 1;
    mpu_consecutive_errors = 0;
    mpu_total_errors = 0;
    mpu_total_reads = 0;
    mpu_fail_step = 0;

    DBG_PRINT("[MPU6050] Init OK, offsets: gx=%.1f gy=%.1f gz=%.1f\r\n",
              gyro_offset_x, gyro_offset_y, gyro_offset_z);

    return 0;
}

void MPU6050_Calibrate(void)
{
    int32_t gx = 0, gy = 0, gz = 0;
    int32_t valid_count = 0;
    MPU6050_RawData_t raw;

    DBG_PRINT("[MPU6050] Calibrating...\r\n");

    HAL_Delay(200);

    for (int i = 0; i < 200; i++) {
        if (MPU6050_ReadRawData(&raw) == 0) {
            gx += raw.gyro_x;
            gy += raw.gyro_y;
            gz += raw.gyro_z;
            valid_count++;
        }
        HAL_Delay(2);
    }

    if (valid_count > 50) {
        gyro_offset_x = (float)gx / (float)valid_count;
        gyro_offset_y = (float)gy / (float)valid_count;
        gyro_offset_z = (float)gz / (float)valid_count;
        DBG_PRINT("[MPU6050] Calibration done: %ld/%d samples, offsets=%.1f,%.1f,%.1f\r\n",
                  (uint32_t)valid_count, 200, gyro_offset_x, gyro_offset_y, gyro_offset_z);
    } else {
        DBG_PRINT("[MPU6050] Calibration WARNING: only %ld/%d valid samples\r\n",
                  (uint32_t)valid_count, 200);
    }
}

uint8_t MPU6050_ReadRawData(MPU6050_RawData_t *raw)
{
    uint8_t buf[14];

    if (MPU6050_ReadReg(MPU6050_REG_ACCEL_XOUT_H, buf, 14) != 0) {
        return 1;
    }

    raw->accel_x = (int16_t)((buf[0] << 8) | buf[1]);
    raw->accel_y = (int16_t)((buf[2] << 8) | buf[3]);
    raw->accel_z = (int16_t)((buf[4] << 8) | buf[5]);
    raw->temp    = (int16_t)((buf[6] << 8) | buf[7]) / 340.0f + 36.53f;
    raw->gyro_x  = (int16_t)((buf[8]  << 8) | buf[9]);
    raw->gyro_y  = (int16_t)((buf[10] << 8) | buf[11]);
    raw->gyro_z  = (int16_t)((buf[12] << 8) | buf[13]);

    return 0;
}

static uint8_t MPU6050_ValidateData(MPU6050_RawData_t *raw)
{
    float ax = raw->accel_x / MPU6050_ACCEL_SENS_4;
    float ay = raw->accel_y / MPU6050_ACCEL_SENS_4;
    float az = raw->accel_z / MPU6050_ACCEL_SENS_4;
    float accel_mag = sqrtf(ax * ax + ay * ay + az * az);

    if (accel_mag < MPU_ACCEL_MAG_MIN || accel_mag > MPU_ACCEL_MAG_MAX) {
        return 0;
    }

    if (raw->accel_x == 0 && raw->accel_y == 0 && raw->accel_z == 0) {
        return 0;
    }

    if (raw->accel_x == -1 && raw->accel_y == -1 && raw->accel_z == -1) {
        return 0;
    }

    return 1;
}

uint8_t MPU6050_CalculateAngle(MPU6050_RawData_t *raw, MPU6050_Angle_t *angle)
{
    if (!MPU6050_ValidateData(raw)) {
        mpu_data_valid = 0;
        angle->roll = angle_roll;
        angle->pitch = angle_pitch;
        angle->yaw = angle_yaw;
        return 2;
    }

    mpu_data_valid = 1;

    float ax = raw->accel_x / MPU6050_ACCEL_SENS_4;
    float ay = raw->accel_y / MPU6050_ACCEL_SENS_4;
    float az = raw->accel_z / MPU6050_ACCEL_SENS_4;

    float gx = (raw->gyro_x - gyro_offset_x) / MPU6050_GYRO_SENS_500;
    float gy = (raw->gyro_y - gyro_offset_y) / MPU6050_GYRO_SENS_500;
    float gz = (raw->gyro_z - gyro_offset_z) / MPU6050_GYRO_SENS_500;

    float accel_roll  = atan2f(ay, az) * 180.0f / (float)M_PI;
    float accel_pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / (float)M_PI;

    accel_roll_lpf = MPU_ACCEL_LPF_ALPHA * accel_roll +
                     (1.0f - MPU_ACCEL_LPF_ALPHA) * accel_roll_lpf;
    accel_pitch_lpf = MPU_ACCEL_LPF_ALPHA * accel_pitch +
                      (1.0f - MPU_ACCEL_LPF_ALPHA) * accel_pitch_lpf;

    accel_roll_lpf2 = MPU_ACCEL_LPF_ALPHA * accel_roll_lpf +
                      (1.0f - MPU_ACCEL_LPF_ALPHA) * accel_roll_lpf2;
    accel_pitch_lpf2 = MPU_ACCEL_LPF_ALPHA * accel_pitch_lpf +
                       (1.0f - MPU_ACCEL_LPF_ALPHA) * accel_pitch_lpf2;

    uint32_t now = HAL_GetTick();
    float dt = (now - last_time) / 1000.0f;
    last_time = now;

    if (dt <= 0.001f || dt > 0.1f) {
        dt = MPU6050_DT;
    }

    float gyro_mag = sqrtf(gx * gx + gy * gy);
    float alpha;
    if (gyro_mag > MPU_GYRO_HIGH_THRESH) {
        alpha = MPU_ALPHA_HIGH;
    } else {
        alpha = MPU_ALPHA_LOW;
    }

    angle_roll  = alpha * (angle_roll  + gx * dt) +
                  (1.0f - alpha) * accel_roll_lpf2;
    angle_pitch = alpha * (angle_pitch + gy * dt) +
                  (1.0f - alpha) * accel_pitch_lpf2;
    angle_yaw  += gz * dt;

    if (angle_roll > 180.0f)  angle_roll = 180.0f;
    if (angle_roll < -180.0f) angle_roll = -180.0f;
    if (angle_pitch > 180.0f)  angle_pitch = 180.0f;
    if (angle_pitch < -180.0f) angle_pitch = -180.0f;

    if (angle_yaw > 180.0f)  angle_yaw -= 360.0f;
    if (angle_yaw < -180.0f) angle_yaw += 360.0f;

    angle->roll  = angle_roll;
    angle->pitch = angle_pitch;
    angle->yaw   = angle_yaw;

    return 0;
}

uint8_t MPU6050_GetAngle(MPU6050_Angle_t *angle)
{
    MPU6050_RawData_t raw;

    mpu_total_reads++;

    if (MPU6050_ReadRawData(&raw) != 0) {
        mpu_data_valid = 0;
        angle->roll = angle_roll;
        angle->pitch = angle_pitch;
        angle->yaw = angle_yaw;
        return 1;
    }

    return MPU6050_CalculateAngle(&raw, angle);
}

void MPU6050_ResetYaw(void)
{
    angle_yaw = 0.0f;
}

void MPU6050_ResetAll(void)
{
    angle_roll = 0.0f;
    angle_pitch = 0.0f;
    angle_yaw = 0.0f;
    accel_roll_lpf = 0.0f;
    accel_pitch_lpf = 0.0f;
    accel_roll_lpf2 = 0.0f;
    accel_pitch_lpf2 = 0.0f;
    last_time = HAL_GetTick();
}

uint8_t MPU6050_ReInit(void)
{
    DBG_PRINT("[MPU6050] ReInit start...\r\n");

    mpu_data_valid = 0;
    mpu_consecutive_errors = 0;

    if (!I2C_IsBusIdle()) {
        DBG_PRINT("[MPU6050] ReInit: bus not idle, performing recovery\r\n");
        I2C_BusRecovery();
        HAL_Delay(10);
    }

    HAL_StatusTypeDef ret = I2C_SafeMemWrite(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1,
                          I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x80}, 1, I2C_TIMEOUT_MS);
    if (ret != HAL_OK) {
        DBG_PRINT("[MPU6050] ReInit: software reset FAILED\r\n");
        I2C_BusRecovery();
        HAL_Delay(10);

        ret = I2C_SafeMemWrite(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1,
              I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x80}, 1, I2C_TIMEOUT_MS);
        if (ret != HAL_OK) {
            DBG_PRINT("[MPU6050] ReInit: 2nd reset FAILED, giving up\r\n");
            return 1;
        }
    }
    HAL_Delay(100);

    uint8_t check;
    ret = I2C_SafeMemRead(MPU6050_ADDR, MPU6050_REG_WHO_AM_I,
                          I2C_MEMADD_SIZE_8BIT, &check, 1, I2C_TIMEOUT_MS);
    if (ret != HAL_OK || (check != 0x68 && check != 0x70 && check != 0x72)) {
        DBG_PRINT("[MPU6050] ReInit: WHO_AM_I FAILED (ret=%d val=0x%02X)\r\n", ret, check);
        return 1;
    }

    if (MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01) != 0) return 1;
    HAL_Delay(50);

    if (MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_2, 0x00) != 0) return 1;
    if (MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x02) != 0) return 1;
    if (MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x04) != 0) return 1;
    if (MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, MPU6050_GYRO_FS_500) != 0) return 1;
    if (MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_FS_4) != 0) return 1;
    if (MPU6050_WriteReg(MPU6050_REG_INT_ENABLE, 0x00) != 0) return 1;
    if (MPU6050_WriteReg(MPU6050_REG_USER_CTRL, 0x00) != 0) return 1;
    if (MPU6050_WriteReg(MPU6050_REG_FIFO_EN, 0x00) != 0) return 1;

    HAL_Delay(100);

    {
        MPU6050_RawData_t test_raw;
        uint8_t test_ok = 0;
        for (int i = 0; i < 10; i++) {
            if (MPU6050_ReadRawData(&test_raw) == 0) {
                test_ok = 1;
                break;
            }
            HAL_Delay(10);
        }
        if (!test_ok) {
            DBG_PRINT("[MPU6050] ReInit: post-init read test FAILED\r\n");
            return 1;
        }
    }

    MPU6050_Calibrate();

    last_time = HAL_GetTick();

    {
        MPU6050_RawData_t init_raw;
        if (MPU6050_ReadRawData(&init_raw) == 0) {
            float ax = init_raw.accel_x / MPU6050_ACCEL_SENS_4;
            float ay = init_raw.accel_y / MPU6050_ACCEL_SENS_4;
            float az = init_raw.accel_z / MPU6050_ACCEL_SENS_4;
            angle_roll = atan2f(ay, az) * 180.0f / (float)M_PI;
            angle_pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / (float)M_PI;
            accel_roll_lpf = angle_roll;
            accel_pitch_lpf = angle_pitch;
            accel_roll_lpf2 = angle_roll;
            accel_pitch_lpf2 = angle_pitch;
        }
    }

    angle_yaw = 0.0f;
    mpu_data_valid = 1;
    mpu_consecutive_errors = 0;

    DBG_PRINT("[MPU6050] ReInit OK\r\n");
    return 0;
}

void MPU6050_PrintDebugInfo(void)
{
    DBG_PRINT("[MPU6050] R=%.1f P=%.1f Y=%.1f err=%u/%u %s\r\n",
              angle_roll, angle_pitch, angle_yaw,
              mpu_total_errors, mpu_total_reads,
              mpu_data_valid ? "OK" : "ERR");
}
