/**
  * @file    pca9685.h
  * @brief   PCA9685 16-channel PWM servo driver
  *          Drives 3x MG996R servos for 3-DOF robotic arm
  */
#ifndef __PCA9685_H
#define __PCA9685_H

#include "main.h"
#include "i2c.h"

/* PCA9685 I2C Address (A5=A4=A3=A2=A1=A0=0) */
#define PCA9685_ADDR            0x80

/* PCA9685 Registers */
#define PCA9685_REG_MODE1       0x00
#define PCA9685_REG_MODE2       0x01
#define PCA9685_REG_PRESCALE    0xFE
#define PCA9685_REG_LED0_ON_L   0x06
#define PCA9685_REG_LED0_ON_H   0x07
#define PCA9685_REG_LED0_OFF_L  0x08
#define PCA9685_REG_LED0_OFF_H  0x09

/* MODE1 register bit definitions */
#define PCA9685_MODE1_RESTART   0x80
#define PCA9685_MODE1_EXTCLK    0x40
#define PCA9685_MODE1_AI        0x20
#define PCA9685_MODE1_SLEEP     0x10

#define PCA9685_ALL_LED_ON_L    0xFA
#define PCA9685_ALL_LED_ON_H    0xFB
#define PCA9685_ALL_LED_OFF_L   0xFC
#define PCA9685_ALL_LED_OFF_H   0xFD

/* PWM channel mapping for robotic arm */
#define SERVO_BASE_CH           0   /* CH0: Base rotation servo */
#define SERVO_ARM1_CH           1   /* CH1: Big arm (shoulder) servo */
#define SERVO_ARM2_CH           2   /* CH2: Small arm (elbow) servo */

/* MG996R PWM timing: 0.5ms~2.5ms pulse, 50Hz (20ms period) */
/* PCA9685 at 50Hz: 4096 counts per 20ms, 1 count = 4.88us */
/* 0.5ms = 102 counts (0 deg), 2.5ms = 512 counts (180 deg) */
#define SERVO_MIN_COUNT         102
#define SERVO_MAX_COUNT         512
#define SERVO_MID_COUNT         307

#define SERVO_ANGLE_MIN         0
#define SERVO_ANGLE_MAX         180
#define SERVO_ANGLE_MID         90

/* Function prototypes */
uint8_t PCA9685_Init(void);
void PCA9685_SetPWMFreq(uint8_t freq);
void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off);
void PCA9685_SetServoAngle(uint8_t channel, uint8_t angle);
void PCA9685_SetServoAngleF(uint8_t channel, float angle);
void PCA9685_Sleep(void);
void PCA9685_Wake(void);
uint8_t PCA9685_SelfTest(void);
uint8_t PCA9685_IsOnline(void);

#endif /* __PCA9685_H */
