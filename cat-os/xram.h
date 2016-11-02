/*
 * xram.h
 *
 * Created: 7/14/2014 11:02:08 PM
 *  Author: mark
 */ 


#ifndef XRAM_H_
#define XRAM_H_

#include "avrstuff.h"

#define XRAM_128K

#define RAMCTRL_PORT  PORTD
#define RAMCTRL_DDR   DDRD
#define RAMCTRL_PIN   PIND
#define RAMCTRL_DDR_MASK 0b01001100  /*three outputs*/
#define RAMCTRL_MASK     0b10110010  /*all bits not used for ram control*/
#define RAMCTRL_WE_MASK  0b01000000  /*d.6*/
#define RAMCTRL_OE_MASK  0b00000100  /*d.2*/
#define RAMCTRL_PL_MASK  0b00001000  /*d.3*/

//address bus signals
#define RAMADDR_PORT PORTB
#define RAMADDR_DDR  DDRB

#define RAMEX_PORT				PORTA
#define RAMEX_DDR				DDRA
#define RAMEX_DDR_MASK			0b00100000  /* 1 output */
#define RAMEX_A16_HIGH_MASK		0b00100000  /* A16 bit */
#define RAMEX_A16_LOW_MASK		0b11011111  /* A16 bit */

//data bus signals
#define RAMDATA_PORT PORTC
#define RAMDATA_DDR  DDRC
#define RAMDATA_PIN  PINC

//let low byte of address
#define SETADDRESSLOW(addr)  RAMADDR_PORT = addr;







//select high byte of address
#define SELECT_RAM_PAGE(zzzp) \
	lastpage=(zzzp);\
	RAMCTRL_PORT |= RAMCTRL_PL_MASK;   \
	SETADDRESSLOW((zzzp)); \
	asm volatile("nop"); \
	asm volatile("nop");  \
	RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_PL_MASK);




//this version doesn't save the selected address (for use in interrupts)
#define SLOW_SELECT_RAM_PAGE_NOSAVE_MACRO(zzzp) \
	SETADDRESSLOW((zzzp)); \
	RAMCTRL_PORT |= RAMCTRL_PL_MASK;   \
	asm volatile("nop"); \
	asm volatile("nop");  \
	asm volatile("nop");  \
	RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_PL_MASK);

#define SELECT_RAM_PAGE_NOSAVE_MACRO(zzzp) \
	SETADDRESSLOW((zzzp)); \
	RAMCTRL_PORT |= RAMCTRL_PL_MASK;    /*latch output*/ \
	RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_PL_MASK);

/*
#define START_FAST_WRITE  \
	RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK | RAMCTRL_OE_MASK ; \
	RAMDATA_DDR = 0xff;
	*/

/*start fast write by disabling reading (read port high) */

#define START_FAST_WRITE  \
RAMCTRL_PORT  |= RAMCTRL_OE_MASK ; \
RAMDATA_DDR = 0xff;
	



//asm volatile("nop"); 


#define FAST_WRITE(zlowaddr,zbyte) \
RAMADDR_PORT = (zlowaddr); \
RAMDATA_PORT = (zbyte); \
RAMCTRL_PORT &= ~RAMCTRL_WE_MASK;\
RAMCTRL_PORT |= RAMCTRL_WE_MASK;



	
//return ram to reading mode
/*
#define END_FAST_WRITE  \
	RAMDATA_DDR = 0; \
	RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK  ;
*/
	

/*end fast write by enabling reading (read pin low) */
#define END_FAST_WRITE  \
RAMDATA_DDR = 0; \
RAMCTRL_PORT &= ~RAMCTRL_OE_MASK  ; \

/*
	asm volatile("nop"); \
	asm volatile("nop"); \
	asm volatile("nop"); \
	asm volatile("nop"); \
*/

#define FAST_READ(dest,zlowaddr) \
	RAMADDR_PORT = (zlowaddr); \
	asm volatile("nop"); \
	asm volatile("nop"); \
	dest = RAMDATA_PIN;
	

/* This allows external ram past 64k */

#ifdef XRAM_128K
	#define SETA16LOW  RAMEX_PORT &= RAMEX_A16_LOW_MASK
	#define SETA16HIGH RAMEX_PORT |= RAMEX_A16_HIGH_MASK
	#define SELECT_RAM_BANK(n) (n ?  (SETA16HIGH) : (SETA16LOW) )
#endif

void memcpyx2i( char* internal, unsigned int addr,  unsigned int cnt) ;

void memcpyi2x(unsigned int addr, char* internal, unsigned int cnt);

extern volatile unsigned char lastpage;
void xram_init();

void xram_test();

#endif /* XRAM_H_ */