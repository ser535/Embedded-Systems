#ifndef __KEYPAD_H
#define __KEYPAD_H

#include "stm32f4xx_hal.h"

/*
 * SDK-1.1M:
 * PCA9538 клавиатуры - адресный байт 0xE2 на I2C1.
 * P0..P3 - строки, P4..P6 - столбцы.
 */
uint8_t Keypad_Init(void);
char Keypad_GetKey(void);

#endif
