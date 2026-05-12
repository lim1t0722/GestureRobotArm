#include "stm32f1xx_hal.h"

void HAL_RCC_DeInit(void) {}
uint32_t HAL_RCC_GetSysClockFreq(void) { return SystemCoreClock; }
uint32_t HAL_RCC_GetHCLKFreq(void) { return SystemCoreClock; }
uint32_t HAL_RCC_GetPCLK1Freq(void) { return SystemCoreClock >> 1; }
uint32_t HAL_RCC_GetPCLK2Freq(void) { return SystemCoreClock; }
