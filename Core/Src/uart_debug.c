/**
  * @file    uart_debug.c
  * @brief   UART1 debug output via register-direct operation
  *          Bypasses HAL to minimize overhead for debug printing.
  *          Only active when DEBUG macro is defined.
  */
#include "uart_debug.h"
#include <stdarg.h>

#ifdef DEBUG

/*
 * Register-level USART1 access for lightweight debug output.
 * This avoids HAL overhead and remains functional even if
 * HAL I2C/SPI handlers consume significant stack space.
 *
 * PA9  = USART1_TX (AF push-pull, 50MHz)
 * BRR  = 0x0271   -> 115200 baud @ PCLK2=72MHz
 * CR1  = UE+TE    -> USART enable + transmitter enable
 */
#define USART1_BASE_DBG     0x40013800UL
#define USART1_SR           (*((volatile uint32_t *)(USART1_BASE_DBG + 0x00)))
#define USART1_DR           (*((volatile uint32_t *)(USART1_BASE_DBG + 0x04)))
#define USART1_SR_TXE       ((uint32_t)0x00000080)

static void UART_Debug_Init_GPIO(void)
{
    RCC->APB2ENR |= (1 << 2) | (1 << 14);

    GPIOA->CRH = (GPIOA->CRH & ~(0x0FUL << 4)) | (0x0BUL << 4);

    volatile uint32_t *usart1_brr = (volatile uint32_t *)(USART1_BASE_DBG + 0x08);
    *usart1_brr = 0x0271;

    volatile uint32_t *usart1_cr1 = (volatile uint32_t *)(USART1_BASE_DBG + 0x0C);
    *usart1_cr1 = (1 << 13) | (1 << 3);
}

void UART_Debug_Init(void)
{
    UART_Debug_Init_GPIO();
}

static void UART_Debug_PutChar(char ch)
{
    while (!(USART1_SR & USART1_SR_TXE));
    USART1_DR = (uint32_t)ch;
}

void UART_Debug_Printf(const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    char *p = buf;
    while (*p) {
        UART_Debug_PutChar(*p++);
    }
}

#else

void UART_Debug_Init(void) {}
void UART_Debug_Printf(const char *fmt, ...) { (void)fmt; }

#endif
