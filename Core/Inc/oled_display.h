/**
  * @file    oled_display.h
  * @brief   High-level OLED display system for gesture robot arm
  *          Layout: 128x64, 6x8 font, 8 rows x 21 columns
  *          Anti-flicker with dirty-page tracking
  */
#ifndef __OLED_DISPLAY_H
#define __OLED_DISPLAY_H

#include "main.h"
#include "oled.h"
#include "mpu6050.h"
#include "servo.h"
#include "i2c.h"
#include "adc.h"

#define OLED_DISP_INTERVAL_MS   200

typedef enum {
    DISP_PAGE_SENSOR = 0,
    DISP_PAGE_SERVO  = 1,
    DISP_PAGE_SYSTEM = 2,
    DISP_PAGE_DEBUG  = 3,
    DISP_PAGE_COUNT  = 4
} OLED_DispPage_t;

void OLED_DispInit(void);
void OLED_DispUpdate(const MPU6050_Angle_t *angle, uint8_t mpu_valid,
                     uint16_t mpu_errors, uint16_t mpu_reads,
                     const Arm_Position_t *target, const Arm_Position_t *current,
                     Arm_Mode_t mode, uint16_t i2c_err, uint16_t i2c_recv,
                     uint8_t knob_connected, float temp,
                     uint8_t pca_valid);
void OLED_DispShowInitProgress(const char *msg, uint8_t step, uint8_t total);
void OLED_DispShowError(const char *msg);
void OLED_DispNextPage(void);
OLED_DispPage_t OLED_DispGetPage(void);

#endif /* __OLED_DISPLAY_H */
