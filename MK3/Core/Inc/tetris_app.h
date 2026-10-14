#ifndef TETRIS_APP_H
#define TETRIS_APP_H
void TetrisApp_Init(void);
void TetrisApp_InputTask(void const *argument);
void TetrisApp_GameTask(void const *argument);
void TetrisApp_DisplayTask(void const *argument);
void TetrisApp_AudioTask(void const *argument);
#endif
