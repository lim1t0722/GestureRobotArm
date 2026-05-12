#include "stm32f1xx_hal.h"

HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc)
{
    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc->Instance->CR1 = 0;
    hadc->Instance->CR2 = hadc->Init.DataAlign;

    if (hadc->Init.ScanConvMode == ADC_SCAN_ENABLE) {
        hadc->Instance->CR1 |= ADC_CR1_SCAN;
    }
    if (hadc->Init.ContinuousConvMode == ENABLE) {
        hadc->Instance->CR2 |= ADC_CR2_CONT;
    }
    if (hadc->Init.ExternalTrigConv == ADC_SOFTWARE_START) {
        hadc->Instance->CR2 |= ADC_CR2_EXTSEL;
    }

    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *sConfig)
{
    hadc->Instance->SQR3 = sConfig->Channel & 0x1F;
    hadc->Instance->SMPR2 = (sConfig->SamplingTime << (3 * (sConfig->Channel & 0x09)));
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc)
{
    hadc->Instance->CR2 |= ADC_CR2_ADON;
    hadc->Instance->CR2 |= ADC_CR2_SWSTART;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc, uint32_t Timeout)
{
    uint32_t tickstart = HAL_GetTick();
    while (!(hadc->Instance->SR & ADC_SR_EOC)) {
        if ((HAL_GetTick() - tickstart) > Timeout) return HAL_TIMEOUT;
    }
    return HAL_OK;
}

uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc)
{
    return hadc->Instance->DR;
}

HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc)
{
    hadc->Instance->CR2 &= ~ADC_CR2_ADON;
    return HAL_OK;
}
