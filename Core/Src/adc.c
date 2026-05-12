/**
  * @file    adc.c
  * @brief   ADC1 initialization and knob reading
  */
#include "adc.h"

ADC_HandleTypeDef hadc1;

static uint16_t adc_raw_latest = 0;
static uint8_t adc_initialized = 0;

uint8_t ADC_IsKnobConnected(void)
{
    return 1;
}

void MX_ADC1_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }

    ADC1->CR1 = 0;
    ADC1->CR2 = ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;

    ADC1->SQR1 = 0;
    ADC1->SQR2 = 0;
    ADC1->SQR3 = 1;

    ADC1->SMPR1 = 0;
    ADC1->SMPR2 = 7 << 3;

    ADC1->CR2 |= ADC_CR2_ADON;
    for (volatile uint32_t d = 0; d < 1000; d++);

    {
        uint32_t tick = HAL_GetTick();
        ADC1->CR2 |= ADC_CR2_RSTCAL;
        while (ADC1->CR2 & ADC_CR2_RSTCAL) {
            if ((HAL_GetTick() - tick) > 100) break;
        }

        tick = HAL_GetTick();
        ADC1->CR2 |= ADC_CR2_CAL;
        while (ADC1->CR2 & ADC_CR2_CAL) {
            if ((HAL_GetTick() - tick) > 100) break;
        }
    }

    adc_initialized = 1;
}

uint16_t ADC_ReadKnob(void)
{
    if (!adc_initialized) return 2048;

    ADC1->CR2 |= ADC_CR2_SWSTART;

    uint32_t tickstart = HAL_GetTick();
    while (!(ADC1->SR & ADC_SR_EOC)) {
        if ((HAL_GetTick() - tickstart) > 50) {
            return 2048;
        }
    }

    uint16_t val = (uint16_t)(ADC1->DR & 0xFFF);

    ADC1->SR = 0;

    adc_raw_latest = val;

    return val;
}

uint16_t ADC_GetRawLatest(void)
{
    return adc_raw_latest;
}
