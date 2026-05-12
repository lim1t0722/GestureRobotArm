/**
  ******************************************************************************
  * @file      startup_stm32f103xb.s
  * @brief     STM32F103C8T6 startup file for GCC toolchain
  *            64KB Flash, 20KB SRAM
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m3
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/* Start of .text section */
.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
.size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  /* External interrupts */
  .word WWDG_IRQHandler
  .word PVD_IRQHandler
  .word TAMPER_IRQHandler
  .word RTC_IRQHandler
  .word FLASH_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_IRQHandler
  .word EXTI1_IRQHandler
  .word EXTI2_IRQHandler
  .word EXTI3_IRQHandler
  .word EXTI4_IRQHandler
  .word DMA1_Channel1_IRQHandler
  .word DMA1_Channel2_IRQHandler
  .word DMA1_Channel3_IRQHandler
  .word DMA1_Channel4_IRQHandler
  .word DMA1_Channel5_IRQHandler
  .word DMA1_Channel6_IRQHandler
  .word DMA1_Channel7_IRQHandler
  .word ADC1_2_IRQHandler
  .word USB_HP_CAN1_TX_IRQHandler
  .word USB_LP_CAN1_RX0_IRQHandler
  .word CAN1_RX1_IRQHandler
  .word CAN1_SCE_IRQHandler
  .word EXTI9_5_IRQHandler
  .word TIM1_BRK_IRQHandler
  .word TIM1_UP_IRQHandler
  .word TIM1_TRG_COM_IRQHandler
  .word TIM1_CC_IRQHandler
  .word TIM2_IRQHandler
  .word TIM3_IRQHandler
  .word TIM4_IRQHandler
  .word I2C1_EV_IRQHandler
  .word I2C1_ER_IRQHandler
  .word I2C2_EV_IRQHandler
  .word I2C2_ER_IRQHandler
  .word SPI1_IRQHandler
  .word SPI2_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word USART3_IRQHandler
  .word EXTI15_10_IRQHandler
  .word RTCAlarm_IRQHandler
  .word USBWakeUp_IRQHandler

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:
  ldr   sp, =_estack

  /* Copy .data from Flash to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit

  /* Zero fill .bss */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

  /* Call system clock init */
  bl SystemInit
  /* Call main */
  bl main
  /* Loop forever if main returns */
  b .
  .size Reset_Handler, .-Reset_Handler

  .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler

  .macro DefIRQHandler name
  .weak \name
  .thumb_set \name, Default_Handler
  .endm

  DefIRQHandler WWDG_IRQHandler
  DefIRQHandler PVD_IRQHandler
  DefIRQHandler TAMPER_IRQHandler
  DefIRQHandler RTC_IRQHandler
  DefIRQHandler FLASH_IRQHandler
  DefIRQHandler RCC_IRQHandler
  DefIRQHandler EXTI0_IRQHandler
  DefIRQHandler EXTI1_IRQHandler
  DefIRQHandler EXTI2_IRQHandler
  DefIRQHandler EXTI3_IRQHandler
  DefIRQHandler EXTI4_IRQHandler
  DefIRQHandler DMA1_Channel1_IRQHandler
  DefIRQHandler DMA1_Channel2_IRQHandler
  DefIRQHandler DMA1_Channel3_IRQHandler
  DefIRQHandler DMA1_Channel4_IRQHandler
  DefIRQHandler DMA1_Channel5_IRQHandler
  DefIRQHandler DMA1_Channel6_IRQHandler
  DefIRQHandler DMA1_Channel7_IRQHandler
  DefIRQHandler ADC1_2_IRQHandler
  DefIRQHandler USB_HP_CAN1_TX_IRQHandler
  DefIRQHandler USB_LP_CAN1_RX0_IRQHandler
  DefIRQHandler CAN1_RX1_IRQHandler
  DefIRQHandler CAN1_SCE_IRQHandler
  DefIRQHandler EXTI9_5_IRQHandler
  DefIRQHandler TIM1_BRK_IRQHandler
  DefIRQHandler TIM1_UP_IRQHandler
  DefIRQHandler TIM1_TRG_COM_IRQHandler
  DefIRQHandler TIM1_CC_IRQHandler
  DefIRQHandler TIM2_IRQHandler
  DefIRQHandler TIM3_IRQHandler
  DefIRQHandler TIM4_IRQHandler
  DefIRQHandler I2C1_EV_IRQHandler
  DefIRQHandler I2C1_ER_IRQHandler
  DefIRQHandler I2C2_EV_IRQHandler
  DefIRQHandler I2C2_ER_IRQHandler
  DefIRQHandler SPI1_IRQHandler
  DefIRQHandler SPI2_IRQHandler
  DefIRQHandler USART1_IRQHandler
  DefIRQHandler USART2_IRQHandler
  DefIRQHandler USART3_IRQHandler
  DefIRQHandler EXTI15_10_IRQHandler
  DefIRQHandler RTCAlarm_IRQHandler
  DefIRQHandler USBWakeUp_IRQHandler
