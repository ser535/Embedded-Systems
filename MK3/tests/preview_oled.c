/* Host capture of actual oled.c + fonts.c output; no physical I2C. */
#include "oled.h"
#include "tetris_display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
I2C_HandleTypeDef hi2c1;
static uint8_t pixels[1024], page;
void HAL_Delay(uint32_t ms) { (void)ms; }
int HAL_I2C_Mem_Write(I2C_HandleTypeDef *h,uint16_t addr,uint16_t reg,uint16_t sz,uint8_t *data,uint16_t n,uint32_t timeout)
{
    (void)h; (void)addr; (void)sz; (void)timeout;
    if(reg==0 && n==1 && (*data & 0xf8)==0xb0) page=*data & 7;
    if(reg==0x40 && n==128) memcpy(pixels+page*128,data,n);
    return 0;
}
static void save(const char *path)
{
    FILE *f=fopen(path,"wb"); if(!f) exit(1);
    fprintf(f,"P5\n128 64\n255\n");
    for(int y=0;y<64;y++) for(int x=0;x<128;x++)
        fputc((pixels[x+(y/8)*128] & (1U<<(y%8)))?255:0,f);
    fclose(f);
}
int main(int argc,char **argv)
{
    if(argc!=2) return 1;
    Tetris g,frame; char path[1024];
    oled_Init(); Tetris_Init(&g,123);
    Tetris_Draw(&g); oled_UpdateScreen();
    snprintf(path,sizeof(path),"%s/help.pgm",argv[1]); save(path);
    Tetris_Key(&g,'*',1000);
    for(int i=0;i<5;i++) { Tetris_Key(&g,'6',1001); Tetris_Key(&g,'0',1002); }
    Tetris_Snapshot(&g,&frame); Tetris_Draw(&frame); oled_UpdateScreen();
    snprintf(path,sizeof(path),"%s/game.pgm",argv[1]); save(path);
    g.mode=TETRIS_PAUSE; Tetris_Snapshot(&g,&frame); Tetris_Draw(&frame); oled_UpdateScreen();
    snprintf(path,sizeof(path),"%s/pause.pgm",argv[1]); save(path);
    g.mode=TETRIS_OVER; Tetris_Draw(&g); oled_UpdateScreen();
    snprintf(path,sizeof(path),"%s/gameover.pgm",argv[1]); save(path);
}
