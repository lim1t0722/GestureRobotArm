/**
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines with fault diagnostics
  */
#include "main.h"
#include "stm32f1xx_it.h"
#include "uart_debug.h"

static void Fault_DiagnosticBlink(uint8_t code)
{
    __disable_irq();

    RCC->APB2ENR |= (1 << 4);

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);

    while (1) {
        for (uint8_t i = 0; i < code; i++) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            for (volatile uint32_t d = 0; d < 1500000; d++);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            for (volatile uint32_t d = 0; d < 1500000; d++);
        }
        for (volatile uint32_t d = 0; d < 6000000; d++);
    }
}

void NMI_Handler(void)
{
    while (1) {}
}

void HardFault_Handler(void)
{
    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;

    DBG_PRINT("\r\n!!! HARD FAULT !!!\r\n");
    DBG_PRINT("CFSR =0x%08lX\r\n", cfsr);
    DBG_PRINT("HFSR =0x%08lX\r\n", hfsr);
    DBG_PRINT("MMFAR=0x%08lX\r\n", mmfar);
    DBG_PRINT("BFAR =0x%08lX\r\n", bfar);

    uint8_t fault_code = 1;
    if (cfsr & 0x0000FFFF) fault_code = 2;
    if (cfsr & 0xFFFF0000) fault_code = 3;
    if (hfsr & 0x00000002) fault_code = 4;
    if (hfsr & 0x80000000) fault_code = 5;

    Fault_DiagnosticBlink(fault_code);
}

void MemManage_Handler(void)
{
    Fault_DiagnosticBlink(6);
}

void BusFault_Handler(void)
{
    Fault_DiagnosticBlink(7);
}

void UsageFault_Handler(void)
{
    Fault_DiagnosticBlink(8);
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
