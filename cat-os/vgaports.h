/*
 * vgaports.h
 *
 * Created: 11/23/2016 8:56:14 PM
 *  Author: mark
 */ 


#ifndef VGAPORTS_H_
#define VGAPORTS_H_


#define VGA_DDR			DDRD
#define VGA_DDR_MASK	0b00110100 /*three outputs*/
#define VGA_PORT		PORTD

#define VGA_MASK        0b11001011  /*all other bits*/
#define VGA_HSYNC_MASK  0b00100000  /* d.5*/
#define VGA_VSYNC_MASK  0b00010000  /*d.4*/

#define VGA_DAC_PORT	PORTA
#define VGA_DAC_DDR		DDRA
#define VGA_DAC_MASK    0b00010000  /*a.4*/


#endif /* VGAPORTS_H_ */