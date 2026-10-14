#ifndef TETRIS_AUDIO_H
#define TETRIS_AUDIO_H
#include "tetris.h"
typedef struct { TetrisMode mode; uint32_t clearSerial,session; } AudioState;
typedef struct {
 uint32_t previousTime,clearSerial,session,noteElapsed,effectElapsed;
 unsigned note;
 uint8_t initialized,effect;
} AudioPlayer;
void AudioPlayer_Init(AudioPlayer *p);
/* Pure state machine. Return desired PWM frequency, zero means silence. */
uint16_t AudioPlayer_Update(AudioPlayer *p,const AudioState *s,uint32_t now);
void Buzzer_Init(void);
void Buzzer_SetHz(uint16_t hz);
#endif
