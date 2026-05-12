#ifndef __STM32F103xB_H
#define __STM32F103xB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define __IO    volatile
#define __I     volatile const
#define __O     volatile

typedef enum { RESET = 0, SET = !RESET } FlagStatus, ITStatus;
typedef enum { DISABLE = 0, ENABLE = !DISABLE } FunctionalState;
typedef enum { ERROR = 0, SUCCESS = !ERROR } ErrorStatus;
typedef enum { LOW = 0, HIGH = !LOW } GPIO_PinState;

#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

#define PERIPH_BASE           ((uint32_t)0x40000000)
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x20000)

#define GPIOA_BASE            (APB2PERIPH_BASE + 0x0800)
#define GPIOB_BASE            (APB2PERIPH_BASE + 0x0C00)
#define GPIOC_BASE            (APB2PERIPH_BASE + 0x1000)
#define GPIOD_BASE            (APB2PERIPH_BASE + 0x1400)

#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400)

#define ADC1_BASE             (APB2PERIPH_BASE + 0x2400)

#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000)

#define RCC_BASE              (AHBPERIPH_BASE + 0x1000)
#define FLASH_BASE            (AHBPERIPH_BASE + 0x2000)
#define DMA1_BASE             (AHBPERIPH_BASE + 0x0000)

typedef struct {
    __IO uint32_t CR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t APB2RSTR;
    __IO uint32_t APB1RSTR;
    __IO uint32_t AHBENR;
    __IO uint32_t APB2ENR;
    __IO uint32_t APB1ENR;
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
} RCC_TypeDef;

#define RCC                   ((RCC_TypeDef *)RCC_BASE)

typedef struct {
    __IO uint32_t CRL;
    __IO uint32_t CRH;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t BRR;
    __IO uint32_t LCKR;
} GPIO_TypeDef;

#define GPIOA                 ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                 ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC                 ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD                 ((GPIO_TypeDef *)GPIOD_BASE)

typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t OAR1;
    __IO uint32_t OAR2;
    __IO uint32_t DR;
    __IO uint32_t SR1;
    __IO uint32_t SR2;
    __IO uint32_t CCR;
    __IO uint32_t TRISE;
} I2C_TypeDef;

#define I2C1                  ((I2C_TypeDef *)I2C1_BASE)

typedef struct {
    __IO uint32_t SR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMPR1;
    __IO uint32_t SMPR2;
    __IO uint32_t JOFR1;
    __IO uint32_t JOFR2;
    __IO uint32_t JOFR3;
    __IO uint32_t JOFR4;
    __IO uint32_t HTR;
    __IO uint32_t LTR;
    __IO uint32_t SQR1;
    __IO uint32_t SQR2;
    __IO uint32_t SQR3;
    __IO uint32_t JSQR;
    __IO uint32_t JDR1;
    __IO uint32_t JDR2;
    __IO uint32_t JDR3;
    __IO uint32_t JDR4;
    __IO uint32_t DR;
} ADC_TypeDef;

#define ADC1                  ((ADC_TypeDef *)ADC1_BASE)

typedef struct {
    __IO uint32_t ACR;
    __IO uint32_t KEYR;
    __IO uint32_t OPTKEYR;
    __IO uint32_t SR;
    __IO uint32_t CR;
    __IO uint32_t AR;
    __IO uint32_t RESERVED;
    __IO uint32_t OBR;
    __IO uint32_t WRPR;
} FLASH_TypeDef;

#define FLASH                 ((FLASH_TypeDef *)FLASH_BASE)

typedef struct {
    __IO uint32_t KR;
    __IO uint32_t PR;
    __IO uint32_t RLR;
    __IO uint32_t SR;
} IWDG_TypeDef;

#define IWDG                  ((IWDG_TypeDef *)IWDG_BASE)

#define IWDG_PR_PR_0          ((uint8_t)0x00)
#define IWDG_PR_PR_2          ((uint8_t)0x02)

#define RCC_CR_HSION          ((uint32_t)0x00000001)
#define RCC_CR_HSEON          ((uint32_t)0x00010000)
#define RCC_CR_HSERDY         ((uint32_t)0x00020000)
#define RCC_CR_CSSON          ((uint32_t)0x00080000)
#define RCC_CR_PLLON          ((uint32_t)0x01000000)
#define RCC_CR_PLLRDY         ((uint32_t)0x02000000)

#define RCC_CFGR_SW           ((uint32_t)0x00000003)
#define RCC_CFGR_SW_PLL       ((uint32_t)0x00000002)
#define RCC_CFGR_SWS          ((uint32_t)0x0000000C)
#define RCC_CFGR_SWS_PLL      ((uint32_t)0x00000008)
#define RCC_CFGR_HPRE         ((uint32_t)0x000000F0)
#define RCC_CFGR_PPRE1        ((uint32_t)0x00000700)
#define RCC_CFGR_PPRE2        ((uint32_t)0x00003800)
#define RCC_CFGR_PLLSRC       ((uint32_t)0x00010000)
#define RCC_CFGR_PLLMULL      ((uint32_t)0x003C0000)
#define RCC_CFGR_PLLMULL9     ((uint32_t)0x001C0000)

#define RCC_CFGR_HPRE_DIV1    ((uint32_t)0x00000000)
#define RCC_CFGR_PPRE1_DIV2   ((uint32_t)0x00000400)
#define RCC_CFGR_PPRE2_DIV1   ((uint32_t)0x00000000)

#define FLASH_ACR_PRFTBE      ((uint32_t)0x00000010)
#define FLASH_ACR_LATENCY_2   ((uint32_t)0x00000002)

#define GPIO_PIN_0            ((uint16_t)0x0001)
#define GPIO_PIN_1            ((uint16_t)0x0002)
#define GPIO_PIN_2            ((uint16_t)0x0004)
#define GPIO_PIN_6            ((uint16_t)0x0040)
#define GPIO_PIN_7            ((uint16_t)0x0080)
#define GPIO_PIN_10           ((uint16_t)0x0400)
#define GPIO_PIN_11           ((uint16_t)0x0800)
#define GPIO_PIN_13           ((uint16_t)0x2000)

#define GPIO_MODE_INPUT        0x00u
#define GPIO_MODE_OUTPUT_PP    0x01u
#define GPIO_MODE_AF_OD        0x02u
#define GPIO_MODE_OUTPUT_OD    0x03u
#define GPIO_MODE_ANALOG       0x04u

#define GPIO_NOPULL           0x00u
#define GPIO_PULLUP           0x01u
#define GPIO_PULLDOWN         0x02u

#define GPIO_SPEED_FREQ_LOW    0x00u
#define GPIO_SPEED_FREQ_HIGH   0x03u

#define GPIO_PIN_RESET         0u
#define GPIO_PIN_SET           1u

#define I2C_DUTYCYCLE_2       0x00000000u
#define I2C_ADDRESSINGMODE_7BIT 0x00004000u
#define I2C_DUALADDRESS_DISABLE 0x00000000u
#define I2C_GENERALCALL_DISABLE 0x00000000u
#define I2C_NOSTRETCH_DISABLE   0x00000000u
#define I2C_MEMADD_SIZE_8BIT    0x00000001u

#define ADC_SCAN_DISABLE       0x00000000u
#define ADC_DATAALIGN_RIGHT    0x00000000u
#define ADC_REGULAR_RANK_1     0x00000001u
#define ADC_SAMPLETIME_71CYCLES_5 0x00000060u
#define ADC_SAMPLETIME_239CYCLES_5 0x00000070u
#define ADC_CHANNEL_1          0x00000001u
#define ADC_SOFTWARE_START     0x00000000u
#define ADC_SCAN_DISABLE       0x00000000u
#define ADC_SCAN_ENABLE        0x00000001u
#define ADC_DATAALIGN_RIGHT    0x00000000u
#define ADC_DATAALIGN_LEFT     0x00000800u
#define ADC_CHANNEL_1          0x00000001u
#define ADC_REGULAR_RANK_1     0x00000001u
#define ADC_SAMPLETIME_71CYCLES_5 0x00000060u
#define ADC_SAMPLETIME_239CYCLES_5 0x00000070u

#define ADC_SR_EOC             ((uint32_t)0x00000002)
#define ADC_CR1_SCAN           ((uint32_t)0x00000100)
#define ADC_CR2_ADON           ((uint32_t)0x00000001)
#define ADC_CR2_CONT           ((uint32_t)0x00000002)
#define ADC_CR2_CAL            ((uint32_t)0x00000004)
#define ADC_CR2_RSTCAL         ((uint32_t)0x00000008)
#define ADC_CR2_EXTTRIG        ((uint32_t)0x00100000)
#define ADC_CR2_SWSTART        ((uint32_t)0x00400000)
#define ADC_CR2_EXTSEL         ((uint32_t)0x000E0000)

#define I2C_CR1_PE             ((uint32_t)0x00000001)
#define I2C_CR1_START          ((uint32_t)0x00000100)
#define I2C_CR1_STOP           ((uint32_t)0x00000200)
#define I2C_CR1_ACK            ((uint32_t)0x00000400)
#define I2C_SR1_SB             ((uint32_t)0x00000001)
#define I2C_SR1_ADDR           ((uint32_t)0x00000002)
#define I2C_SR1_TXE            ((uint32_t)0x00000080)
#define I2C_SR1_RXNE           ((uint32_t)0x00000040)
#define I2C_SR1_BTF            ((uint32_t)0x00000004)

#define SysTick_CTRL_CLKSOURCE_Msk   (1UL << 2)
#define SysTick_CTRL_TICKINT_Msk     (1UL << 1)
#define SysTick_CTRL_ENABLE_Msk      (1UL << 0)

typedef enum {
    NonMaskableInt_IRQn         = -14,
    HardFault_IRQn             = -13,
    MemoryManagement_IRQn      = -12,
    BusFault_IRQn              = -11,
    UsageFault_IRQn            = -10,
    SVCall_IRQn                = -5,
    DebugMonitor_IRQn          = -4,
    PendSV_IRQn                = -2,
    SysTick_IRQn               = -1,
    WWDG_IRQn                  = 0,
    PVD_IRQn                   = 1,
    TAMPER_IRQn                = 2,
    RTC_IRQn                   = 3,
    FLASH_IRQn                 = 4,
    RCC_IRQn                   = 5,
    EXTI0_IRQn                 = 6,
    EXTI1_IRQn                 = 7,
    EXTI2_IRQn                 = 8,
    EXTI3_IRQn                 = 9,
    EXTI4_IRQn                 = 10,
    DMA1_Channel1_IRQn         = 11,
    DMA1_Channel2_IRQn         = 12,
    DMA1_Channel3_IRQn         = 13,
    DMA1_Channel4_IRQn         = 14,
    DMA1_Channel5_IRQn         = 15,
    DMA1_Channel6_IRQn         = 16,
    DMA1_Channel7_IRQn         = 17,
    ADC1_2_IRQn                = 18,
    USB_HP_CAN1_TX_IRQn        = 19,
    USB_LP_CAN1_RX0_IRQn       = 20,
    CAN1_RX1_IRQn              = 21,
    CAN1_SCE_IRQn              = 22,
    EXTI9_5_IRQn               = 23,
    TIM1_BRK_IRQn              = 24,
    TIM1_UP_IRQn               = 25,
    TIM1_TRG_COM_IRQn          = 26,
    TIM1_CC_IRQn               = 27,
    TIM2_IRQn                  = 28,
    TIM3_IRQn                  = 29,
    TIM4_IRQn                  = 30,
    I2C1_EV_IRQn               = 31,
    I2C1_ER_IRQn               = 32,
    I2C2_EV_IRQn               = 33,
    I2C2_ER_IRQn               = 34,
    SPI1_IRQn                  = 35,
    SPI2_IRQn                  = 36,
    USART1_IRQn                = 37,
    USART2_IRQn                = 38,
    USART3_IRQn                = 39,
    EXTI15_10_IRQn             = 40,
    RTCAlarm_IRQn              = 41,
    USBWakeUp_IRQn             = 42
} IRQn_Type;

#define __HAL_RCC_GPIOA_CLK_ENABLE()  (RCC->APB2ENR |= (1 << 2))
#define __HAL_RCC_GPIOB_CLK_ENABLE()  (RCC->APB2ENR |= (1 << 3))
#define __HAL_RCC_GPIOC_CLK_ENABLE()  (RCC->APB2ENR |= (1 << 4))
#define __HAL_RCC_AFIO_CLK_ENABLE()   (RCC->APB2ENR |= (1 << 0))
#define __HAL_RCC_I2C1_CLK_ENABLE()   (RCC->APB1ENR |= (1 << 21))
#define __HAL_RCC_ADC1_CLK_ENABLE()   (RCC->APB2ENR |= (1 << 9))

#ifndef HSE_VALUE
#define HSE_VALUE             8000000U
#endif
#ifndef HSI_VALUE
#define HSI_VALUE             8000000U
#endif

#ifdef __cplusplus
}
#endif

static inline void NVIC_SystemReset(void)
{
    SCB->AIRCR = 0x05FA0004U;
    while (1) {}
}

#endif
