/**
  * @file    i2c.c
  * @brief   I2C1 driver with bus scan and robust error recovery
  *          Direct register-based HAL, no state machine dependency
  */
#include "i2c.h"
#include "gpio.h"
#include "uart_debug.h"

I2C_HandleTypeDef hi2c1;

static uint16_t i2c_error_count = 0;
static uint16_t i2c_recovery_count = 0;

#define I2C_SR2_BUSY_BIT    ((uint32_t)0x00000002)
#define I2C_CR1_PE_BIT      ((uint32_t)0x00000001)
#define I2C_SR1_AF_BIT      ((uint32_t)0x0400)

uint16_t I2C_GetErrorCount(void)
{
    return i2c_error_count;
}

uint16_t I2C_GetRecoveryCount(void)
{
    return i2c_recovery_count;
}

static void I2C_FullReset(void)
{
    I2C1->CR1 &= ~I2C_CR1_PE_BIT;
    for (volatile uint32_t d = 0; d < 500; d++);
    I2C1->CR1 |= I2C_CR1_PE_BIT;
    for (volatile uint32_t d = 0; d < 500; d++);
}

static void I2C_ClearAllFlags(void)
{
    if (I2C1->SR1 & I2C_SR1_AF_BIT) {
        __IO uint32_t tmp = I2C1->SR1;
        (void)tmp;
    }
    (void)I2C1->SR2;

    I2C1->CR1 |= I2C_CR1_STOP;
    for (volatile uint32_t d = 0; d < 2000; d++);

    I2C_FullReset();
}

void MX_I2C1_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }

    HAL_Delay(1);
}

static void I2C_GPIO_RecoveryMode(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    I2C1->CR1 &= ~I2C_CR1_PE_BIT;
    for (volatile uint32_t d = 0; d < 500; d++);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
}

static void I2C_GPIO_RestoreAF(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

uint8_t I2C_IsBusIdle(void)
{
    if (I2C1->SR2 & I2C_SR2_BUSY_BIT) {
        return 0;
    }
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) != GPIO_PIN_SET) {
        return 0;
    }
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) != GPIO_PIN_SET) {
        return 0;
    }
    return 1;
}

void I2C_BusRecovery(void)
{
    i2c_recovery_count++;
    DBG_PRINT("[I2C] Bus recovery #%u\r\n", i2c_recovery_count);

    I2C_GPIO_RecoveryMode();

    HAL_Delay(1);

    for (uint8_t i = 0; i < 9; i++) {
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET) {
            break;
        }

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(1);

    I2C_GPIO_RestoreAF();

    HAL_I2C_Init(&hi2c1);

    HAL_Delay(5);

    DBG_PRINT("[I2C] Bus recovery done, SR1=0x%04lX SR2=0x%04lX idle=%u\r\n",
              (uint32_t)I2C1->SR1, (uint32_t)I2C1->SR2, I2C_IsBusIdle());
}

uint8_t I2C_ScanBus(uint8_t *found_addrs, uint8_t max_found)
{
    uint8_t count = 0;

    I2C_ClearAllFlags();
    HAL_Delay(5);

    for (uint8_t addr = 0x03; addr < 0x78; addr++) {
        uint16_t dev_addr = (uint16_t)(addr << 1);
        HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&hi2c1, dev_addr, 0,
                                                   I2C_MEMADD_SIZE_8BIT,
                                                   (uint8_t*)0, 0, 10);
        if (ret == HAL_OK) {
            if (count < max_found) {
                found_addrs[count++] = addr;
            }
            I2C1->CR1 |= I2C_CR1_STOP;
            for (volatile uint32_t d = 0; d < 200; d++);
        } else {
            I2C_ClearAllFlags();
        }

        for (volatile uint32_t d = 0; d < 100; d++);
    }

    I2C_ClearAllFlags();
    HAL_Delay(5);

    return count;
}

HAL_StatusTypeDef I2C_SafeMemWrite(uint16_t dev_addr, uint16_t mem_addr,
                                    uint16_t mem_size, uint8_t *data, uint16_t len,
                                    uint32_t timeout)
{
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&hi2c1, dev_addr, mem_addr,
                                               mem_size, data, len, timeout);
    if (ret != HAL_OK) {
        i2c_error_count++;
        I2C_ClearAllFlags();
    }
    return ret;
}

HAL_StatusTypeDef I2C_SafeMemRead(uint16_t dev_addr, uint16_t mem_addr,
                                   uint16_t mem_size, uint8_t *data, uint16_t len,
                                   uint32_t timeout)
{
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(&hi2c1, dev_addr, mem_addr,
                                              mem_size, data, len, timeout);
    if (ret != HAL_OK) {
        i2c_error_count++;
        I2C_ClearAllFlags();
    }
    return ret;
}
