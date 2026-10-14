#include "tetris_audio.h"
#include <string.h>
/* Traditional melody Korobeiniki (Tetris Theme A).
   Pitch/rhythm reference, Robson Couto, 2019:
   https://github.com/robsoncouto/arduino-songs/blob/master/tetris/tetris.ino
   Playback code below is a new nonblocking FreeRTOS-compatible implementation.
   Duration: quarter=4, eighth=8, dotted quarter=-4. Tempo 144 BPM. */
typedef struct { uint16_t hz; int8_t duration; } Note;
static const Note melody[]={
 {659,4},{494,8},{523,8},{587,4},{523,8},{494,8},
 {440,4},{440,8},{523,8},{659,4},{587,8},{523,8},
 {494,-4},{523,8},{587,4},{659,4},
 {523,4},{440,4},{440,4},{0,4},
 {0,8},{587,4},{698,8},{880,4},{784,8},{698,8},
 {659,-4},{523,8},{659,4},{587,8},{523,8},
 {494,4},{494,8},{523,8},{587,4},{659,4},
 {523,4},{440,4},{440,4},{0,4},
 {659,2},{523,2},{587,2},{494,2},{523,2},{440,2},{494,1},
 {659,2},{523,2},{587,2},{494,2},{523,4},{659,4},{880,2},{831,1}
};
static uint32_t duration(unsigned i)
{
 int d=melody[i].duration;
 uint32_t ms=(60000U*4U/144U)/(uint32_t)(d<0?-d:d);
 return d<0?ms*3U/2U:ms;
}
void AudioPlayer_Init(AudioPlayer *p) { memset(p,0,sizeof(*p)); }
uint16_t AudioPlayer_Update(AudioPlayer *p,const AudioState *s,uint32_t now)
{
 uint32_t dt=p->initialized?(uint32_t)(now-p->previousTime):0;
 p->previousTime=now;p->initialized=1;
 if(s->session!=p->session) {
  p->session=s->session;p->note=0;p->noteElapsed=0;p->effect=0;dt=0;
 }
 if(s->mode!=TETRIS_PLAY) {
  if(s->mode!=TETRIS_PAUSE) {
   p->clearSerial=s->clearSerial;p->note=0;p->noteElapsed=0;p->effect=0;
  }
  return 0;
 }
 if(s->clearSerial!=p->clearSerial) {
  p->clearSerial=s->clearSerial;p->effect=1;p->effectElapsed=0;dt=0;
 }
 if(p->effect) {
  p->effectElapsed+=dt;
  if(p->effectElapsed<90) return 1047;
  if(p->effectElapsed<120) return 0;
  if(p->effectElapsed<210) return 1397;
  if(p->effectElapsed<240) return 0;
  p->effect=0;dt=0; /* Resume interrupted note at its saved position. */
 }
 p->noteElapsed+=dt;
 while(p->noteElapsed>=duration(p->note)) {
  p->noteElapsed-=duration(p->note);
  p->note=(p->note+1U)%(sizeof(melody)/sizeof(melody[0]));
 }
 return p->noteElapsed<duration(p->note)*9U/10U?melody[p->note].hz:0;
}
