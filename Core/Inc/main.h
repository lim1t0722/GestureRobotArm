/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "stm32f1xx_hal.h"

/* Debug configuration - define to enable UART debug output */
#define DEBUG

/* Exported functions prototypes */
void Error_Handler(void);

/* Private defines */
#define KEY_GPIO_Port       GPIOA
#define KEY_Pin             GPIO_PIN_0

#define DBG_KEY_GPIO_Port   GPIOA
#define DBG_KEY_Pin         GPIO_PIN_2

#define LED_GPIO_Port       GPIOC
#define LED_Pin             GPIO_PIN_13

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
