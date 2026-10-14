#include "tetris_app.h"
#include "tetris_display.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"
#include "keypad.h"
#include "key_controls.h"
#include "tetris_audio.h"
#include "oled.h"
#include "i2c.h"
#include <string.h>
static QueueHandle_t keyQueue, frameQueue, audioQueue;
static SemaphoreHandle_t i2cMutex;
void TetrisApp_Init(void)
{
    keyQueue=xQueueCreate(16,sizeof(char));
    /* Latest complete frame wins; the game never waits for OLED traffic. */
    frameQueue=xQueueCreate(1,sizeof(Tetris));
    audioQueue=xQueueCreate(1,sizeof(AudioState));
    i2cMutex=xSemaphoreCreateMutex();
    configASSERT(keyQueue && frameQueue && audioQueue && i2cMutex);
    vQueueAddToRegistry(keyQueue,"Keys");
    vQueueAddToRegistry(frameQueue,"Frames");
    vQueueAddToRegistry(i2cMutex,"I2C1");
    vQueueAddToRegistry(audioQueue,"Audio");
}
void TetrisApp_InputTask(void const *argument)
{
    (void)argument;
    uint8_t ready=0, faultSent=0;
    KeyControls controls; KeyControls_Init(&controls);
    TickType_t wake=xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(i2cMutex,portMAX_DELAY);
        if (!ready) ready=Keypad_Init();
        uint16_t mask=0;
        if(ready && !Keypad_ReadMask(&mask)) ready=0;
        char key=0;
        xSemaphoreGive(i2cMutex);
        if (!ready) {
            if (!faultSent) {
                KeyControls_Init(&controls);
                key='!'; xQueueSend(keyQueue,&key,portMAX_DELAY); faultSent=1;
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); wake=xTaskGetTickCount(); continue;
        }
        if (faultSent) { char recovered='~'; xQueueSend(keyQueue,&recovered,portMAX_DELAY); faultSent=0; }
        char events[12];
        unsigned count=KeyControls_Update(&controls,mask,HAL_GetTick(),events);
        for(unsigned i=0;i<count;i++) xQueueSend(keyQueue,&events[i],portMAX_DELAY);
        vTaskDelayUntil(&wake,pdMS_TO_TICKS(10));
    }
}
void TetrisApp_GameTask(void const *argument)
{
    (void)argument;
    Tetris game, frame, previous;
    AudioState audio={TETRIS_HELP,0,0};
    xQueueOverwrite(audioQueue,&audio);
    Tetris_Init(&game,0x12345678);
    Tetris_Snapshot(&game,&frame); previous=frame;
    xQueueOverwrite(frameQueue,&frame);
    for (;;) {
        char key;
        uint32_t oldLines=game.lines;
        TetrisMode oldMode=game.mode;
        if (xQueueReceive(keyQueue,&key,pdMS_TO_TICKS(10))==pdPASS)
            Tetris_Key(&game,key,HAL_GetTick());
        Tetris_Tick(&game,HAL_GetTick());
        if(game.mode==TETRIS_PLAY && (oldMode==TETRIS_HELP || oldMode==TETRIS_OVER)) audio.session++;
        if(game.lines>oldLines) audio.clearSerial++;
        if(audio.mode!=game.mode || game.lines>oldLines ||
           (game.mode==TETRIS_PLAY && (oldMode==TETRIS_HELP || oldMode==TETRIS_OVER))) {
            audio.mode=game.mode; xQueueOverwrite(audioQueue,&audio);
        }
        Tetris_Snapshot(&game,&frame);
        if (memcmp(&frame,&previous,sizeof(frame))!=0) {
            xQueueOverwrite(frameQueue,&frame); previous=frame;
        }
    }
}
void TetrisApp_DisplayTask(void const *argument)
{
    (void)argument;
    Tetris frame;
    xSemaphoreTake(i2cMutex,portMAX_DELAY);
    if (HAL_I2C_IsDeviceReady(&hi2c1,OLED_I2C_ADDR,3,100)!=HAL_OK) Error_Handler();
    oled_Init();
    xSemaphoreGive(i2cMutex);
    TickType_t last=xTaskGetTickCount();
    for (;;) {
        xQueueReceive(frameQueue,&frame,portMAX_DELAY);
        /* Drawing does not touch I2C; only DisplayTask owns the framebuffer. */
        Tetris_Draw(&frame);
        xSemaphoreTake(i2cMutex,portMAX_DELAY);
        oled_UpdateScreen();
        xSemaphoreGive(i2cMutex);
        /* Limit display traffic to <= 20 fps, leave time for keypad polling. */
        vTaskDelayUntil(&last,pdMS_TO_TICKS(50));
        last=xTaskGetTickCount();
    }
}

void TetrisApp_AudioTask(void const *argument)
{
    (void)argument;
    AudioPlayer player; AudioPlayer_Init(&player);
    AudioState state={TETRIS_HELP,0,0},incoming;
    Buzzer_Init();
    TickType_t wake=xTaskGetTickCount();
    for(;;) {
        if(xQueueReceive(audioQueue,&incoming,0)==pdPASS) state=incoming;
        Buzzer_SetHz(AudioPlayer_Update(&player,&state,HAL_GetTick()));
        vTaskDelayUntil(&wake,pdMS_TO_TICKS(5));
    }
}
