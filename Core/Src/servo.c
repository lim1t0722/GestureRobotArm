/**
  * @file    servo.c
  * @brief   3-DOF robotic arm servo control with S-curve trajectory planning
  *
  * S-curve interpolation using second-order spring-damper system:
  *   acceleration = spring_force + damping_force
  *   spring_force = (target - current) * K_SPRING
  *   damping_force = -velocity * K_DAMPING
  *
  * This produces S-curve position trajectories with:
  *   - Smooth acceleration from rest
  *   - Smooth deceleration to target
  *   - Zero velocity at target (no overshoot)
  *   - No abrupt velocity changes (eliminates servo jitter)
  */
#include "servo.h"
#include "adc.h"
#include "uart_debug.h"
#include <math.h>

static Arm_Mode_t current_mode = MODE_MANUAL;

static Arm_Position_t target_pos  = {90.0f, 90.0f, 90.0f};
static Arm_Position_t current_pos = {90.0f, 90.0f, 90.0f};
static uint32_t last_update = 0;

static uint8_t action_held = 0;

#define LIFT_REST_BIG       90.0f
#define LIFT_REST_SMALL     90.0f
#define LIFT_DOWN_BIG       40.0f
#define LIFT_DOWN_SMALL     140.0f

#define K_SPRING_MANUAL     0.12f
#define K_DAMPING_MANUAL    0.55f
#define MAX_VEL_MANUAL      4.0f
#define MAX_ACCEL_MANUAL    1.5f

#define K_SPRING_GESTURE    0.22f
#define K_DAMPING_GESTURE   0.60f
#define MAX_VEL_GESTURE     5.0f
#define MAX_ACCEL_GESTURE   2.0f

#define ARRIVE_THRESHOLD    0.3f
#define VEL_THRESHOLD       0.05f

typedef struct {
    float velocity;
} ServoDyn_t;

static ServoDyn_t dyn[3];

static float GetCurrentPos(uint8_t ch)
{
    switch (ch) {
        case ARM_BASE:  return current_pos.base_angle;
        case ARM_BIG:   return current_pos.big_arm_angle;
        case ARM_SMALL: return current_pos.small_arm_angle;
        default: return 90.0f;
    }
}

static void SetCurrentPos(uint8_t ch, float pos)
{
    switch (ch) {
        case ARM_BASE:  current_pos.base_angle = pos; break;
        case ARM_BIG:   current_pos.big_arm_angle = pos; break;
        case ARM_SMALL: current_pos.small_arm_angle = pos; break;
    }
}

static float ClampAngle(uint8_t ch, float angle)
{
    float min_a, max_a;
    switch (ch) {
        case ARM_BASE:  min_a = (float)BASE_MIN; max_a = (float)BASE_MAX; break;
        case ARM_BIG:   min_a = (float)BIG_MIN;  max_a = (float)BIG_MAX; break;
        case ARM_SMALL: min_a = (float)SMALL_MIN; max_a = (float)SMALL_MAX; break;
        default: return angle;
    }
    if (angle < min_a) return min_a;
    if (angle > max_a) return max_a;
    return angle;
}

static float GetMinAngle(uint8_t ch)
{
    switch (ch) {
        case ARM_BASE:  return (float)BASE_MIN;
        case ARM_BIG:   return (float)BIG_MIN;
        case ARM_SMALL: return (float)SMALL_MIN;
        default: return 0.0f;
    }
}

static float GetMaxAngle(uint8_t ch)
{
    switch (ch) {
        case ARM_BASE:  return (float)BASE_MAX;
        case ARM_BIG:   return (float)BIG_MAX;
        case ARM_SMALL: return (float)SMALL_MAX;
        default: return 180.0f;
    }
}

static void Arm_MoveServo(uint8_t channel, float target_angle)
{
    float current = GetCurrentPos(channel);
    target_angle = ClampAngle(channel, target_angle);

    float k_spring = (current_mode == MODE_GESTURE) ? K_SPRING_GESTURE : K_SPRING_MANUAL;
    float k_damping = (current_mode == MODE_GESTURE) ? K_DAMPING_GESTURE : K_DAMPING_MANUAL;
    float max_vel = (current_mode == MODE_GESTURE) ? MAX_VEL_GESTURE : MAX_VEL_MANUAL;
    float max_accel = (current_mode == MODE_GESTURE) ? MAX_ACCEL_GESTURE : MAX_ACCEL_MANUAL;

    float spring_force = (target_angle - current) * k_spring;
    float damping_force = -dyn[channel].velocity * k_damping;
    float accel = spring_force + damping_force;

    if (accel > max_accel) accel = max_accel;
    if (accel < -max_accel) accel = -max_accel;

    dyn[channel].velocity += accel;

    if (dyn[channel].velocity > max_vel) dyn[channel].velocity = max_vel;
    if (dyn[channel].velocity < -max_vel) dyn[channel].velocity = -max_vel;

    current += dyn[channel].velocity;

    float min_a = GetMinAngle(channel);
    float max_a = GetMaxAngle(channel);
    if (current < min_a) { current = min_a; dyn[channel].velocity = 0; }
    if (current > max_a) { current = max_a; dyn[channel].velocity = 0; }

    if (fabsf(target_angle - current) < ARRIVE_THRESHOLD &&
        fabsf(dyn[channel].velocity) < VEL_THRESHOLD) {
        current = target_angle;
        dyn[channel].velocity = 0;
    }

    SetCurrentPos(channel, current);
    PCA9685_SetServoAngleF(channel, current);
}

void Arm_Init(void)
{
    current_pos.base_angle     = 90.0f;
    current_pos.big_arm_angle  = 90.0f;
    current_pos.small_arm_angle = 90.0f;
    target_pos = current_pos;

    for (uint8_t i = 0; i < 3; i++) {
        dyn[i].velocity = 0;
    }

    PCA9685_SetServoAngleF(ARM_BASE,  90.0f);
    PCA9685_SetServoAngleF(ARM_BIG,   90.0f);
    PCA9685_SetServoAngleF(ARM_SMALL, 90.0f);

    HAL_Delay(500);

    DBG_PRINT("[ARM] Init OK (S-curve spring-damper)\r\n");
}

void Arm_SetGestureMode(float roll, float pitch, float yaw)
{
    target_pos.base_angle = 90.0f + roll * 1.0f;
    if (target_pos.base_angle < BASE_MIN) target_pos.base_angle = (float)BASE_MIN;
    if (target_pos.base_angle > BASE_MAX) target_pos.base_angle = (float)BASE_MAX;

    target_pos.big_arm_angle = 90.0f + pitch * 1.0f;
    if (target_pos.big_arm_angle < BIG_MIN) target_pos.big_arm_angle = (float)BIG_MIN;
    if (target_pos.big_arm_angle > BIG_MAX) target_pos.big_arm_angle = (float)BIG_MAX;

    target_pos.small_arm_angle = 90.0f - pitch * 0.8f;
    if (target_pos.small_arm_angle < SMALL_MIN) target_pos.small_arm_angle = (float)SMALL_MIN;
    if (target_pos.small_arm_angle > SMALL_MAX) target_pos.small_arm_angle = (float)SMALL_MAX;

    (void)yaw;
}

void Arm_SetManualKnob(uint16_t adc_value)
{
    target_pos.base_angle = (float)adc_value * 180.0f / 4095.0f;
}

void Arm_SetActionHold(uint8_t held)
{
    action_held = held;
}

void Arm_UpdateManualAction(void)
{
    if (action_held) {
        target_pos.big_arm_angle = LIFT_DOWN_BIG;
        target_pos.small_arm_angle = LIFT_DOWN_SMALL;
    } else {
        target_pos.big_arm_angle = LIFT_REST_BIG;
        target_pos.small_arm_angle = LIFT_REST_SMALL;
    }
}

void Arm_DetectKnob(uint16_t adc_value)
{
    (void)adc_value;
}

void Arm_SetPosition(Arm_Position_t *pos)
{
    if (!pos) return;
    target_pos = *pos;
}

void Arm_Stop(void)
{
    PCA9685_SetServoAngle(ARM_BASE,  90);
    PCA9685_SetServoAngle(ARM_BIG,   90);
    PCA9685_SetServoAngle(ARM_SMALL, 90);

    current_pos.base_angle     = 90.0f;
    current_pos.big_arm_angle  = 90.0f;
    current_pos.small_arm_angle = 90.0f;
    target_pos = current_pos;

    for (uint8_t i = 0; i < 3; i++) {
        dyn[i].velocity = 0;
    }
}

Arm_Mode_t Arm_GetMode(void)
{
    return current_mode;
}

void Arm_SetMode(Arm_Mode_t mode)
{
    current_mode = mode;
    if (mode == MODE_MANUAL) {
        target_pos.big_arm_angle = current_pos.big_arm_angle;
        target_pos.small_arm_angle = current_pos.small_arm_angle;
    }
    for (uint8_t i = 0; i < 3; i++) {
        dyn[i].velocity = 0;
    }
    DBG_PRINT("[ARM] Mode: %s\r\n", mode == MODE_GESTURE ? "GESTURE" : "MANUAL");
}

void Arm_Update(void)
{
    uint32_t now = HAL_GetTick();
    if (now - last_update < 20) return;
    last_update = now;

    if (current_mode == MODE_MANUAL) {
        Arm_UpdateManualAction();
    }

    Arm_MoveServo(ARM_BASE,  target_pos.base_angle);
    Arm_MoveServo(ARM_BIG,   target_pos.big_arm_angle);
    Arm_MoveServo(ARM_SMALL, target_pos.small_arm_angle);
}

void Arm_PrintDebugInfo(void)
{
    DBG_PRINT("[ARM] Tgt: B=%.1f Big=%.1f S=%.1f Cur: B=%.1f Big=%.1f S=%.1f V: B=%.2f Big=%.2f S=%.2f %s\r\n",
              target_pos.base_angle, target_pos.big_arm_angle, target_pos.small_arm_angle,
              current_pos.base_angle, current_pos.big_arm_angle, current_pos.small_arm_angle,
              dyn[ARM_BASE].velocity, dyn[ARM_BIG].velocity, dyn[ARM_SMALL].velocity,
              current_mode == MODE_GESTURE ? "GES" : "MAN");
}

Arm_Position_t* Arm_GetTargetPos(void)
{
    return &target_pos;
}

Arm_Position_t* Arm_GetCurrentPos(void)
{
    return &current_pos;
}
