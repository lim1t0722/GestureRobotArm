#ifndef __STM32F1xx_HAL_CORTEX_H
#define __STM32F1xx_HAL_CORTEX_H

#include "stm32f1xx_hal_def.h"

void HAL_NVIC_SetPriority(uint32_t IRQn, uint32_t PreemptPriority, uint32_t SubPriority);
void HAL_NVIC_EnableIRQ(uint32_t IRQn);
void HAL_NVIC_DisableIRQ(uint32_t IRQn);
uint32_t HAL_NVIC_GetPriority(uint32_t IRQn);

#endif
