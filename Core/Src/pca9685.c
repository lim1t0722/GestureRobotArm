/**
  * @file    pca9685.c
  * @brief   PCA9685 16-channel PWM driver implementation
  *          Uses safe I2C communication with error tracking
  */
#include "pca9685.h"
#include "uart_debug.h"

static uint8_t pca9685_consecutive_errors = 0;

#define PCA_MAX_CONSECUTIVE_ERRORS  5

static uint8_t PCA9685_WriteReg(uint8_t reg, uint8_t data)
{
    HAL_StatusTypeDef ret = I2C_SafeMemWrite(PCA9685_ADDR, reg,
                          I2C_MEMADD_SIZE_8BIT, &data, 1, I2C_TIMEOUT_MS);
    if (ret != HAL_OK) {
        pca9685_consecutive_errors++;
        if (pca9685_consecutive_errors >= PCA_MAX_CONSECUTIVE_ERRORS) {
            I2C_BusRecovery();
            pca9685_consecutive_errors = 0;
        }
        return 1;
    }
    pca9685_consecutive_errors = 0;
    return 0;
}

static uint8_t PCA9685_ReadReg(uint8_t reg, uint8_t *data)
{
    HAL_StatusTypeDef ret = I2C_SafeMemRead(PCA9685_ADDR, reg,
                         I2C_MEMADD_SIZE_8BIT, data, 1, I2C_TIMEOUT_MS);
    if (ret != HAL_OK) {
        pca9685_consecutive_errors++;
        if (pca9685_consecutive_errors >= PCA_MAX_CONSECUTIVE_ERRORS) {
            I2C_BusRecovery();
            pca9685_consecutive_errors = 0;
        }
        return 1;
    }
    pca9685_consecutive_errors = 0;
    return 0;
}

uint8_t PCA9685_Init(void)
{
    uint8_t mode1;

    if (PCA9685_ReadReg(PCA9685_REG_MODE1, &mode1) != 0) {
        DBG_PRINT("[PCA9685] Init: read MODE1 FAILED\r\n");
        return 1;
    }
    DBG_PRINT("[PCA9685] Init: MODE1=0x%02X\r\n", mode1);

    if (PCA9685_WriteReg(PCA9685_REG_MODE1, 0x00) != 0) {
        DBG_PRINT("[PCA9685] Init: wake FAILED\r\n");
        return 1;
    }
    HAL_Delay(10);

    if (PCA9685_WriteReg(PCA9685_REG_MODE1, 0x10) != 0) return 1;

    if (PCA9685_WriteReg(PCA9685_REG_PRESCALE, 121) != 0) return 1;

    if (PCA9685_WriteReg(PCA9685_REG_MODE1, 0x20) != 0) return 1;
    HAL_Delay(5);

    if (PCA9685_WriteReg(PCA9685_REG_MODE1, 0xA0) != 0) return 1;

    if (PCA9685_WriteReg(PCA9685_REG_MODE2, 0x04) != 0) return 1;

    PCA9685_SetServoAngle(SERVO_BASE_CH, SERVO_ANGLE_MID);
    PCA9685_SetServoAngle(SERVO_ARM1_CH, SERVO_ANGLE_MID);
    PCA9685_SetServoAngle(SERVO_ARM2_CH, SERVO_ANGLE_MID);

    DBG_PRINT("[PCA9685] Init OK\r\n");

    return 0;
}

void PCA9685_SetPWMFreq(uint8_t freq)
{
    uint8_t prescale = (uint8_t)((25000000U / (4096U * freq)) - 1);
    uint8_t old_mode;

    if (PCA9685_ReadReg(PCA9685_REG_MODE1, &old_mode) != 0) return;

    uint8_t new_mode = (old_mode & 0x7F) | 0x10;
    if (PCA9685_WriteReg(PCA9685_REG_MODE1, new_mode) != 0) return;
    if (PCA9685_WriteReg(PCA9685_REG_PRESCALE, prescale) != 0) return;
    if (PCA9685_WriteReg(PCA9685_REG_MODE1, old_mode) != 0) return;
    HAL_Delay(5);

    uint8_t restart_mode = (old_mode & 0x7F) | 0x80;
    PCA9685_WriteReg(PCA9685_REG_MODE1, restart_mode);
}

void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off)
{
    if (channel > 15) return;

    uint8_t buf[4];
    buf[0] = on & 0xFF;
    buf[1] = (on >> 8) & 0x0F;
    buf[2] = off & 0xFF;
    buf[3] = (off >> 8) & 0x0F;

    HAL_StatusTypeDef ret = I2C_SafeMemWrite(PCA9685_ADDR,
                      PCA9685_REG_LED0_ON_L + 4 * channel,
                      I2C_MEMADD_SIZE_8BIT, buf, 4, I2C_TIMEOUT_MS);
    if (ret != HAL_OK) {
        pca9685_consecutive_errors++;
        if (pca9685_consecutive_errors >= PCA_MAX_CONSECUTIVE_ERRORS) {
            I2C_BusRecovery();
            pca9685_consecutive_errors = 0;
        }
    } else {
        pca9685_consecutive_errors = 0;
    }
}

void PCA9685_SetServoAngle(uint8_t channel, uint8_t angle)
{
    if (channel > 15) return;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    uint16_t off_count = SERVO_MIN_COUNT +
                         ((uint16_t)angle * (SERVO_MAX_COUNT - SERVO_MIN_COUNT)) / SERVO_ANGLE_MAX;

    PCA9685_SetPWM(channel, 0, off_count);
}

void PCA9685_SetServoAngleF(uint8_t channel, float angle)
{
    if (channel > 15) return;
    if (angle < 0) angle = 0;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    uint16_t off_count = SERVO_MIN_COUNT +
                         (uint16_t)(angle * (float)(SERVO_MAX_COUNT - SERVO_MIN_COUNT) / SERVO_ANGLE_MAX);

    PCA9685_SetPWM(channel, 0, off_count);
}

void PCA9685_Sleep(void)
{
    uint8_t mode1;
    if (PCA9685_ReadReg(PCA9685_REG_MODE1, &mode1) != 0) return;
    mode1 |= 0x10;
    PCA9685_WriteReg(PCA9685_REG_MODE1, mode1);
}

void PCA9685_Wake(void)
{
    uint8_t mode1;
    if (PCA9685_ReadReg(PCA9685_REG_MODE1, &mode1) != 0) return;
    mode1 &= ~0x10;
    mode1 |= 0x20;
    PCA9685_WriteReg(PCA9685_REG_MODE1, mode1);
    HAL_Delay(1);
}

uint8_t PCA9685_SelfTest(void)
{
    uint8_t err = 0;
    uint8_t val;

    if (PCA9685_ReadReg(PCA9685_REG_MODE1, &val) != 0) {
        DBG_PRINT("[PCA9685] SelfTest: I2C read MODE1 FAILED\r\n");
        return 0x01;
    }
    DBG_PRINT("[PCA9685] MODE1=0x%02X (expect 0xA0)\r\n", val);
    if (val != 0xA0) {
        err |= 0x04;
    }

    if (PCA9685_ReadReg(PCA9685_REG_PRESCALE, &val) != 0) {
        DBG_PRINT("[PCA9685] SelfTest: I2C read PRESCALE FAILED\r\n");
        return 0x01;
    }
    DBG_PRINT("[PCA9685] PRESCALE=%u (expect 121 for 50Hz)\r\n", val);
    if (val != 121) {
        err |= 0x02;
    }

    if (PCA9685_ReadReg(PCA9685_REG_MODE2, &val) != 0) {
        DBG_PRINT("[PCA9685] SelfTest: I2C read MODE2 FAILED\r\n");
        return 0x01;
    }
    DBG_PRINT("[PCA9685] MODE2=0x%02X (expect 0x04)\r\n", val);

    DBG_PRINT("[PCA9685] SelfTest %s\r\n", err ? "FAIL" : "PASS");
    return err;
}

uint8_t PCA9685_IsOnline(void)
{
    uint8_t val;
    if (PCA9685_ReadReg(PCA9685_REG_MODE1, &val) != 0) {
        return 0;
    }
    return 1;
}
