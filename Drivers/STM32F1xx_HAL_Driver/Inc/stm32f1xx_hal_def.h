#ifndef __STM32F1xx_HAL_DEF_H
#define __STM32F1xx_HAL_DEF_H

#include <stdint.h>
#include "stm32f1xx.h"

typedef struct __DMA_HandleTypeDef DMA_HandleTypeDef;

typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

typedef enum {
    HAL_UNLOCKED = 0x00U,
    HAL_LOCKED   = 0x01U
} HAL_LockTypeDef;

#define HAL_MAX_DELAY 0xFFFFFFFFU
#define HAL_IS_BIT_SET(REG, BIT) (((REG) & (BIT)) != 0U)
#define HAL_IS_BIT_CLR(REG, BIT) (((REG) & (BIT)) == 0U)

#define UNUSED(X) (void)X

#define __HAL_LOCK(__HANDLE__)           \
  do{                                    \
    if((__HANDLE__)->Lock == HAL_LOCKED) \
    {                                    \
      return HAL_BUSY;                   \
    }                                    \
    else                                 \
    {                                    \
      (__HANDLE__)->Lock = HAL_LOCKED;   \
    }                                    \
  }while(0U)

#define __HAL_UNLOCK(__HANDLE__)         \
  do{                                    \
    (__HANDLE__)->Lock = HAL_UNLOCKED;   \
  }while(0U)

#endif
