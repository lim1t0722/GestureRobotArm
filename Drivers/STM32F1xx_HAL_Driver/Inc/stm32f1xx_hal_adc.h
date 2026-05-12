#ifndef __STM32F1xx_HAL_ADC_H
#define __STM32F1xx_HAL_ADC_H

#include "stm32f1xx_hal_def.h"

typedef struct {
    uint32_t ScanConvMode;
    uint32_t ContinuousConvMode;
    uint32_t DiscontinuousConvMode;
    uint32_t NbrOfDiscConversion;
    uint32_t ExternalTrigConv;
    uint32_t DataAlign;
    uint32_t NbrOfConversion;
} ADC_InitTypeDef;

typedef struct {
    ADC_TypeDef *Instance;
    ADC_InitTypeDef Init;
    DMA_HandleTypeDef *DMA_Handle;
    HAL_LockTypeDef Lock;
    uint32_t State;
    uint32_t ErrorCode;
} ADC_HandleTypeDef;

typedef struct {
    uint32_t Channel;
    uint32_t Rank;
    uint32_t SamplingTime;
} ADC_ChannelConfTypeDef;

HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *sConfig);
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc, uint32_t Timeout);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc);

#endif
