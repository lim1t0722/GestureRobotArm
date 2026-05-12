#ifndef __CORE_CM3_H
#define __CORE_CM3_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __STATIC_INLINE
#define __STATIC_INLINE static inline
#endif

#ifndef __IO
#define __IO    volatile
#endif
#ifndef __I
#define __I     volatile const
#endif
#ifndef __O
#define __O     volatile
#endif

#define __NVIC_PRIO_BITS          4
#define __Vendor_SysTickConfig    0
#define __CM3_REV                 0x0200U

typedef union {
    struct {
        uint32_t nPRIV:1;
        uint32_t SPSEL:1;
        uint32_t FPCA:1;
        uint32_t _reserved0:29;
    } b;
    uint32_t w;
} CONTROL_Type;

typedef union {
    struct {
        uint32_t ISR:9;
        uint32_t _reserved0:23;
    } b;
    uint32_t w;
} IPSR_Type;

typedef struct {
    __IO uint32_t ISER[8];
    uint32_t RESERVED0[24];
    __IO uint32_t ICER[8];
    uint32_t RSERVED1[24];
    __IO uint32_t ISPR[8];
    uint32_t RESERVED2[24];
    __IO uint32_t ICPR[8];
    uint32_t RESERVED3[24];
    __IO uint32_t IABR[8];
    uint32_t RESERVED4[56];
    __IO uint8_t  IP[240];
    uint32_t RESERVED5[644];
    __IO uint32_t STIR;
} NVIC_Type;

typedef struct {
    __IO uint32_t CPUID;
    __IO uint32_t ICSR;
    __IO uint32_t VTOR;
    __IO uint32_t AIRCR;
    __IO uint32_t SCR;
    __IO uint32_t CCR;
    __IO uint8_t  SHP[12];
    __IO uint32_t SHCSR;
    __IO uint32_t CFSR;
    __IO uint32_t HFSR;
    __IO uint32_t DFSR;
    __IO uint32_t MMFAR;
    __IO uint32_t BFAR;
    __IO uint32_t AFSR;
} SCB_Type;

#define SCS_BASE            (0xE000E000UL)
#define NVIC_BASE           (SCS_BASE + 0x0100UL)
#define SCB_BASE            (SCS_BASE + 0x0D00UL)
#define SysTick_BASE        (SCS_BASE + 0x0010UL)

#define NVIC                ((NVIC_Type *)NVIC_BASE)
#define SCB                 ((SCB_Type *)SCB_BASE)

typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t LOAD;
    __IO uint32_t VAL;
    __I  uint32_t CALIB;
} SysTick_Type;

#define SysTick             ((SysTick_Type *)SysTick_BASE)

__STATIC_INLINE void __enable_irq(void)  { __asm volatile ("cpsie i"); }
__STATIC_INLINE void __disable_irq(void) { __asm volatile ("cpsid i"); }
__STATIC_INLINE uint32_t __get_CONTROL(void) { uint32_t r; __asm volatile ("mrs %0, control" : "=r"(r)); return r; }
__STATIC_INLINE void __set_CONTROL(uint32_t control) { __asm volatile ("msr control, %0" :: "r"(control)); }
__STATIC_INLINE uint32_t __get_IPSR(void) { uint32_t r; __asm volatile ("mrs %0, ipsr" : "=r"(r)); return r; }
__STATIC_INLINE uint32_t __get_PRIMASK(void) { uint32_t r; __asm volatile ("mrs %0, primask" : "=r"(r)); return r; }
__STATIC_INLINE void __set_PRIMASK(uint32_t priMask) { __asm volatile ("msr primask, %0" :: "r"(priMask)); }

#ifdef __cplusplus
}
#endif

#endif
