/**
  * @file    servo.h
  * @brief   3-DOF robotic arm servo control with PID
  */
#ifndef __SERVO_H
#define __SERVO_H

#include "main.h"
#include "pca9685.h"

#define ARM_BASE    SERVO_BASE_CH
#define ARM_BIG     SERVO_ARM1_CH
#define ARM_SMALL   SERVO_ARM2_CH

#define BASE_MIN    0
#define BASE_MAX    180
#define BIG_MIN     30
#define BIG_MAX     150
#define SMALL_MIN   20
#define SMALL_MAX   160

typedef enum {
    MODE_GESTURE = 0,
    MODE_MANUAL   = 1
} Arm_Mode_t;

typedef struct {
    float base_angle;
    float big_arm_angle;
    float small_arm_angle;
} Arm_Position_t;

void Arm_Init(void);
void Arm_SetPosition(Arm_Position_t *pos);
void Arm_SetGestureMode(float roll, float pitch, float yaw);
void Arm_SetManualKnob(uint16_t adc_value);
void Arm_SetActionHold(uint8_t held);
void Arm_UpdateManualAction(void);
void Arm_DetectKnob(uint16_t adc_value);
void Arm_Stop(void);
Arm_Mode_t Arm_GetMode(void);
void Arm_SetMode(Arm_Mode_t mode);
void Arm_Update(void);
void Arm_PrintDebugInfo(void);
Arm_Position_t* Arm_GetTargetPos(void);
Arm_Position_t* Arm_GetCurrentPos(void);

#endif /* __SERVO_H */
