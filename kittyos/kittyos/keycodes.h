/*
 * keycodes.h
 *
 * Created: 4/29/2014 8:45:43 PM
 *  Author: mark
 */ 


#ifndef KEYCODES_H_
#define KEYCODES_H_

extern unsigned const char ktab[] PROGMEM;
extern unsigned const char ktab_shift[] PROGMEM ;
extern unsigned const char ktab_eo[] PROGMEM;

#define K_RELEASE 0xF0

#define K_SHIFT	1



#define K_UP	0x18
#define K_DOWN	0x19
#define K_LEFT	0x1b
#define K_RIGHT 0x1a
#define K_BACKSPACE 8
#define K_ESC	27

#endif /* KEYCODES_H_ */