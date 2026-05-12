/**
  * @file    i2c.h
  * @brief   I2C driver for STM32F103C8T6
  *          Communicates with MPU6050 and PCA9685
  *          Includes bus scan, NACK handling, and robust recovery
  */
#ifndef __I2C_H
#define __I2C_H

#include "main.h"

#define I2C_TIMEOUT_MS          50

extern I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void);
void I2C_BusRecovery(void);
uint8_t I2C_IsBusIdle(void);
uint16_t I2C_GetErrorCount(void);
uint16_t I2C_GetRecoveryCount(void);
uint8_t I2C_ScanBus(uint8_t *found_addrs, uint8_t max_found);
HAL_StatusTypeDef I2C_SafeMemWrite(uint16_t dev_addr, uint16_t mem_addr,
                                    uint16_t mem_size, uint8_t *data, uint16_t len,
                                    uint32_t timeout);
HAL_StatusTypeDef I2C_SafeMemRead(uint16_t dev_addr, uint16_t mem_addr,
                                   uint16_t mem_size, uint8_t *data, uint16_t len,
                                   uint32_t timeout);

#endif /* __I2C_H */
