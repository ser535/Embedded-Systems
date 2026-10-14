#include "tetris.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    Tetris g, f, before;
    for(int p=0;p<7;p++) for(int r=0;r<4;r++) {
        uint16_t m=Tetris_Shape(p,r); int n=0;
        for(int i=0;i<16;i++) n+=(m>>i)&1;
        assert(n==4);
    }
    Tetris_Init(&g,1);
    Tetris_Key(&g,'4',500); assert(g.mode==TETRIS_HELP);
    Tetris_Key(&g,'*',1000); assert(g.mode==TETRIS_PLAY);
    before=g; Tetris_Snapshot(&g,&f);
    assert(memcmp(&g,&before,sizeof(g))==0); /* rendering never modifies game */
    assert(memcmp(f.rows,g.rows,sizeof(g.rows))!=0);
    for(int i=0;i<40;i++) Tetris_Key(&g,'4',1000);
    assert(g.x>=-2);
    Tetris_Key(&g,'#',1100); before=g;
    Tetris_Tick(&g,10000); Tetris_Key(&g,'0',10000);
    assert(memcmp(&g,&before,sizeof(g))==0);
    Tetris_Key(&g,'#',10000); assert(g.mode==TETRIS_PLAY);
    int y=g.y; Tetris_Tick(&g,10699); assert(g.y==y);
    Tetris_Tick(&g,10700); assert(g.y==y+1);
    /* Four lines in one lock, all are compacted, score is 800. */
    Tetris_Init(&g,1); g.mode=TETRIS_PLAY; g.piece=0; g.rotation=1; g.x=3; g.y=16;
    for(int i=16;i<20;i++) g.rows[i]=0x3ff & ~(1U<<4);
    Tetris_Key(&g,'0',1);
    assert(g.lines==4 && g.score==800);
    for(int i=0;i<20;i++) assert(g.rows[i]==0);
    /* Level progression uses old level for scoring. */
    g.lines=9; g.piece=0; g.rotation=0; g.x=3; g.y=18;
    g.rows[19]=0x3ff & ~(15U<<3);
    Tetris_Key(&g,'0',2); assert(g.lines==10 && g.level==2 && g.score==900);
    /* Top collision -> game over -> * starts a clean board. */
    for(int i=0;i<20;i++) g.rows[i]=1U<<4;
    g.rows[0]=0; g.rows[1]=0; g.rows[19]=0;
    g.piece=1; g.rotation=0; g.x=3; g.y=0;
    Tetris_Key(&g,'0',3); assert(g.mode==TETRIS_OVER);
    Tetris_Key(&g,'*',4); assert(g.mode==TETRIS_PLAY && g.lines==0 && g.score==0);
    /* Verify each bag contains all seven pieces once. */
    Tetris_Init(&g,123); Tetris_Key(&g,'*',1);
    for(int b=0;b<20;b++) {
        unsigned seen=0;
        for(int i=0;i<7;i++) {
            assert(!(seen & (1U<<g.piece))); seen|=1U<<g.piece;
            memset(g.rows,0,sizeof(g.rows)); Tetris_Key(&g,'0',2+b*7+i);
        }
        assert(seen==127);
    }
    /* Unsigned tick wrap. */
    Tetris_Init(&g,1); Tetris_Key(&g,'*',UINT32_MAX-100);
    y=g.y; Tetris_Tick(&g,598); assert(g.y==y);
    Tetris_Tick(&g,599); assert(g.y==y+1);
    /* Deterministic stress run: moves, rotation, lock, restart, bounds. */
    uint32_t rng=1;
    const char keys[]="44662258800#*";
    for(unsigned i=0;i<100000;i++) {
        rng=rng*1664525U+1013904223U;
        Tetris_Key(&g,keys[rng%(sizeof(keys)-1)],i*31);
        Tetris_Tick(&g,i*31);
        for(int r=0;r<20;r++) assert((g.rows[r]&~0x3ffU)==0);
        Tetris_Snapshot(&g,&f);
    }
    puts("PASS: pieces, help, boundaries, snapshot, pause, gravity, 1/4-line clears, level, game over, restart, seven-bag, tick wrap, 100000 stress steps");
}
