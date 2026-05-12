/**
  * @file    oled_display.c
  * @brief   High-level OLED display system implementation
  *          3-page layout: Sensor / Servo / System
  *          Dirty-page tracking for anti-flicker partial refresh
  */
#include "oled_display.h"
#include <stdio.h>
#include <string.h>

static OLED_DispPage_t current_page = DISP_PAGE_SENSOR;
static uint8_t dirty_pages[OLED_PAGES];
static uint32_t last_disp_time = 0;

static void MarkPageDirty(uint8_t page_start, uint8_t page_end)
{
    for (uint8_t p = page_start; p <= page_end && p < OLED_PAGES; p++) {
        dirty_pages[p] = 1;
    }
}

static void ClearDirty(void)
{
    memset(dirty_pages, 0, sizeof(dirty_pages));
}

static void RefreshDirtyPages(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        if (dirty_pages[page]) {
            OLED_WriteCmd(0xB0 + page);
            OLED_WriteCmd(0x00);
            OLED_WriteCmd(0x10);
            OLED_WriteDataBurst(&oled_fb[page * OLED_WIDTH], OLED_WIDTH);
        }
    }
    ClearDirty();
}

void OLED_DispInit(void)
{
    OLED_Init();
    ClearDirty();
    last_disp_time = HAL_GetTick();
}

static void DrawPageHeader(const char *title, uint8_t page_num)
{
    OLED_FillArea(0, 0, 128, 8, 0xFF);
    OLED_DrawString(2, 0, title, OLED_FONT_6x8);
    char pn[4];
    pn[0] = 'P';
    pn[1] = '0' + page_num;
    pn[2] = '/';
    pn[3] = '0' + DISP_PAGE_COUNT;
    OLED_DrawString(110, 0, pn, OLED_FONT_6x8);
    MarkPageDirty(0, 0);
}

static void DrawSensorPage(const MPU6050_Angle_t *angle, uint8_t mpu_valid,
                           uint16_t mpu_errors, uint16_t mpu_reads, float temp)
{
    DrawPageHeader("MPU6050 Sensor", 1);

    OLED_FillArea(0, 8, 128, 56, 0x00);
    MarkPageDirty(1, 7);

    if (mpu_valid) {
        OLED_DrawString(0, 8,  "Roll:", OLED_FONT_6x8);
        OLED_DrawFloat(36, 8, angle->roll, 1, OLED_FONT_6x8);
        OLED_DrawString(78, 8,  "deg", OLED_FONT_6x8);

        OLED_DrawString(0, 16, "Ptch:", OLED_FONT_6x8);
        OLED_DrawFloat(36, 16, angle->pitch, 1, OLED_FONT_6x8);
        OLED_DrawString(78, 16, "deg", OLED_FONT_6x8);

        OLED_DrawString(0, 24, "Yaw :", OLED_FONT_6x8);
        OLED_DrawFloat(36, 24, angle->yaw, 1, OLED_FONT_6x8);
        OLED_DrawString(78, 24, "deg", OLED_FONT_6x8);

        OLED_DrawHLine(0, 32, 128);
        MarkPageDirty(4, 4);

        OLED_DrawString(0, 36, "Temp:", OLED_FONT_6x8);
        OLED_DrawFloat(36, 36, temp, 1, OLED_FONT_6x8);
        OLED_DrawString(72, 36, "C", OLED_FONT_6x8);

        OLED_DrawString(0, 44, "Stat:", OLED_FONT_6x8);
        OLED_DrawString(36, 44, "OK", OLED_FONT_6x8);

        OLED_DrawString(54, 44, "Err:", OLED_FONT_6x8);
        OLED_DrawInt(78, 44, mpu_errors, OLED_FONT_6x8);
        OLED_DrawString(96, 44, "/", OLED_FONT_6x8);
        OLED_DrawInt(102, 44, mpu_reads, OLED_FONT_6x8);

        OLED_DrawString(0, 56, "Filt:Compl a=0.94", OLED_FONT_6x8);
    } else {
        OLED_DrawString(0, 8,  "!! MPU6050 FAIL !!", OLED_FONT_6x8);

        OLED_DrawHLine(0, 16, 128);
        MarkPageDirty(2, 2);

        uint8_t step = MPU6050_GetFailStep();
        OLED_DrawString(0, 18, "Step:", OLED_FONT_6x8);
        OLED_DrawInt(36, 18, step, OLED_FONT_6x8);

        const char *step_msg = "Unknown";
        switch (step) {
            case 0:  step_msg = "No error"; break;
            case 1:  step_msg = "Bus BUSY"; break;
            case 2:  step_msg = "WHO_AM_I RD"; break;
            case 3:  step_msg = "WHO_AM_I VAL"; break;
            case 4:  step_msg = "PWR1 reset"; break;
            case 5:  step_msg = "PWR1 wake"; break;
            case 6:  step_msg = "PWR2 cfg"; break;
            case 7:  step_msg = "CONFIG cfg"; break;
            case 8:  step_msg = "SMPLRT cfg"; break;
            case 9:  step_msg = "GYRO cfg"; break;
            case 10: step_msg = "ACCEL cfg"; break;
            case 11: step_msg = "INT cfg"; break;
            case 12: step_msg = "USER cfg"; break;
            case 13: step_msg = "FIFO cfg"; break;
            case 14: step_msg = "Read test"; break;
        }
        OLED_DrawString(54, 18, step_msg, OLED_FONT_6x8);

        uint16_t i2c_status = MPU6050_GetI2CStatus();
        OLED_DrawString(0, 26, "SR1:", OLED_FONT_6x8);
        OLED_DrawHex(24, 26, i2c_status >> 8, OLED_FONT_6x8);
        OLED_DrawString(54, 26, "SR2:", OLED_FONT_6x8);
        OLED_DrawHex(78, 26, i2c_status & 0xFF, OLED_FONT_6x8);

        OLED_DrawString(0, 34, "Err:", OLED_FONT_6x8);
        OLED_DrawInt(24, 34, mpu_errors, OLED_FONT_6x8);
        OLED_DrawString(54, 34, "Rcv:", OLED_FONT_6x8);
        OLED_DrawInt(78, 34, I2C_GetRecoveryCount(), OLED_FONT_6x8);

        OLED_DrawString(0, 42, "PB6:", OLED_FONT_6x8);
        OLED_DrawString(24, 42, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) ? "H" : "L", OLED_FONT_6x8);
        OLED_DrawString(42, 42, "PB7:", OLED_FONT_6x8);
        OLED_DrawString(66, 42, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) ? "H" : "L", OLED_FONT_6x8);
        OLED_DrawString(84, 42, "Idle:", OLED_FONT_6x8);
        OLED_DrawString(108, 42, I2C_IsBusIdle() ? "Y" : "N", OLED_FONT_6x8);

        OLED_DrawString(0, 52, "Addr:0xD0  P4=Debug", OLED_FONT_6x8);
    }
}

static void DrawServoPage(const Arm_Position_t *target, const Arm_Position_t *current,
                          Arm_Mode_t mode)
{
    DrawPageHeader("Servo Control", 2);

    OLED_FillArea(0, 8, 128, 56, 0x00);
    MarkPageDirty(1, 7);

    OLED_DrawString(0, 8,  "Mode:", OLED_FONT_6x8);
    OLED_DrawString(36, 8, mode == MODE_GESTURE ? "GESTURE" : "MANUAL", OLED_FONT_6x8);

    OLED_DrawHLine(0, 16, 128);
    MarkPageDirty(2, 2);

    OLED_DrawString(0, 18, "    Tgt   Cur", OLED_FONT_6x8);

    OLED_DrawString(0, 26, "B:", OLED_FONT_6x8);
    OLED_DrawFloat(12, 26, target->base_angle, 0, OLED_FONT_6x8);
    OLED_DrawFloat(54, 26, current->base_angle, 0, OLED_FONT_6x8);

    OLED_DrawString(0, 34, "G:", OLED_FONT_6x8);
    OLED_DrawFloat(12, 34, target->big_arm_angle, 0, OLED_FONT_6x8);
    OLED_DrawFloat(54, 34, current->big_arm_angle, 0, OLED_FONT_6x8);

    OLED_DrawString(0, 42, "S:", OLED_FONT_6x8);
    OLED_DrawFloat(12, 42, target->small_arm_angle, 0, OLED_FONT_6x8);
    OLED_DrawFloat(54, 42, current->small_arm_angle, 0, OLED_FONT_6x8);

    OLED_DrawHLine(0, 50, 128);
    MarkPageDirty(6, 6);

    {
        uint8_t bx = 2;
        uint8_t bw = 124;
        uint8_t by = 54;
        uint8_t bh = 6;
        OLED_DrawRect(bx, by, bw, bh);
        MarkPageDirty(by / 8, (by + bh - 1) / 8);
        uint8_t fill = (uint8_t)(current->base_angle * (bw - 2) / 180.0f);
        for (uint8_t i = 0; i < fill && i < bw - 2; i++) {
            OLED_SetPixel(bx + 1 + i, by + 1, 1);
            OLED_SetPixel(bx + 1 + i, by + 2, 1);
            OLED_SetPixel(bx + 1 + i, by + 3, 1);
            OLED_SetPixel(bx + 1 + i, by + 4, 1);
        }
    }
}

static void DrawSystemPage(uint16_t i2c_err, uint16_t i2c_recv,
                           uint8_t knob_connected,
                           uint8_t mpu_valid, uint8_t pca_valid)
{
    DrawPageHeader("System Info", 3);

    OLED_FillArea(0, 8, 128, 56, 0x00);
    MarkPageDirty(1, 7);

    OLED_DrawString(0, 8,  "SYSCLK:", OLED_FONT_6x8);
    OLED_DrawInt(54, 8, SystemCoreClock / 1000000, OLED_FONT_6x8);
    OLED_DrawString(78, 8, "MHz", OLED_FONT_6x8);

    OLED_DrawString(0, 16, "MPU6050:", OLED_FONT_6x8);
    OLED_DrawString(54, 16, mpu_valid ? "OK" : "FAIL", OLED_FONT_6x8);

    OLED_DrawString(72, 16, "PCA:", OLED_FONT_6x8);
    OLED_DrawString(96, 16, pca_valid ? "OK" : "FAIL", OLED_FONT_6x8);

    OLED_DrawString(0, 24, "I2C Err:", OLED_FONT_6x8);
    OLED_DrawInt(54, 24, i2c_err, OLED_FONT_6x8);
    OLED_DrawString(72, 24, "Rcv:", OLED_FONT_6x8);
    OLED_DrawInt(96, 24, i2c_recv, OLED_FONT_6x8);

    OLED_DrawString(0, 32, "Knob:", OLED_FONT_6x8);
    OLED_DrawString(36, 32, knob_connected ? "Connected" : "N/C", OLED_FONT_6x8);

    OLED_DrawHLine(0, 40, 128);
    MarkPageDirty(5, 5);

    OLED_DrawString(0, 44, "I2C1: PB6/PB7", OLED_FONT_6x8);
    OLED_DrawString(0, 52, "SW_I2C: PB10/PB11", OLED_FONT_6x8);
    OLED_DrawString(0, 60, "  SSD1306 OLED", OLED_FONT_6x8);
}

static void DrawDebugPage(void)
{
    DrawPageHeader("I2C Debug", 4);

    OLED_FillArea(0, 8, 128, 56, 0x00);
    MarkPageDirty(1, 7);

    OLED_DrawString(0, 8,  "I2C1 SR1:", OLED_FONT_6x8);
    OLED_DrawHex(54, 8, I2C1->SR1 & 0xFF, OLED_FONT_6x8);
    OLED_DrawString(72, 8,  "SR2:", OLED_FONT_6x8);
    OLED_DrawHex(96, 8, I2C1->SR2 & 0xFF, OLED_FONT_6x8);

    OLED_DrawString(0, 16, "CR1:", OLED_FONT_6x8);
    OLED_DrawHex(24, 16, I2C1->CR1 & 0xFF, OLED_FONT_6x8);
    OLED_DrawString(54, 16, "CR2:", OLED_FONT_6x8);
    OLED_DrawHex(78, 16, I2C1->CR2 & 0xFF, OLED_FONT_6x8);

    OLED_DrawString(0, 24, "PB6:", OLED_FONT_6x8);
    OLED_DrawString(24, 24, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) ? "H" : "L", OLED_FONT_6x8);
    OLED_DrawString(42, 24, "PB7:", OLED_FONT_6x8);
    OLED_DrawString(66, 24, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) ? "H" : "L", OLED_FONT_6x8);
    OLED_DrawString(84, 24, "Idle:", OLED_FONT_6x8);
    OLED_DrawString(108, 24, I2C_IsBusIdle() ? "Y" : "N", OLED_FONT_6x8);

    OLED_DrawHLine(0, 32, 128);
    MarkPageDirty(4, 4);

    OLED_DrawString(0, 34, "I2C Err:", OLED_FONT_6x8);
    OLED_DrawInt(54, 34, I2C_GetErrorCount(), OLED_FONT_6x8);
    OLED_DrawString(72, 34, "Rcv:", OLED_FONT_6x8);
    OLED_DrawInt(96, 34, I2C_GetRecoveryCount(), OLED_FONT_6x8);

    OLED_DrawString(0, 42, "MPU:", OLED_FONT_6x8);
    OLED_DrawString(24, 42, MPU6050_IsDataValid() ? "OK" : "FAIL", OLED_FONT_6x8);
    OLED_DrawString(54, 42, "PCA:", OLED_FONT_6x8);
    OLED_DrawString(78, 42, PCA9685_IsOnline() ? "OK" : "FAIL", OLED_FONT_6x8);

    OLED_DrawString(0, 52, "FailStep:", OLED_FONT_6x8);
    OLED_DrawInt(60, 52, MPU6050_GetFailStep(), OLED_FONT_6x8);

    OLED_DrawString(0, 60, "Addr:0xD0=MPU 0x80=PCA", OLED_FONT_6x8);
}

void OLED_DispUpdate(const MPU6050_Angle_t *angle, uint8_t mpu_valid,
                     uint16_t mpu_errors, uint16_t mpu_reads,
                     const Arm_Position_t *target, const Arm_Position_t *current,
                     Arm_Mode_t mode, uint16_t i2c_err, uint16_t i2c_recv,
                     uint8_t knob_connected, float temp,
                     uint8_t pca_valid)
{
    if (HAL_GetTick() - last_disp_time < OLED_DISP_INTERVAL_MS) return;
    last_disp_time = HAL_GetTick();

    switch (current_page) {
        case DISP_PAGE_SENSOR:
            DrawSensorPage(angle, mpu_valid, mpu_errors, mpu_reads, temp);
            break;
        case DISP_PAGE_SERVO:
            DrawServoPage(target, current, mode);
            break;
        case DISP_PAGE_SYSTEM:
            DrawSystemPage(i2c_err, i2c_recv, knob_connected, mpu_valid, pca_valid);
            break;
        case DISP_PAGE_DEBUG:
            DrawDebugPage();
            break;
        default:
            break;
    }

    RefreshDirtyPages();
}

void OLED_DispShowInitProgress(const char *msg, uint8_t step, uint8_t total)
{
    OLED_Clear();

    OLED_DrawString(0, 0, "Robot Arm Init", OLED_FONT_6x8);
    OLED_DrawHLine(0, 8, 128);

    OLED_DrawString(0, 16, msg, OLED_FONT_6x8);

    OLED_DrawRect(8, 32, 112, 10);
    uint8_t fill = (step * 110) / total;
    for (uint8_t i = 0; i < fill; i++) {
        OLED_SetPixel(9 + i, 33, 1);
        OLED_SetPixel(9 + i, 34, 1);
        OLED_SetPixel(9 + i, 35, 1);
        OLED_SetPixel(9 + i, 36, 1);
        OLED_SetPixel(9 + i, 37, 1);
        OLED_SetPixel(9 + i, 38, 1);
        OLED_SetPixel(9 + i, 39, 1);
        OLED_SetPixel(9 + i, 40, 1);
    }

    OLED_Refresh();
}

void OLED_DispShowError(const char *msg)
{
    OLED_Clear();

    OLED_FillArea(0, 0, 128, 8, 0xFF);
    OLED_DrawString(2, 0, "!! ERROR !!", OLED_FONT_6x8);

    OLED_DrawString(0, 16, msg, OLED_FONT_6x8);

    OLED_DrawString(0, 48, "Reset in 5s...", OLED_FONT_6x8);

    OLED_Refresh();
}

void OLED_DispNextPage(void)
{
    current_page = (OLED_DispPage_t)((current_page + 1) % DISP_PAGE_COUNT);
}

OLED_DispPage_t OLED_DispGetPage(void)
{
    return current_page;
}
