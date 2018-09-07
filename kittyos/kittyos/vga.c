/*
 * vga.c
 *
 * Created: 7/15/2014 7:41:34 PM
 *  Author: mark
 */ 

#include "xram.h"
#include "vga.h"
#include "font.h"

//#define VGA_KEY_INTERRUPT

#ifdef VGA_KEY_INTERRUPT

	#include "keyps2.h"
	unsigned char kp=0;


	/* This is basically a copy of the ps2 ISR.  It gets run as a 'tag along' with video*/
	

	void inline  keypoll(unsigned char portin) {
	
		char inbit = (portin & KEY_DATA)<<1;
		
		
		
		
		//this was a falling edge
		
		if (keybitcnt==0)
		{
			if (inbit==0)
			keybitcnt++;
		}
		else if (keybitcnt<9)
		{
			keybits = keybits>>1;
			keybits |= inbit;
			keybitcnt++;
		}
		else if (keybitcnt==9)
		keybitcnt++;
		else { //cnt is 10 (11th bit of frame)
			if (inbit==128)
			{
				keyb[keyb_writepos] = keybits;  //write in buffer
				if (keyb_writepos==(KEYB_SIZE-1))
				keyb_writepos=0;
				else
				keyb_writepos++;	
			}	
			keybitcnt=0;
		}	
	
	}

#endif


 unsigned char hires=0;

//goes to video interrupt
void * vidptr;
extern void vidfull();
extern void vidskippy();

volatile unsigned char skiplines=0;
void vga_init()
{



	vidptr = vidskippy;

	
	//TIMSK1=_BV(OCIE1A);
	//TIMSK1=_BV(TOIE1);  //get on overflow
	TIMSK1=_BV(ICIE1);  //get on match to ICR
	//TCCR1B = (1<<CS10)  | (1<<WGM12 );
	TCNT1 = 0x00; //zero timer count
	
	
	TCCR1B = (1<<CS10)  | (1<<WGM13 ) | (1<<WGM12 ); //unscaled clock, CTC with ICR as top 
		ICR1 = 636; //overflow every 636
		OCR1B = 636-76+40;  //trip vsync here
		//OCR1B = 636;  //trip hsync here
		
//	OCR1A = 636;  //Every 636 cycles do video interrupt
	
	VGA_DDR |= VGA_DDR_MASK;   //enable vga outputs
	VGA_PORT = (VGA_PORT & VGA_MASK) | VGA_HSYNC_MASK | VGA_VSYNC_MASK ;  //HSYNC and VSYNC are kept high
	
	VGA_DAC_DDR |= VGA_DAC_MASK; //enable  1 output
	VGA_DAC_PORT |= VGA_DAC_MASK; //turn that bit on
	
	

	//1B is PD4       (hsync)
	//1A  is PD5	(vsync)
	
	TCCR1A =  (1<<COM1A1) | (1<<COM1A0)    //set channel A high on timer1 match (OCR1A)
			| (1<<COM1B1) | (1<<COM1B0) ;   //set channel A high on timer1 match (OCR1B)  
	
	TCCR1C = (1<<FOC1A) | (1<<FOC1B)  ;// do timer match now   Both high now.
	
	asm volatile ("nop");
	
	//use channel B to drive hync low when timer goes off
	
	//TCCR1A = (1<<COM1B1);  //clear OC1B, on match, which is hsync  // also , since COM1Ax are not set anymore, this pin goes to 'normal' which is set to high
	
	TCCR1A = (1<<COM1B0);  //toggle OC1B on match 
	
	
	OCR1B = HSYNCGOESLOW;
	OCR1A = HSYNCGOESLOW;  //when sync low, do same here
	
}

/* controls whether to display odd scanlines  */
void vga_fast()
{
	vidptr = vidskippy;
}

void vga_slow()
{
	vidptr = vidfull;
}


void vga_mode(char x){
	
	if (x==VGA_512)	
		hires=1;
		
	else if (x==VGA_256)
		hires=0;
	
}

//Graphics commands for VGA_256

void clearscreen(char color)
{
	
	unsigned char i;
	unsigned char j;
		
	START_FAST_WRITE;
	for(j=0;j<240;j++) {
		SELECT_RAM_PAGE(j);
		for (i=0;;i++) {
			FAST_WRITE( i, color );
			if (i==255)
				break;
		}
		
	}
	
	END_FAST_WRITE;
}

void drawchar(unsigned char x, unsigned char y, unsigned char c, unsigned char color, unsigned char colorback)
{
	unsigned int i;
	unsigned char j;
	unsigned char k;
	unsigned char code;
	
	i = ((unsigned int)c)*8;
	
	START_FAST_WRITE;
	
	for (j=0;j<8;j++)
	{
		SELECT_RAM_PAGE((y+j));
		
		code = pgm_read_byte( font + i );
		
		for (k=0;k<8;k++)
		{
			if (code&128)
			{
				FAST_WRITE( k+x, color );
			}
					
			else if (colorback != TRANSPARENT)
			{
				FAST_WRITE( k+x, colorback );	
			}
	
				
			code = code << 1;
		}
		i++;
	}
	
	END_FAST_WRITE;
}

void drawsprite(unsigned char x, unsigned char y, unsigned char* sprite, unsigned char width, unsigned char height)
{
	unsigned char a;
	unsigned char ystop = height+y;

	
	START_FAST_WRITE;
	
	while (y != ystop){
		
		SELECT_RAM_PAGE(y);		
		
		for (a=0;a!=width;a++) {
				FAST_WRITE(a+x, *(sprite++));
		}
		
		y++;
	}
	
	END_FAST_WRITE;
}

//extern volatile unsigned char framecount;
 volatile unsigned char vscroll = 0;
volatile unsigned char hscroll = 0;
 volatile unsigned char vcnt=220;

//these not to be used outside vga
unsigned char drawrow=0;  //row to draw
unsigned char evenodd;

volatile unsigned char framecount;

void vga_delay(unsigned char frames)
{
	char start = framecount;
	char end = start+frames;
	
	if (frames==0)
		return;
	
	while (framecount != end);
	
	
}

