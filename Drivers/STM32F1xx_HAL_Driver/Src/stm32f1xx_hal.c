#include "stm32f1xx_hal.h"

static volatile uint32_t uwTick = 0;
static HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;

HAL_StatusTypeDef HAL_Init(void)
{
    HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0U);
    HAL_InitTick(TICK_INT_PRIORITY);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DeInit(void)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    SysTick->LOAD = (SystemCoreClock / (1000U / uwTickFreq)) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    return HAL_OK;
}

void HAL_IncTick(void)
{
    uwTick += uwTickFreq;
}

uint32_t HAL_GetTick(void)
{
    return uwTick;
}

void HAL_Delay(uint32_t Delay)
{
    uint32_t tickstart = HAL_GetTick();
    uint32_t wait = Delay;
    while ((HAL_GetTick() - tickstart) < wait) {}
}

void HAL_SuspendTick(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void HAL_ResumeTick(void)
{
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}
