#include "stm32f1xx_hal.h"

static uint32_t I2C_GetTiming(I2C_HandleTypeDef *hi2c)
{
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t ccr = 0;

    if (hi2c->Init.ClockSpeed <= 100000) {
        ccr = pclk1 / (hi2c->Init.ClockSpeed * 2);
    } else {
        if (hi2c->Init.DutyCycle == I2C_DUTYCYCLE_2) {
            ccr = pclk1 / (hi2c->Init.ClockSpeed * 3);
        } else {
            ccr = pclk1 / (hi2c->Init.ClockSpeed * 25);
        }
        ccr |= 0x8000;
    }

    if (ccr < 4) ccr = 4;
    return ccr;
}

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
    __HAL_RCC_I2C1_CLK_ENABLE();

    hi2c->Instance->CR1 = 0;
    hi2c->Instance->CR2 = (HAL_RCC_GetPCLK1Freq() / 1000000) & 0x3F;
    hi2c->Instance->CCR = I2C_GetTiming(hi2c);
    hi2c->Instance->TRISE = (HAL_RCC_GetPCLK1Freq() / 1000000) + 1;
    hi2c->Instance->CR1 = I2C_CR1_PE;

    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
    hi2c->Instance->CR1 = 0;
    return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitFlag(I2C_TypeDef *Instance, uint32_t Flag, uint32_t Timeout)
{
    uint32_t tickstart = HAL_GetTick();
    while (!(Instance->SR1 & Flag)) {
        if ((HAL_GetTick() - tickstart) > Timeout) return HAL_TIMEOUT;
    }
    return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitFlagClear(I2C_TypeDef *Instance, uint32_t Flag, uint32_t Timeout)
{
    uint32_t tickstart = HAL_GetTick();
    while (Instance->SR1 & Flag) {
        if ((HAL_GetTick() - tickstart) > Timeout) return HAL_TIMEOUT;
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
                                     uint16_t MemAddress, uint16_t MemAddSize,
                                     uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    I2C_TypeDef *I2Cx = hi2c->Instance;

    I2Cx->CR1 |= I2C_CR1_START;
    if (I2C_WaitFlag(I2Cx, I2C_SR1_SB, Timeout) != HAL_OK) return HAL_TIMEOUT;

    I2Cx->DR = DevAddress;
    if (I2C_WaitFlag(I2Cx, I2C_SR1_ADDR, Timeout) != HAL_OK) return HAL_TIMEOUT;
    (void)I2Cx->SR2;

    if (I2C_WaitFlag(I2Cx, I2C_SR1_TXE, Timeout) != HAL_OK) return HAL_TIMEOUT;
    I2Cx->DR = (uint8_t)MemAddress;

    for (uint16_t i = 0; i < Size; i++) {
        if (I2C_WaitFlag(I2Cx, I2C_SR1_TXE, Timeout) != HAL_OK) return HAL_TIMEOUT;
        I2Cx->DR = pData[i];
    }

    if (I2C_WaitFlag(I2Cx, I2C_SR1_BTF, Timeout) != HAL_OK) return HAL_TIMEOUT;
    I2Cx->CR1 |= I2C_CR1_STOP;

    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
                                    uint16_t MemAddress, uint16_t MemAddSize,
                                    uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    I2C_TypeDef *I2Cx = hi2c->Instance;

    I2Cx->CR1 |= I2C_CR1_START;
    if (I2C_WaitFlag(I2Cx, I2C_SR1_SB, Timeout) != HAL_OK) return HAL_TIMEOUT;

    I2Cx->DR = DevAddress;
    if (I2C_WaitFlag(I2Cx, I2C_SR1_ADDR, Timeout) != HAL_OK) return HAL_TIMEOUT;
    (void)I2Cx->SR2;

    if (I2C_WaitFlag(I2Cx, I2C_SR1_TXE, Timeout) != HAL_OK) return HAL_TIMEOUT;
    I2Cx->DR = (uint8_t)MemAddress;

    if (I2C_WaitFlag(I2Cx, I2C_SR1_BTF, Timeout) != HAL_OK) return HAL_TIMEOUT;

    I2Cx->CR1 |= I2C_CR1_START;
    if (I2C_WaitFlag(I2Cx, I2C_SR1_SB, Timeout) != HAL_OK) return HAL_TIMEOUT;

    I2Cx->DR = DevAddress | 0x01;
    if (I2C_WaitFlag(I2Cx, I2C_SR1_ADDR, Timeout) != HAL_OK) return HAL_TIMEOUT;

    if (Size == 1) {
        I2Cx->CR1 &= ~I2C_CR1_ACK;
        (void)I2Cx->SR2;
        I2Cx->CR1 |= I2C_CR1_STOP;
        if (I2C_WaitFlag(I2Cx, I2C_SR1_RXNE, Timeout) != HAL_OK) return HAL_TIMEOUT;
        pData[0] = (uint8_t)I2Cx->DR;
    } else {
        I2Cx->CR1 |= I2C_CR1_ACK;
        (void)I2Cx->SR2;
        for (uint16_t i = 0; i < Size; i++) {
            if (I2C_WaitFlag(I2Cx, I2C_SR1_RXNE, Timeout) != HAL_OK) return HAL_TIMEOUT;
            pData[i] = (uint8_t)I2Cx->DR;
            if (i == Size - 2) {
                I2Cx->CR1 &= ~I2C_CR1_ACK;
            }
        }
        I2Cx->CR1 |= I2C_CR1_STOP;
    }

    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
                                           uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    return HAL_I2C_Mem_Write(hi2c, DevAddress, 0, 0, pData, Size, Timeout);
}
