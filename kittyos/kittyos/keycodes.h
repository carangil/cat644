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



#define K_UP	30
#define K_DOWN	31
#define K_LEFT	17
#define K_RIGHT 16

#define K_BACKSPACE 9
#define K_ESC	27

#endif /* KEYCODES_H_ */