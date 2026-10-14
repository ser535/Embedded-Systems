#ifndef TETRIS_DISPLAY_H
#define TETRIS_DISPLAY_H
#include "tetris.h"
/* Draws only to the OLED RAM buffer; caller sends it under the I2C mutex. */
void Tetris_Draw(const Tetris *frame);
#endif
