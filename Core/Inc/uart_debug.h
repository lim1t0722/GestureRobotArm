#ifndef __UART_DEBUG_H
#define __UART_DEBUG_H

#include "main.h"
#include <stdio.h>

void UART_Debug_Init(void);
void UART_Debug_Printf(const char *fmt, ...);

#ifdef DEBUG
#define DBG_PRINT(fmt, ...) UART_Debug_Printf(fmt, ##__VA_ARGS__)
#else
#define DBG_PRINT(fmt, ...) ((void)0)
#endif

#endif
