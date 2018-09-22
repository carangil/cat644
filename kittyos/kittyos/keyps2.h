/*
 * keyps2.h
 *
 * Created: 7/14/2014 8:46:36 PM
 *  Author: mark
 */ 


#ifndef KEYPS2_H_
#define KEYPS2_H_



#define KEY_PORT  PORTA
#define KEY_DDR   DDRA
#define KEY_PIN   PINA
#define KEY_MASK  0b00111111
#define KEY_CLK   0b10000000
#define KEY_DATA  0b01000000
#define KEY_DATA_POS 6
#define KEY_CLK_POS 7

#ifndef ASM

extern chardevice_t dev_keyraw;
extern chardevice_t dev_keychar;


#endif

#endif /* KEYPS2_H_ */