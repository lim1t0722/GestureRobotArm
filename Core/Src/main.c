/**
  * @file    main.c
  * @brief   Gesture-following robotic arm with 3-DOF
  */
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "adc.h"
#include "mpu6050.h"
#include "pca9685.h"
#include "servo.h"
#include "uart_debug.h"
#include "oled_display.h"
#include <stdio.h>

#define MAIN_LOOP_DELAY_MS      20
#define BUTTON_DEBOUNCE_MS      50
#define MODE_SWITCH_LONG_PRESS  3000
#define INIT_RETRY_COUNT        3
#define INIT_RETRY_DELAY_MS     500

#define DIAG_PRINT_INTERVAL_MS  2000
#define OLED_DISP_INTERVAL_MS   200
#define MPU_REINIT_THRESHOLD    10
#define MPU_REINIT_COOLDOWN_MS  3000

#define DOUBLE_CLICK_INTERVAL   400

/* #define SERVO_TEST */

#ifdef SERVO_TEST
static void Servo_TestRoutine(void);
#endif

void SystemClock_Config(void);
static void Check_PA2_Button(void);
static void Check_PA0_Button(void);
static void LED_Blink(uint16_t times, uint16_t duration_ms);

static uint8_t s_pa2_pressed = 0;
static uint32_t s_pa2_press_time = 0;

static uint8_t s_pa0_prev = 1;
static uint32_t s_pa0_last_release = 0;
static uint8_t s_pa0_click_count = 0;
static uint32_t s_pa0_click_time = 0;

int main(void)
{
    MPU6050_Angle_t mpu_angle = {0};
    uint16_t knob_value = 2048;
    uint32_t last_mpu_time = 0;
    uint32_t last_led_toggle = 0;
    uint32_t last_diag_time = 0;
    uint16_t mpu_fail_count = 0;
    uint32_t mpu_reinit_last = 0;
    float mpu_temp = 0.0f;
    uint32_t last_temp_time = 0;

    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_ADC1_Init();

    UART_Debug_Init();
    DBG_PRINT("\r\n========================================\r\n");
    DBG_PRINT("Gesture Robot Arm - System Start\r\n");
    DBG_PRINT("SYSCLK=%luHz\r\n", SystemCoreClock);
    DBG_PRINT("========================================\r\n");

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

    OLED_DispInit();
    OLED_DispShowInitProgress("System init...", 1, 5);

    {
        uint8_t retry = 0;
        while (PCA9685_Init() != 0) {
            retry++;
            DBG_PRINT("[INIT] PCA9685 retry %u/%u\r\n", retry, INIT_RETRY_COUNT);
            OLED_DispShowInitProgress("PCA9685 init...", 2, 5);
            if (retry >= INIT_RETRY_COUNT) {
                DBG_PRINT("[INIT] PCA9685 FAILED\r\n");
                OLED_DispShowError("PCA9685 FAIL");
                HAL_Delay(5000);
                NVIC_SystemReset();
            }
            HAL_Delay(INIT_RETRY_DELAY_MS);
        }
    }
    DBG_PRINT("[INIT] PCA9685 OK\r\n");
    OLED_DispShowInitProgress("PCA9685 OK", 2, 5);

    PCA9685_SelfTest();

#ifdef SERVO_TEST
    Servo_TestRoutine();
#else

    {
        uint8_t retry = 0;
        while (MPU6050_Init() != 0) {
            retry++;
            DBG_PRINT("[INIT] MPU6050 retry %u/%u\r\n", retry, INIT_RETRY_COUNT);
            OLED_DispShowInitProgress("MPU6050 init...", 3, 5);
            if (retry >= INIT_RETRY_COUNT) {
                DBG_PRINT("[INIT] MPU6050 FAILED, continuing without MPU\r\n");
                break;
            }
            HAL_Delay(INIT_RETRY_DELAY_MS);
        }
    }
    if (MPU6050_IsDataValid()) {
        DBG_PRINT("[INIT] MPU6050 OK\r\n");
        OLED_DispShowInitProgress("MPU6050 OK", 3, 5);
    } else {
        DBG_PRINT("[INIT] MPU6050 FAIL, step=%u\r\n", MPU6050_GetFailStep());
        OLED_DispShowInitProgress("MPU6050 FAIL", 3, 5);
        HAL_Delay(1000);
    }

    Arm_Init();
    OLED_DispShowInitProgress("Arm init OK", 4, 5);

    if (!MPU6050_IsDataValid()) {
        Arm_SetMode(MODE_MANUAL);
        DBG_PRINT("[INIT] MPU6050 not available, default MANUAL mode\r\n");
    }

    OLED_DispShowInitProgress("Ready!", 5, 5);
    HAL_Delay(500);

    LED_Blink(5, 200);

    DBG_PRINT("[INIT] All OK. Mode: %s\r\n", Arm_GetMode() == MODE_GESTURE ? "GESTURE" : "MANUAL");

    while (1) {
        Check_PA2_Button();
        Check_PA0_Button();

        if (Arm_GetMode() == MODE_GESTURE) {
            if (HAL_GetTick() - last_mpu_time >= 20) {
                last_mpu_time = HAL_GetTick();

                uint8_t mpu_result = MPU6050_GetAngle(&mpu_angle);
                if (mpu_result == 0) {
                    mpu_fail_count = 0;
                    Arm_SetGestureMode(mpu_angle.roll,
                                      mpu_angle.pitch,
                                       mpu_angle.yaw);
                } else {
                    mpu_fail_count++;
                    if (mpu_fail_count >= MPU_REINIT_THRESHOLD &&
                        (HAL_GetTick() - mpu_reinit_last) >= MPU_REINIT_COOLDOWN_MS) {
                        mpu_reinit_last = HAL_GetTick();
                        DBG_PRINT("[MAIN] MPU6050 %u failures, attempting ReInit...\r\n", mpu_fail_count);
                        if (MPU6050_ReInit() == 0) {
                            mpu_fail_count = 0;
                            DBG_PRINT("[MAIN] MPU6050 ReInit SUCCESS\r\n");
                        } else {
                            DBG_PRINT("[MAIN] MPU6050 ReInit FAILED, will retry later\r\n");
                        }
                    }
                }
            }

            if (HAL_GetTick() - last_led_toggle >= 500) {
                last_led_toggle = HAL_GetTick();
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }

        } else {
            knob_value = ADC_ReadKnob();
            Arm_SetManualKnob(knob_value);

            uint8_t pa0_held = (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) ? 1 : 0;
            Arm_SetActionHold(pa0_held);

            if (HAL_GetTick() - last_led_toggle >= 150) {
                last_led_toggle = HAL_GetTick();
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
        }

        Arm_Update();

        if (HAL_GetTick() - last_temp_time >= 1000) {
            last_temp_time = HAL_GetTick();
            mpu_temp = MPU6050_GetTemp();
        }

        OLED_DispUpdate(&mpu_angle, MPU6050_IsDataValid(),
                         MPU6050_GetErrorCount(), MPU6050_GetReadCount(),
                         Arm_GetTargetPos(), Arm_GetCurrentPos(),
                         Arm_GetMode(),
                         I2C_GetErrorCount(), I2C_GetRecoveryCount(),
                         ADC_IsKnobConnected(), mpu_temp,
                         PCA9685_IsOnline());

        if (HAL_GetTick() - last_diag_time >= DIAG_PRINT_INTERVAL_MS) {
            last_diag_time = HAL_GetTick();
            MPU6050_PrintDebugInfo();
            Arm_PrintDebugInfo();
            DBG_PRINT("[ADC] raw=%u knob=%u\r\n", ADC_GetRawLatest(), knob_value);
        }

        HAL_Delay(MAIN_LOOP_DELAY_MS);
    }
#endif
}

static void Check_PA0_Button(void)
{
    uint8_t pa0_cur = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);

    if (s_pa0_prev == 0 && pa0_cur == 1) {
        uint32_t now = HAL_GetTick();
        if (now - s_pa0_last_release < DOUBLE_CLICK_INTERVAL) {
            s_pa0_click_count++;
            if (s_pa0_click_count >= 2) {
                Arm_Mode_t new_mode = (Arm_GetMode() == MODE_GESTURE) ? MODE_MANUAL : MODE_GESTURE;
                Arm_SetMode(new_mode);
                if (new_mode == MODE_GESTURE) {
                    MPU6050_ResetAll();
                }
                LED_Blink(new_mode == MODE_GESTURE ? 2 : 4, 100);
                s_pa0_click_count = 0;
            }
        } else {
            s_pa0_click_count = 1;
        }
        s_pa0_last_release = now;
        s_pa0_click_time = now;
    }

    if (s_pa0_click_count == 1 && (HAL_GetTick() - s_pa0_click_time > DOUBLE_CLICK_INTERVAL + 50)) {
        s_pa0_click_count = 0;
    }

    s_pa0_prev = pa0_cur;
}

static void Check_PA2_Button(void)
{
    static uint8_t stable_state = 1;
    static uint8_t debouncing = 0;
    static uint32_t debounce_start = 0;
    static uint8_t raw_prev = 1;

    uint8_t raw = HAL_GPIO_ReadPin(DBG_KEY_GPIO_Port, DBG_KEY_Pin);

    if (raw != raw_prev) {
        debouncing = 1;
        debounce_start = HAL_GetTick();
        raw_prev = raw;
    }

    if (debouncing && (HAL_GetTick() - debounce_start >= BUTTON_DEBOUNCE_MS)) {
        debouncing = 0;
        uint8_t new_state = raw;

        if (stable_state == 1 && new_state == 0) {
            s_pa2_press_time = HAL_GetTick();
            s_pa2_pressed = 1;
        }
        else if (stable_state == 0 && new_state == 1) {
            if (s_pa2_pressed) {
                uint32_t press_duration = HAL_GetTick() - s_pa2_press_time;

                if (press_duration >= MODE_SWITCH_LONG_PRESS) {
                    Arm_Mode_t new_mode = (Arm_GetMode() == MODE_GESTURE) ? MODE_MANUAL : MODE_GESTURE;
                    Arm_SetMode(new_mode);

                    if (new_mode == MODE_GESTURE) {
                        MPU6050_ResetAll();
                    }

                    LED_Blink(new_mode == MODE_GESTURE ? 2 : 4, 100);
                } else {
                    OLED_DispNextPage();
                }
                s_pa2_pressed = 0;
            }
        }
        stable_state = new_state;
    }
}

static void LED_Blink(uint16_t times, uint16_t duration_ms)
{
    for (uint16_t i = 0; i < times; i++) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        HAL_Delay(duration_ms);
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        HAL_Delay(duration_ms);
    }
}

void SystemClock_Config(void)
{
    SystemCoreClockUpdate();
    HAL_InitTick(TICK_INT_PRIORITY);
}

#ifdef SERVO_TEST
static void Servo_TestRoutine(void)
{
    DBG_PRINT("\r\n===== SERVO TEST MODE =====\r\n");

    const uint8_t channels[3] = {SERVO_BASE_CH, SERVO_ARM1_CH, SERVO_ARM2_CH};

    while (1) {
        for (uint8_t angle = 0; angle <= 180; angle += 10) {
            for (int i = 0; i < 3; i++) {
                PCA9685_SetServoAngle(channels[i], angle);
            }
            HAL_Delay(200);
        }

        for (uint8_t angle = 180; angle > 0; angle -= 10) {
            for (int i = 0; i < 3; i++) {
                PCA9685_SetServoAngle(channels[i], angle);
            }
            HAL_Delay(200);
        }

        LED_Blink(2, 200);
    }
}
#endif

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        for (volatile uint32_t i = 0; i < 7200000; i++);
    }
}
