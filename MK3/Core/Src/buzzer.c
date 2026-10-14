#include "tetris_audio.h"
#include "main.h"
/* SDK-1.1M buzzer PE9=TIM1_CH1. TIM6 must own HAL timebase. */
void Buzzer_Init(void)
{
 GPIO_InitTypeDef gpio={0};RCC_ClkInitTypeDef clk={0};uint32_t latency;
 __HAL_RCC_GPIOE_CLK_ENABLE();__HAL_RCC_TIM1_CLK_ENABLE();
 gpio.Pin=GPIO_PIN_9;gpio.Mode=GPIO_MODE_AF_PP;gpio.Pull=GPIO_NOPULL;
 gpio.Speed=GPIO_SPEED_FREQ_LOW;gpio.Alternate=GPIO_AF1_TIM1;
 HAL_GPIO_Init(GPIOE,&gpio);
 HAL_RCC_GetClockConfig(&clk,&latency);
 uint32_t hz=HAL_RCC_GetPCLK2Freq();
 if(clk.APB2CLKDivider!=RCC_HCLK_DIV1) hz*=2U;
 TIM1->CR1=0;TIM1->DIER=0;TIM1->PSC=hz/1000000U-1U;
 TIM1->ARR=999;TIM1->CCR1=0;TIM1->RCR=0;
 TIM1->CCMR1=TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1PE;
 TIM1->CCER=TIM_CCER_CC1E;TIM1->BDTR=TIM_BDTR_MOE;
 TIM1->CR1=TIM_CR1_ARPE;TIM1->EGR=TIM_EGR_UG;TIM1->SR=0;
 TIM1->CR1|=TIM_CR1_CEN;
}
void Buzzer_SetHz(uint16_t hz)
{
 static uint16_t previous=0;
 if(hz==previous) return;
 previous=hz;
 if(!hz) {TIM1->CCR1=0;return;}
 uint32_t period=1000000U/hz;
 TIM1->CR1&=~TIM_CR1_CEN;
 TIM1->ARR=period-1U;TIM1->CCR1=period/2U;TIM1->CNT=0;
 TIM1->EGR=TIM_EGR_UG;TIM1->SR=0;TIM1->CR1|=TIM_CR1_CEN;
}
