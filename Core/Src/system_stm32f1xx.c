/**
  * @file    system_stm32f1xx.c
  * @brief   CMSIS Cortex-M3 Device Peripheral Access Layer System Source File
  *          Configure system clock to 72MHz (HSE 8MHz * 9 PLL)
  */
#include "stm32f1xx.h"

uint32_t SystemCoreClock = 72000000U;
const uint8_t AHBPrescTable[16U] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8U] = {0, 0, 0, 0, 1, 2, 3, 4};

#define HSE_STARTUP_TIMEOUT     0x0500U
#define PLL_STARTUP_TIMEOUT     0x0500U

/**
  * @brief  System clock configuration
  *         HSE 8MHz -> PLL *9 -> SYSCLK 72MHz
  *         Fallback to HSI 8MHz if HSE/PLL fails
  */
void SystemInit(void)
{
    uint32_t timeout;

    RCC->CR |= RCC_CR_HSION;
    RCC->CFGR = 0x00000000;
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);

    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;

    RCC->CR |= RCC_CR_HSEON;
    timeout = HSE_STARTUP_TIMEOUT;
    while (!(RCC->CR & RCC_CR_HSERDY)) {
        if (--timeout == 0) {
            SystemCoreClock = HSI_VALUE;
            return;
        }
    }

    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;

    RCC->CR |= RCC_CR_PLLON;
    timeout = PLL_STARTUP_TIMEOUT;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {
        if (--timeout == 0) {
            SystemCoreClock = HSE_VALUE;
            return;
        }
    }

    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    timeout = HSE_STARTUP_TIMEOUT;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
        if (--timeout == 0) {
            SystemCoreClock = HSE_VALUE;
            return;
        }
    }

    SystemCoreClock = 72000000U;
}

/**
  * @brief  Update SystemCoreClock variable
  */
void SystemCoreClockUpdate(void)
{
    uint32_t tmp, pllmull, pllsource;

    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp) {
        case 0x00:
            SystemCoreClock = HSI_VALUE;
            break;
        case 0x04:
            SystemCoreClock = HSE_VALUE;
            break;
        case 0x08:
            pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
            pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
            pllmull = (pllmull >> 18) + 2;

            if (pllsource == 0x00) {
                SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
            } else {
                SystemCoreClock = HSE_VALUE * pllmull;
            }
            break;
        default:
            SystemCoreClock = HSI_VALUE;
            break;
    }

    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    SystemCoreClock >>= tmp;
}
