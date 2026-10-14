#include "countdown_timer.h"

static volatile uint32_t second_ticks = 0;

static uint32_t tim6_clock_hz(void)
{
    RCC_ClkInitTypeDef clk = {0};
    uint32_t flash_latency = 0;
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();

    HAL_RCC_GetClockConfig(&clk, &flash_latency);
    if (clk.APB1CLKDivider != RCC_HCLK_DIV1) {
        pclk1 *= 2U;
    }
    return pclk1;
}

void CountdownTimer_Init(void)
{
    uint32_t timer_clock = tim6_clock_hz();

    __HAL_RCC_TIM6_CLK_ENABLE();

    TIM6->CR1 = 0;
    TIM6->DIER = 0;
    TIM6->PSC = (timer_clock / 10000U) - 1U; /* счётчик = 10 кГц */
    TIM6->ARR = 10000U - 1U;                /* обновление = 1 Гц */
    TIM6->CNT = 0;
    TIM6->EGR = TIM_EGR_UG;
    TIM6->SR = 0;
    TIM6->DIER = TIM_DIER_UIE;

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

void CountdownTimer_Start(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    second_ticks = 0;
    TIM6->CR1 &= ~TIM_CR1_CEN;
    TIM6->CNT = 0;
    TIM6->SR = 0;
    TIM6->CR1 |= TIM_CR1_CEN;

    __set_PRIMASK(primask);
}

void CountdownTimer_Stop(void)
{
    TIM6->CR1 &= ~TIM_CR1_CEN;
}

uint32_t CountdownTimer_GetTicks(void)
{
    return second_ticks;
}

/* Имя обработчика прерывания TIM6 на STM32F4. */
void TIM6_DAC_IRQHandler(void)
{
    if ((TIM6->SR & TIM_SR_UIF) != 0U) {
        TIM6->SR &= ~TIM_SR_UIF;
        second_ticks++;
    }
}
