/*
 * vga.h
 *
 * Created: 7/15/2014 7:39:10 PM
 *  Author: mark
 */ 


#ifndef VGA_H_
#define VGA_H_

#include "avrstuff.h"
#include "vgadefs.h"


#define VGA_DISPLAY(n) if(n) vga_a16 = RAMEX_A16_HIGH_MASK; else vga_a16 =0;

void vga_init();

extern  unsigned char vga_a16;
extern unsigned char hires;



void vga_fast();
void vga_slow();
void vga_mode(char x);

#define VGA_256 0
#define VGA_512 1


//fast drawing macros

#define VGA_BEGIN_DRAW   START_FAST_WRITE
#define VGA_END_DRAW	 END_FAST_WRITE

#define VGA_DOT(vx,vy,vc)   SELECT_RAM_PAGE(vy); FAST_WRITE(vx, vc);

#define BLACK 0
#define WHITE 0b111111
#define RED   0b000011
#define GREEN 0b001100
#define BLUE  0b110000
#define YELLOW  (GREEN|RED)
#define CYAN    (GREEN|BLUE)
#define VIOLET  (RED|BLUE)


/* Some functions accept transparent as a 'color' */
#define TRANSPARENT 0xff
void clearscreen(char color);

void drawchar(unsigned char x, unsigned char y, unsigned char c, unsigned char color, unsigned char colorback);


void drawsprite(unsigned char x, unsigned char y, unsigned char* sprite, unsigned char width, unsigned char height);



void vga_delay(int frames);


#endif /* VGA_H_ */