/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "oled.h"
#include "fonts.h"
#include "keypad.h"
#include "countdown_timer.h"
#include "buzzer.h"

#include <stdio.h>

#define MAX_INPUT_SECONDS 99999U
#define ALARM_SECONDS     3U

typedef enum {
    MODE_SET = 0,
    MODE_RUNNING,
    MODE_ALARM
} AppMode;

static void Display_KeypadLegend(void);
static void Display_SetValue(uint32_t value);
static void Display_Remaining(uint32_t value);
static void Display_Alarm(void);
static void DrawNumber(uint32_t value, uint8_t y);
void SystemClock_Config(void);

static void DrawNumber(uint32_t value, uint8_t y)
{
    char buffer[12];
    (void)snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)value);

    uint8_t len = 0;
    while (buffer[len] != '\0') len++;

    uint8_t width = (uint8_t)(len * Font_16x26.FontWidth);
    uint8_t x = (width < 128U) ? (uint8_t)((128U - width) / 2U) : 0U;

    oled_SetCursor(x, y);
    oled_WriteString(buffer, Font_16x26, White);
}

static void Display_KeypadLegend(void)
{
    oled_Fill(Black);

    oled_SetCursor(43, 0);
    oled_WriteString("KEYPAD", Font_7x10, White);

    oled_SetCursor(46, 10);
    oled_WriteString("1 2 3", Font_7x10, White);

    oled_SetCursor(46, 20);
    oled_WriteString("4 5 6", Font_7x10, White);

    oled_SetCursor(46, 30);
    oled_WriteString("7 8 9", Font_7x10, White);

    oled_SetCursor(46, 40);
    oled_WriteString("* 0 #", Font_7x10, White);

    oled_SetCursor(4, 53);
    oled_WriteString("PRESS * FOR TIMER", Font_7x10, White);

    oled_UpdateScreen();
}

static void Display_SetValue(uint32_t value)
{
    oled_Fill(Black);
    oled_SetCursor(25, 0);
    oled_WriteString("SET SECONDS", Font_7x10, White);
    DrawNumber(value, 18);
    oled_SetCursor(8, 53);
    oled_WriteString("* CLEAR  # START", Font_7x10, White);
    oled_UpdateScreen();
}

static void Display_Remaining(uint32_t value)
{
    oled_Fill(Black);
    oled_SetCursor(28, 0);
    oled_WriteString("REMAINING", Font_7x10, White);
    DrawNumber(value, 18);
    oled_SetCursor(22, 53);
    oled_WriteString("* CANCEL", Font_7x10, White);
    oled_UpdateScreen();
}

static void Display_Alarm(void)
{
    oled_Fill(Black);
    oled_SetCursor(31, 5);
    oled_WriteString("TIMER", Font_11x18, White);
    oled_SetCursor(20, 27);
    oled_WriteString("TIME UP!", Font_11x18, White);
    oled_SetCursor(11, 54);
    oled_WriteString("PRESS ANY KEY", Font_7x10, White);
    oled_UpdateScreen();
}

int main(void)
{
    AppMode mode = MODE_SET;
    uint32_t input_seconds = 0;
    uint32_t remaining = 0;
    uint32_t last_second = 0;
    uint32_t alarm_started = 0;

    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    oled_Init();

    if (!Keypad_Init()) {
        oled_Fill(Black);
        oled_SetCursor(15, 12);
        oled_WriteString("KEYPAD ERROR", Font_11x18, White);
        oled_SetCursor(22, 40);
        oled_WriteString("I2C 0xE2", Font_7x10, White);
        oled_UpdateScreen();

        while (1) {
            HAL_Delay(100);
        }
    }

    CountdownTimer_Init();
    Buzzer_Init();
    Buzzer_Off();

    Display_KeypadLegend();

    /* В начальный экран таймера переходим только после нажатия '*'. */
    while (Keypad_GetKey() != '*') {
        HAL_Delay(5);
    }

    Display_SetValue(input_seconds);

    while (1)
    {
        char key = Keypad_GetKey();

        if (key != 0) {
            if (mode == MODE_ALARM) {
                Buzzer_Off();
                CountdownTimer_Stop();
                input_seconds = 0;
                remaining = 0;
                mode = MODE_SET;
                Display_SetValue(input_seconds);
                continue;
            }

            if (key >= '0' && key <= '9') {
                if (mode == MODE_SET) {
                    uint32_t digit = (uint32_t)(key - '0');
                    if (input_seconds <= (MAX_INPUT_SECONDS - digit) / 10U) {
                        input_seconds = input_seconds * 10U + digit;
                        Display_SetValue(input_seconds);
                    }
                }
            }
            else if (key == '#') {
                if (mode == MODE_SET && input_seconds > 0U) {
                    remaining = input_seconds;
                    last_second = 0;
                    CountdownTimer_Start();
                    mode = MODE_RUNNING;
                    Display_Remaining(remaining);
                }
            }
            else if (key == '*') {
                Buzzer_Off();
                CountdownTimer_Stop();
                input_seconds = 0;
                remaining = 0;
                mode = MODE_SET;
                Display_SetValue(input_seconds);
            }
        }

        if (mode == MODE_RUNNING) {
            uint32_t now = CountdownTimer_GetTicks();

            while (last_second < now && mode == MODE_RUNNING) {
                last_second++;

                if (remaining > 0U) {
                    remaining--;
                }

                if (remaining == 0U) {
                    mode = MODE_ALARM;
                    alarm_started = last_second;
                    Buzzer_On();
                    Display_Alarm();
                } else {
                    Display_Remaining(remaining);
                }
            }
        }
        else if (mode == MODE_ALARM) {
            uint32_t now = CountdownTimer_GetTicks();
            if ((uint32_t)(now - alarm_started) >= ALARM_SECONDS) {
                Buzzer_Off();
                CountdownTimer_Stop();
                /* TIME UP remains on the display until a key is pressed. */
            }
        }
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  *
  * Kept from the known-working SDK_OLED project: HSI -> PLL -> 120 MHz.
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 120;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }

    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
