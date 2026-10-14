#include "key_controls.h"
#include "tetris_audio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
 KeyControls k;char ev[12];unsigned n;KeyControls_Init(&k);
 assert(!KeyControls_Update(&k,1U<<3,0,ev));
 assert(!KeyControls_Update(&k,1U<<3,29,ev));
 n=KeyControls_Update(&k,1U<<3,30,ev);assert(n==1 && ev[0]=='4');
 assert(!KeyControls_Update(&k,1U<<3,279,ev));
 n=KeyControls_Update(&k,1U<<3,280,ev);assert(n==1 && ev[0]=='4');
 n=KeyControls_Update(&k,(1U<<3)|(1U<<1),300,ev);assert(n==0);
 n=KeyControls_Update(&k,(1U<<3)|(1U<<1),330,ev);assert(n==1 && ev[0]=='2');
 n=KeyControls_Update(&k,(1U<<3)|(1U<<1),360,ev);assert(n==1 && ev[0]=='4');
 n=KeyControls_Update(&k,(1U<<3)|(1U<<1),440,ev);assert(n==1 && ev[0]=='4');
 /* Release movement stops repeats immediately, rotation remains one-shot. */
 assert(!KeyControls_Update(&k,1U<<1,520,ev));
 assert(!KeyControls_Update(&k,1U<<1,550,ev));
 assert(!KeyControls_Update(&k,1U<<1,1000,ev));
 for(unsigned a=0;a<12;a++) for(unsigned b=a+1;b<12;b++)
  assert(!KeyControls_Ambiguous((1U<<a)|(1U<<b)));
 assert(KeyControls_Ambiguous((1U<<0)|(1U<<1)|(1U<<3)|(1U<<4)));
 /* Every pair can produce two independent press events. */
 for(unsigned a=0;a<12;a++) for(unsigned b=a+1;b<12;b++) {
  KeyControls_Init(&k);uint16_t mask=(1U<<a)|(1U<<b);
  KeyControls_Update(&k,mask,0,ev);assert(KeyControls_Update(&k,mask,30,ev)==2);
 }
 KeyControls_Init(&k);KeyControls_Update(&k,1U<<5,UINT32_MAX-10,ev);
 assert(KeyControls_Update(&k,1U<<5,19,ev)==1);
 /* End-to-end: held right and freshly pressed rotate update same game. */
 Tetris g;Tetris_Init(&g,1);Tetris_Key(&g,'*',0);
 g.piece=2;g.x=3;g.y=0;g.rotation=0;
 KeyControls_Init(&k);KeyControls_Update(&k,1U<<5,0,ev);
 n=KeyControls_Update(&k,1U<<5,30,ev);
 for(unsigned i=0;i<n;i++) Tetris_Key(&g,ev[i],30);
 KeyControls_Update(&k,(1U<<5)|(1U<<1),250,ev);
 n=KeyControls_Update(&k,(1U<<5)|(1U<<1),280,ev);assert(n==2);
 for(unsigned i=0;i<n;i++) Tetris_Key(&g,ev[i],280);
 assert(g.x==5 && g.rotation==1);
 AudioPlayer p;AudioPlayer_Init(&p);AudioState s={TETRIS_HELP,0,0};
 assert(!AudioPlayer_Update(&p,&s,0));
 s.mode=TETRIS_PLAY;s.session=1;assert(AudioPlayer_Update(&p,&s,100)==659);
 AudioPlayer_Update(&p,&s,200);uint32_t pos=p.noteElapsed;
 s.clearSerial=1;assert(AudioPlayer_Update(&p,&s,205)==1047);
 assert(AudioPlayer_Update(&p,&s,295)==0);
 assert(AudioPlayer_Update(&p,&s,325)==1397);
 assert(AudioPlayer_Update(&p,&s,415)==0);
 assert(AudioPlayer_Update(&p,&s,445)==659 && p.noteElapsed==pos);
 s.mode=TETRIS_PAUSE;assert(!AudioPlayer_Update(&p,&s,450));
 pos=p.noteElapsed;AudioPlayer_Update(&p,&s,9999);assert(p.noteElapsed==pos);
 s.mode=TETRIS_PLAY;assert(AudioPlayer_Update(&p,&s,10004)==659 && p.noteElapsed==pos+5);
 s.mode=TETRIS_HELP;assert(!AudioPlayer_Update(&p,&s,10009));
 s.mode=TETRIS_PLAY;s.session++;assert(AudioPlayer_Update(&p,&s,10014)==659 && p.noteElapsed==0);
 /* Clearing followed immediately by pause must not lose the pending beep. */
 s.clearSerial++;s.mode=TETRIS_PAUSE;assert(!AudioPlayer_Update(&p,&s,10015));
 s.mode=TETRIS_PLAY;assert(AudioPlayer_Update(&p,&s,10016)==1047);
 for(unsigned i=0;i<100000;i++) AudioPlayer_Update(&p,&s,10019+i*5);
 puts("PASS: debounce, hold/repeat/release, all 66 key pairs, ghost rectangle, tick wrap, move+rotate integration, melody, clear interruption/resume, pause/resume, restart, audio stress");
}
