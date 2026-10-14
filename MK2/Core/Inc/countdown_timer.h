#ifndef __COUNTDOWN_TIMER_H
#define __COUNTDOWN_TIMER_H

#include "stm32f4xx_hal.h"

void CountdownTimer_Init(void);
void CountdownTimer_Start(void);
void CountdownTimer_Stop(void);
uint32_t CountdownTimer_GetTicks(void);

#endif
