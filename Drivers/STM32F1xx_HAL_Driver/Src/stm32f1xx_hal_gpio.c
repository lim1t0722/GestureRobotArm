#include "stm32f1xx_hal.h"
#include <stddef.h>

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    uint32_t position = 0;
    uint32_t ioposition = 0x00;
    uint32_t iocurrent = 0x00;
    volatile uint32_t *configregister = NULL;
    uint32_t registeroffset = 0;
    uint32_t config = 0;

    while ((GPIO_Init->Pin >> position) != 0) {
        ioposition = 0x01UL << position;
        iocurrent = GPIO_Init->Pin & ioposition;

        if (iocurrent == ioposition) {
            if (position < 8) {
                configregister = &GPIOx->CRL;
                registeroffset = position * 4;
            } else {
                configregister = &GPIOx->CRH;
                registeroffset = (position - 8) * 4;
            }

            *configregister &= ~(0x0FUL << registeroffset);

            if (GPIO_Init->Mode == GPIO_MODE_INPUT) {
                config = 0x00;
                if (GPIO_Init->Pull == GPIO_PULLUP) {
                    config = 0x08;
                    GPIOx->BSRR = ioposition;
                } else if (GPIO_Init->Pull == GPIO_PULLDOWN) {
                    config = 0x08;
                    GPIOx->BRR = ioposition;
                }
            } else if (GPIO_Init->Mode == GPIO_MODE_OUTPUT_PP) {
                config = (GPIO_Init->Speed << 0) | 0x00;
            } else if (GPIO_Init->Mode == GPIO_MODE_OUTPUT_OD) {
                config = (GPIO_Init->Speed << 0) | 0x04;
            } else if (GPIO_Init->Mode == GPIO_MODE_AF_OD) {
                config = (GPIO_Init->Speed << 0) | 0x0C;
            } else if (GPIO_Init->Mode == GPIO_MODE_ANALOG) {
                config = 0x00;
            }

            *configregister |= (config << registeroffset);
        }
        position++;
    }
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t PinState)
{
    if (PinState != GPIO_PIN_RESET) {
        GPIOx->BSRR = GPIO_Pin;
    } else {
        GPIOx->BRR = GPIO_Pin;
    }
}

uint8_t HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    return ((GPIOx->IDR & GPIO_Pin) != 0x00u) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->ODR ^= GPIO_Pin;
}
