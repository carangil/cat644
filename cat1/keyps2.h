/*
 * keyps2.h
 *
 * Created: 7/14/2014 8:46:36 PM
 *  Author: mark
 */ 


#ifndef KEYPS2_H_
#define KEYPS2_H_

#include "keycodes.h"
#include "avrstuff.h"

#define KEY_PORT  PORTA
#define KEY_DDR   DDRA
#define KEY_PIN   PINA
#define KEY_MASK  0b00111111
#define KEY_CLK   0b10000000
#define KEY_DATA  0b01000000

//#define KEY_INTERRUPT 
void key_init();

unsigned char key_kbhit();
unsigned char key_getc();
unsigned char key_ioctl(unsigned char num, void* v);


#define KEY_IOCTL_INIT 10
#define KEY_IOCTL_INTERRUPT_ENABLE 11
#define KEY_IOCTL_INTERRUPT_DISABLE 12



#define KEYB_SIZE 10
extern volatile char keyb_readpos;  //written by reader
extern volatile char keyb_writepos; //written by driver
extern volatile char keyb[];
extern volatile char keybits;
extern volatile char keybitcnt;

#endif /* KEYPS2_H_ */