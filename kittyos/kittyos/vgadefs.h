/*
 * vgadefs.h
 *
 * Created: 9/4/2018 8:41:06 PM
 *  Author: mark
 */ 


#ifndef VGADEFS_H_
#define VGADEFS_H_


#define VGA_DDR			DDRD
#define VGA_DDR_MASK	0b00110100 /*three outputs*/
#define VGA_PORT		PORTD

#define VGA_MASK        0b11001011  /*all other bits*/
#define VGA_VSYNC_MASK  0b00100000  /* d.5*/
#define VGA_HSYNC_MASK  0b00010000  /*d.4*/

#define VGA_DAC_PORT	PORTA
#define VGA_DAC_DDR		DDRA
#define VGA_DAC_MASK    0b00010000  /*a.4*/
#define VGA_DAC_BITNUM	4

//old
#define VGA_HSYNC_HIGH			76
#define VGA_HSYNC_LOW		(636-20)


#define BACKPORCH	36
#define HSYNC		76
#define HSYNCGOESHIGH	54
#define HSYNCGOESLOW	636 - HSYNC + HSYNCGOESHIGH

//0 AKA 636 is when interrupt fires 
//  HSYNCGOES HIGH is when HSYNC IS DONE, SO   HSYNCGOESHIGH - 76 + 636 is when HSYNC GOES LOW





#endif /* VGADEFS_H_ */