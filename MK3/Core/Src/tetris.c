#include "tetris.h"
#include <string.h>
/* Each nibble is a row, low bit is left. I O T S Z J L. */
static const uint16_t shapes[7][4] = {
    {0x00F0,0x2222,0x00F0,0x2222},
    {0x0066,0x0066,0x0066,0x0066},
    {0x0072,0x0262,0x0270,0x0232},
    {0x0036,0x0462,0x0036,0x0462},
    {0x0063,0x0264,0x0063,0x0264},
    {0x0071,0x0226,0x0470,0x0322},
    {0x0074,0x0622,0x0170,0x0223}
};
uint16_t Tetris_Shape(uint8_t p, uint8_t r) { return shapes[p % 7][r % 4]; }
static uint32_t random32(Tetris *g)
{
    uint32_t x = g->rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return g->rng = x;
}
static uint8_t draw(Tetris *g)
{
    if (g->bagIndex >= 7) {
        for (int i=0;i<7;i++) g->bag[i]=(uint8_t)i;
        for (int i=6;i>0;i--) {
            int j=(int)(random32(g) % (uint32_t)(i+1));
            uint8_t t=g->bag[i]; g->bag[i]=g->bag[j]; g->bag[j]=t;
        }
        g->bagIndex=0;
    }
    return g->bag[g->bagIndex++];
}
static int fits(const Tetris *g, int x, int y, int rotation)
{
    uint16_t mask=Tetris_Shape(g->piece,(uint8_t)rotation);
    for (int r=0;r<4;r++) for (int c=0;c<4;c++) {
        if (!(mask & (1U << (r*4+c)))) continue;
        int bx=x+c, by=y+r;
        if (bx<0 || bx>=TETRIS_W || by>=TETRIS_HGT) return 0;
        if (by>=0 && (g->rows[by] & (1U<<bx))) return 0;
    }
    return 1;
}
static void spawn(Tetris *g)
{
    g->piece=g->next; g->next=draw(g);
    g->rotation=0; g->x=3; g->y=0;
    if (!fits(g,g->x,g->y,0)) g->mode=TETRIS_OVER;
}
static void start(Tetris *g, uint32_t now)
{
    uint32_t seed=g->rng ^ now;
    Tetris_Init(g,seed); g->mode=TETRIS_PLAY;
    spawn(g); g->lastFall=now;
}
void Tetris_Init(Tetris *g, uint32_t seed)
{
    memset(g,0,sizeof(*g));
    g->rng=seed ? seed : 0xA341316CU;
    g->bagIndex=7; g->next=draw(g); g->level=1;
    g->mode=TETRIS_HELP;
}
static void lock(Tetris *g)
{
    uint16_t mask=Tetris_Shape(g->piece,g->rotation);
    for (int r=0;r<4;r++) for (int c=0;c<4;c++) {
        if (!(mask & (1U<<(r*4+c)))) continue;
        int y=g->y+r;
        if (y<0) { g->mode=TETRIS_OVER; return; }
        g->rows[y] |= (uint16_t)(1U<<(g->x+c));
    }
    unsigned cleared=0;
    for (int y=TETRIS_HGT-1;y>=0;) {
        if (g->rows[y]==0x03FFU) {
            for (int k=y;k>0;k--) g->rows[k]=g->rows[k-1];
            g->rows[0]=0; cleared++;
        } else y--;
    }
    static const uint16_t points[5]={0,100,300,500,800};
    g->score+=(uint32_t)points[cleared]*g->level;
    g->lines+=cleared;
    uint32_t level=1+g->lines/10;
    g->level=(uint8_t)(level>15?15:level);
    spawn(g);
}
static void down(Tetris *g)
{
    if (fits(g,g->x,g->y+1,g->rotation)) g->y++;
    else lock(g);
}
void Tetris_Key(Tetris *g, char key, uint32_t now)
{
    if (key=='!') { g->mode=TETRIS_KEYPAD_ERROR; return; }
    if (key=='~') { if (g->mode==TETRIS_KEYPAD_ERROR) g->mode=TETRIS_HELP; return; }
    if (g->mode==TETRIS_HELP || g->mode==TETRIS_OVER) {
        if (key=='*') start(g,now);
        return;
    }
    if (g->mode==TETRIS_KEYPAD_ERROR) return;
    if (key=='#') {
        g->mode=g->mode==TETRIS_PAUSE?TETRIS_PLAY:TETRIS_PAUSE;
        g->lastFall=now; return;
    }
    /* Return to instructions before restarting, so accidental * is harmless. */
    if (key=='*') { g->mode=TETRIS_HELP; return; }
    if (g->mode!=TETRIS_PLAY) return;
    if (key=='4' || key=='6') {
        int dx=key=='4'?-1:1;
        if (fits(g,g->x+dx,g->y,g->rotation)) g->x+=(int8_t)dx;
    } else if (key=='2' || key=='5') {
        int r=(g->rotation+1)%4;
        /* Simple wall kicks, not the full tournament SRS. */
        static const int8_t kicks[5]={0,-1,1,-2,2};
        for (int i=0;i<5;i++) if (fits(g,g->x+kicks[i],g->y,r)) {
            g->x+=kicks[i]; g->rotation=(uint8_t)r; break;
        }
    } else if (key=='8') {
        if (fits(g,g->x,g->y+1,g->rotation)) g->score++;
        down(g); g->lastFall=now;
    } else if (key=='0') {
        while (fits(g,g->x,g->y+1,g->rotation)) { g->y++; g->score+=2; }
        lock(g); g->lastFall=now;
    }
}
void Tetris_Tick(Tetris *g, uint32_t now)
{
    if (g->mode!=TETRIS_PLAY) return;
    uint32_t interval=700U-(uint32_t)(g->level-1)*40U;
    if ((uint32_t)(now-g->lastFall)>=interval) {
        down(g); g->lastFall=now;
    }
}
void Tetris_Snapshot(const Tetris *g, Tetris *frame)
{
    *frame=*g;
    if (g->mode!=TETRIS_PLAY && g->mode!=TETRIS_PAUSE) return;
    uint16_t mask=Tetris_Shape(g->piece,g->rotation);
    for (int r=0;r<4;r++) for (int c=0;c<4;c++) {
        int x=g->x+c, y=g->y+r;
        if ((mask & (1U<<(r*4+c))) && x>=0 && x<TETRIS_W && y>=0 && y<TETRIS_HGT)
            frame->rows[y] |= (uint16_t)(1U<<x);
    }
}
