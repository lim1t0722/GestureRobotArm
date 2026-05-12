/**
  * @file    adc.h
  * @brief   ADC driver for reading knob (potentiometer)
  */
#ifndef __ADC_H
#define __ADC_H

#include "main.h"

extern ADC_HandleTypeDef hadc1;

void MX_ADC1_Init(void);
uint16_t ADC_ReadKnob(void);
uint8_t ADC_IsKnobConnected(void);
uint16_t ADC_GetRawLatest(void);

#endif /* __ADC_H */
