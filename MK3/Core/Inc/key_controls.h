#ifndef KEY_CONTROLS_H
#define KEY_CONTROLS_H
#include <stdint.h>
typedef struct {
 uint16_t raw, stable;
 uint32_t changed[12], repeated[12];
 uint8_t repeating[12];
} KeyControls;
void KeyControls_Init(KeyControls *s);
/* Up to 12 independent edge/repeat events. Bit row*3+column. */
unsigned KeyControls_Update(KeyControls *s,uint16_t mask,uint32_t now,char events[12]);
int KeyControls_Ambiguous(uint16_t mask);
#endif
