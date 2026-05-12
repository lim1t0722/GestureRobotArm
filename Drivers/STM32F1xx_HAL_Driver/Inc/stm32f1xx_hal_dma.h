#ifndef __STM32F1xx_HAL_DMA_H
#define __STM32F1xx_HAL_DMA_H

#include "stm32f1xx_hal_def.h"

struct __DMA_HandleTypeDef {
    void          *Instance;
    HAL_LockTypeDef Lock;
    uint32_t       State;
    void          *Parent;
    void          *XferCpltCallback;
    void          *XferHalfCpltCallback;
    void          *XferErrorCallback;
    uint32_t       ErrorCode;
    uint32_t       StreamBaseAddress;
    uint32_t       StreamIndex;
};

#endif
