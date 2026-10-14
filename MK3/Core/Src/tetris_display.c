#include "tetris_display.h"
#include "oled.h"
#include <stdio.h>
static void text(int x,int y,const char *s)
{
    oled_SetCursor((uint8_t)x,(uint8_t)y);
    oled_WriteString((char *)s,Font_7x10,White);
}
void Tetris_Draw(const Tetris *f)
{
    char s[12];
    oled_Fill(Black);
    if (f->mode==TETRIS_HELP) {
        /* Keyboard layout on the left, action legend on the right. */
        text(0,0,"1 2 3"); text(42,0,"2/5 TURN");
        text(0,10,"4 5 6"); text(42,10,"4< 6> 8v");
        text(0,20,"7 8 9"); text(42,20,"0 DROP");
        text(0,30,"* 0 #"); text(42,30,"# PAUSE");
        text(0,40,"* HELP / RESTART");
        text(0,52,"PRESS * TO PLAY"); return;
    }
    if (f->mode==TETRIS_KEYPAD_ERROR) {
        text(7,10,"KEYPAD ERROR"); text(7,25,"I2C ADDR 0x71");
        text(7,42,"AUTO RETRY..."); return;
    }
    if (f->mode==TETRIS_OVER) {
        text(28,0,"GAME OVER"); text(0,15,"SCORE");
        snprintf(s,sizeof(s),"%lu",(unsigned long)f->score); text(45,15,s);
        text(0,30,"LINES"); snprintf(s,sizeof(s),"%lu",(unsigned long)f->lines); text(45,30,s);
        text(0,52,"* RESTART"); return;
    }
    /* 10 x 20 cells of 3 x 3 px: full-height board in landscape OLED. */
    oled_DrawSquare(0,31,0,61,White);
    for (int y=0;y<TETRIS_HGT;y++) for (int x=0;x<TETRIS_W;x++)
        if (f->rows[y] & (1U<<x)) {
            int px=1+3*x, py=1+3*y;
            oled_DrawPixel(px,py,White); oled_DrawPixel(px+1,py,White);
            oled_DrawPixel(px,py+1,White); oled_DrawPixel(px+1,py+1,White);
        }
    text(36,0,f->mode==TETRIS_PAUSE?"PAUSED":"TETRIS");
    snprintf(s,sizeof(s),"S:%lu",(unsigned long)f->score); text(36,10,s);
    snprintf(s,sizeof(s),"L:%lu",(unsigned long)f->lines); text(36,20,s);
    snprintf(s,sizeof(s),"LV:%u",f->level); text(36,30,s);
    text(36,42,"NEXT");
    uint16_t mask=Tetris_Shape(f->next,0);
    for(int r=0;r<4;r++) for(int c=0;c<4;c++) if(mask & (1U<<(r*4+c))) {
        int px=75+c*4,py=43+r*4;
        oled_DrawSquare(px,px+2,py,py+2,White);
    }
    text(36,53,"# PAUSE");
}
