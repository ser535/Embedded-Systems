#ifndef TETRIS_H
#define TETRIS_H
#include <stdint.h>
#define TETRIS_W 10
#define TETRIS_HGT 20
/* Board belongs only to GameTask. DisplayTask receives a copy. */
typedef enum { TETRIS_HELP, TETRIS_PLAY, TETRIS_PAUSE, TETRIS_OVER, TETRIS_KEYPAD_ERROR } TetrisMode;
typedef struct {
    uint16_t rows[TETRIS_HGT];
    uint32_t score, lines;
    uint8_t level, piece, next, rotation;
    int8_t x, y;
    TetrisMode mode;
    uint32_t rng, lastFall;
    uint8_t bag[7], bagIndex;
} Tetris;
void Tetris_Init(Tetris *g, uint32_t seed);
void Tetris_Key(Tetris *g, char key, uint32_t now);
void Tetris_Tick(Tetris *g, uint32_t now);
uint16_t Tetris_Shape(uint8_t piece, uint8_t rotation);
void Tetris_Snapshot(const Tetris *g, Tetris *frame);
#endif
