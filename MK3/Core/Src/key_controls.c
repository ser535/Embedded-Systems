#include "key_controls.h"
#include <string.h>
static const char keys[]="123456789*0#";
void KeyControls_Init(KeyControls *s) { memset(s,0,sizeof(*s)); }
int KeyControls_Ambiguous(uint16_t mask)
{
 /* Two rows sharing >=2 columns form a rectangle: ghosting is possible.
    Reject that whole sample. Any two physical keys remain distinguishable. */
 for(unsigned a=0;a<4;a++) for(unsigned b=a+1;b<4;b++) {
  unsigned common=((mask>>(a*3)) & (mask>>(b*3))) & 7U;
  if(common && (common & (common-1U))) return 1;
 }
 return 0;
}
unsigned KeyControls_Update(KeyControls *s,uint16_t mask,uint32_t now,char events[12])
{
 unsigned n=0;
 if(KeyControls_Ambiguous(mask)) mask=0;
 for(unsigned i=0;i<12;i++) {
  uint16_t bit=(uint16_t)(1U<<i);
  if((mask^s->raw)&bit) { s->raw^=bit; s->changed[i]=now; }
  if(((s->raw^s->stable)&bit) && (uint32_t)(now-s->changed[i])>=30U) {
   s->stable^=bit; s->repeating[i]=0; s->repeated[i]=now;
   if(s->stable & bit) events[n++]=keys[i];
  } else if((s->stable & s->raw & bit) && (keys[i]=='4'||keys[i]=='6'||keys[i]=='8')) {
   uint32_t delay=s->repeating[i]?(keys[i]=='8'?50U:80U):250U;
   if((uint32_t)(now-s->repeated[i])>=delay) {
    events[n++]=keys[i];s->repeated[i]=now;s->repeating[i]=1;
   }
  }
 }
 return n;
}
