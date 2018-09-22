/*
 * vga.c
 *
 * Created: 7/15/2014 7:41:34 PM
 *  Author: mark
 */ 

#include "xram.h"
#include "vga.h"
#include "font.h"
#include "drivers.h"




//goes to video interrupt
void * vidptr;
extern void vidfull();
extern void vidskippy();

void vga_init()
{




	//vidptr = vidfull;
	vidptr = vidskippy;

	
	
	TIMSK1=_BV(ICIE1);  //get on match to ICR
	TCNT1 = 0x00; //zero timer count
	
	
	TCCR1B = (1<<CS10)  | (1<<WGM13 ) | (1<<WGM12 ); //unscaled clock, CTC with ICR as top 
	ICR1 = 636; //reset every 636


	VGA_DDR |= VGA_DDR_MASK;   //enable vga outputs
	VGA_PORT = (VGA_PORT & VGA_MASK) | VGA_HSYNC_MASK | VGA_VSYNC_MASK ;  //HSYNC and VSYNC are kept high
	
	VGA_DAC_DDR |= VGA_DAC_MASK; //enable  1 output
	VGA_DAC_PORT |= VGA_DAC_MASK; //turn that bit on
	
	//1B is PD4       (hsync)
	//1A  is PD5	(vsync)
	
	TCCR1A =  (1<<COM1A1) | (1<<COM1A0)    //set channel A high on timer1 match (OCR1A)
			| (1<<COM1B1) | (1<<COM1B0) ;   //set channel A high on timer1 match (OCR1B)  
	
	TCCR1C = (1<<FOC1A) | (1<<FOC1B)  ;// do timer match now   Both high now.
	
	
	//use channel B to drive hync low when timer goes off
	
	TCCR1A = (1<<COM1B0);  //toggle OC1B on match ; timer event can do both sides of hsync
	
	
	OCR1B = HSYNCGOESLOW;
	OCR1A = HSYNCGOESLOW;  //when sync low, do same here


}

/* controls whether to display odd scanlines  */
void vga_fast()

{
	cli();
	vidptr = vidskippy;
	sei();
}

void vga_slow()
{
	cli();
	vidptr = vidfull;
	sei();
}

void clearscreen(char color)
{
	
	unsigned char i;
	unsigned char j;
	
	START_FAST_WRITE;
	for(j=0;j<240;j++) {
		SELECT_RAM_PAGE(j);
		
		for (i=0;i++;) {
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
volatile unsigned char vcnt=240;

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

//virtual console for text
#define MAXX  (256-8)
#define MAXY  (240)

static uchar cx=0;
static uchar cy=0;
#define FONT_HEIGHT 8
#define FONT_WIDTH	8

void vga_putc(chardevice_t* dev, unsigned char c)
{

	if (c=='\n'){
		drawchar(cx, cy, '$', WHITE, BLACK);
		cx=0;
		cy+= FONT_HEIGHT;
	} else {
		drawchar(cx, cy, c, WHITE, BLACK);
		cx+=7;  //try slender font
	}
	
	if (cx >= MAXX){
		cx=0;
		cy+=FONT_HEIGHT;
	}
	
	if (  ((unsigned char)(cy- vscroll)   )>= (unsigned char)MAXY){
		int j;		
		vscroll +=8; //scroll smoothly (instead of 8)
		for (j=cx;j<MAXX;j++)
			drawchar(j, cy, ' ', WHITE, BLACK);
	}
	drawchar(cx, cy, '_', WHITE, BLACK);
}

chardevice_t dev_scr = {{NULL, 0}, vga_putc, NULL, NULL };
	
