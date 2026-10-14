#include "buzzer.h"

#define BUZZER_FREQUENCY_HZ 2000U
#define TIMER_COUNTER_HZ    1000000U

static uint32_t tim1_clock_hz(void)
{
    RCC_ClkInitTypeDef clk = {0};
    uint32_t flash_latency = 0;
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

    HAL_RCC_GetClockConfig(&clk, &flash_latency);
    if (clk.APB2CLKDivider != RCC_HCLK_DIV1) {
        pclk2 *= 2U;
    }
    return pclk2;
}

void Buzzer_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    uint32_t timer_clock = tim1_clock_hz();
    uint32_t period;

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* Вход пищалки SDK-1.1M: PE9 = TIM1_CH1. */
    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOE, &gpio);

    TIM1->CR1 = 0;
    TIM1->PSC = (timer_clock / TIMER_COUNTER_HZ) - 1U;
    period = TIMER_COUNTER_HZ / BUZZER_FREQUENCY_HZ;
    TIM1->ARR = period - 1U;
    TIM1->CCR1 = 0;

    /* Режим PWM 1 на канале 1, предварительная загрузка включена. */
    TIM1->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE;
    TIM1->CCER = TIM_CCER_CC1E;
    TIM1->BDTR = TIM_BDTR_MOE;
    TIM1->CR1 = TIM_CR1_ARPE;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;
}

void Buzzer_On(void)
{
    TIM1->CCR1 = (TIM1->ARR + 1U) / 2U; /* 50% duty cycle */
}

void Buzzer_Off(void)
{
    TIM1->CCR1 = 0;
}
