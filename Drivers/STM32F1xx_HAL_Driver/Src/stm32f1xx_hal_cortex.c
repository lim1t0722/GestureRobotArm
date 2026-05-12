#include "stm32f1xx_hal.h"

void HAL_NVIC_SetPriority(uint32_t IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
    uint32_t priority = (PreemptPriority << 4) | SubPriority;
    SCB->SHP[(IRQn & 0xF) - 4] = (uint8_t)(priority << 4);
}

void HAL_NVIC_EnableIRQ(uint32_t IRQn)
{
    NVIC->ISER[IRQn >> 5] = (1UL << (IRQn & 0x1F));
}

void HAL_NVIC_DisableIRQ(uint32_t IRQn)
{
    NVIC->ICER[IRQn >> 5] = (1UL << (IRQn & 0x1F));
}

uint32_t HAL_NVIC_GetPriority(uint32_t IRQn)
{
    return NVIC->IP[IRQn];
}
