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

//copied from arduino tv-out
static inline void wait_until(unsigned char time) {
	
	__asm__ __volatile__ (
	"sub	%[time], %[tcnt1l]\n\t"
	"subi	%[time], 10\n"
	"100:\n\t"
	"subi	%[time], 3\n\t"
	"brcc	100b\n\t"
	"subi	%[time], 0-3\n\t"
	"breq	101f\n\t"
	"dec	%[time]\n\t"
	"breq	102f\n\t"
	"rjmp	102f\n"
	"101:\n\t"
	"nop\n"
	"102:\n"
	:
	: [time] "a" (time),
	[tcnt1l] "a" (TCNT1L)
	);
}


 unsigned char hires=0;

#define PX1(cnt)  __asm__ __volatile__ (\
"subi %[j], -1 \n" \
"out 0x5, %[j] \n" \
: : [j] "a" (cnt));

#define PX9(cnt)  __asm__ __volatile__ (\
"subi %[j], -9\n" \
"out 0x5, %[j] \n" \
: : [j] "a" (cnt));



unsigned char active_video=1;  //set to zero to blank screen to steal cpu (but keep vsync)
unsigned char lscroll=0;   //current left-right scroll 
unsigned char oldclock=1;  //last state of ps2 clock
unsigned char vsyncstate=0;  //what the next lines vsyncstate will be
unsigned char vga_a16=0;  //which bank to display onto the screen
unsigned int rowcnt=0;  //current row (0 to 240 are active 
//unsigned char activecount=0;
//unsigned char blankcount=0;

unsigned char dispeven=1;  //display odd rows


volatile unsigned char framecount=0;

ISR (TIMER1_COMPA_vect)
{
	unsigned char i;
	unsigned char laddr;
	unsigned char lddr;
	unsigned char ldata;
	unsigned char lctrl;
		
	asm volatile ("revideo:");
	wait_until(62);
	
	#ifdef VGA_KEY_INTERRUPT
	kp = KEY_PIN;  //capture ps/2 port at regular interval
	#endif
	
	//front porch is just function startup code
	
	//go directly into sync
	
	//vsync is happening after sync pulse
	VGA_PORT = (VGA_PORT & (~VGA_VSYNC_MASK)) | vsyncstate; //maybe maybe vsync low
	
	
	//BACK PORCH  burn 32 clocks.  add nops until bpend = syncend =32
	
	
	laddr = RAMADDR_PORT;
	lddr =  RAMDATA_DDR;
	ldata = RAMDATA_PORT;
		//lctrl = RAMCTRL_PORT & (~RAMCTRL_MASK);  //grab our control bits
	lctrl = RAMCTRL_PORT;
	
	//hsync up
	TCCR1A = _BV(COM1A0)|_BV(COM1A1); //set on compare match
	TCCR1C = _BV(FOC1A); //force match (so its set high)
	TCCR1A = _BV(COM1A1);   //clear on our next match
	//here about 70 clocks
	
	i = lscroll;
		
	if (  ( dispeven| (rowcnt&1)  )  &&(rowcnt < 480)        &&active_video)
	{
	
		RAMCTRL_PORT |= RAMCTRL_WE_MASK; //no writing
		RAMCTRL_PORT |= RAMCTRL_OE_MASK; //no reading
		
		//settle ram control signals
		asm volatile("nop\n nop\n ");
		
		RAMDATA_DDR = 0x00;  //input
		RAMDATA_PORT = 0x00;  //floating (no pullup)
		
		RAMCTRL_PORT = RAMCTRL_PORT &(~RAMCTRL_OE_MASK);  //ok to read

		SELECT_RAM_PAGE_NOSAVE_MACRO(rowcnt>>1);
		
		//pixel 0
		if (hires){
			char la16=PORTA;
			
			SELECT_RAM_BANK(0);
			
			//clear portb.3 operation
			TCCR0A = (1<<COM0A1)|(1<<WGM01); //clear on force
			//TCCR0A = (1<<COM0A1)|(1<<COM0A0)|(1<<WGM01); //set on force
			TCCR0B |= (1<<FOC0A);  //force
			
			//set up for toggle of portb.3 every clock
			TCCR0A = (1<<COM0A0)|(1<<WGM01); //toggle OC0A on match,ctc
			TCNT0=0xff; //roll to zero
						
			SETADDRESSLOW(i);
			VGA_DAC_PORT &= (~VGA_DAC_MASK);  //low the dac mask to enable dac
			
			//64px
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			
			//64px
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			
			//64px
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			
			//64px
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);
			SELECT_RAM_BANK(1);PX9(i);
			
			//64px
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			
			//64px
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX1(i);PX9(i);
			
			PORTA=la16;
		} else
		{
			char la16 = PORTA;
			PORTA = (la16 & RAMEX_A16_LOW_MASK) | vga_a16;
			
			//lowres
			SETADDRESSLOW(i);
			VGA_DAC_PORT &= (~VGA_DAC_MASK);  //low the dac mask to enable dac
			
			//64px
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			
			
			//64px
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			
			//64px
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			
			//32 px
			SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			//SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			//	SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			//		SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);SETADDRESSLOW(++i);
			
			PORTA=la16;
		}
		//new
		TCCR0A=0; //portb timer off
		
		RAMCTRL_PORT |= RAMCTRL_OE_MASK; //disable read
		RAMDATA_DDR = 0xff;  //all output
		RAMDATA_PORT=0;//zero
		
		//end new
		
		VGA_DAC_PORT |= VGA_DAC_MASK;  //high the dac mask to disable dac
		
		//put things back how they were
		
	
		SELECT_RAM_PAGE_NOSAVE_MACRO(lastpage); // do this while burning time
		
		
		RAMADDR_PORT = laddr;
						
		RAMCTRL_PORT |= RAMCTRL_OE_MASK; //disable read
		
		//put old data back on (if we were doing that)
		RAMDATA_DDR = lddr;  
		RAMDATA_PORT = ldata;
		
		//go back to doing whatever
		//RAMCTRL_PORT = (RAMCTRL_PORT & RAMCTRL_MASK) | lctrl;
		asm volatile("nop\n nop\n"); //settle address before possible going back to writing
		RAMCTRL_PORT = lctrl;  //this also messes with some vga bits, but whatever
		
		
		//rowcnt+=oddeven;
		//oddeven^=1;			
		//activecount++;
		rowcnt++;
	}
	else /*inactive video line */
	{
		rowcnt++;
		
		if (rowcnt==490 ) 
			vsyncstate = 0;  //vsync low
		else if (rowcnt==492)
			vsyncstate = VGA_VSYNC_MASK;
		
		//reset
		if (rowcnt==525) {
			rowcnt=0;		
			framecount++;	
		}
		
	}
		
	#ifdef VGA_KEY_INTERRUPT
	//look at keyboard before we go back
	
	i = kp & KEY_CLK;
	
	if ((!i) && (i != oldclock))
	{
		oldclock=i;
		//		TIMSK1=0; //disable tcnt interrupt
		
		keypoll(kp);  //mess with keyboard
				
		while (TCNT1>50);//wait for counter to roll over
		TIFR1 |= (1 << OCF1A); //clear flag  (setting will clear it)
		asm volatile ("rjmp revideo");
		
		//goto revid;  //will wait until the top of the next video interrupt
	}
	
	TIMSK1 |=_BV(OCIE1A);  //enable tcnt1 interrupt
	
	oldclock=i;
	#endif
	
}




void vga_init()
{
	int i;
	unsigned char j;
	
	
	TIMSK1=_BV(OCIE1A);
	
	TCCR1B = (1<<CS10)  | (1<<WGM12 );
	TCNT1 = 0x00; //zero timer count
	
	OCR1A = 636;  //Every 636 cycles do video interrupt
	
	VGA_DDR |= VGA_DDR_MASK;   //enable vga outputs
	VGA_PORT = (VGA_PORT & VGA_MASK) | VGA_HSYNC_MASK | VGA_VSYNC_MASK ;
	
	VGA_DAC_DDR |= VGA_DAC_MASK; //enable  1 output
	VGA_DAC_PORT |= VGA_DAC_MASK; //turn that bit on
	
	
	TCCR1A = (1<<COM1A1);  //clear OC1A, on match, which is hsync
	
	/* Try OC0 */
	
	TCCR0B = (1<<CS00); //unscaled clock
	OCR0A = 0;//compare to zero
	
	/*
		
	for (i=0;i<255;i++) {
		
		j =  pgm_read_byte( font+i);	
		PORTA = j;
	}
		*/
		
	
}

/* controls whether to display odd scanlines  */
void vga_fast()
{
	dispeven = 0;
}

void vga_slow()
{
	dispeven = 1;	
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


void vga_delay(int frames)
{
	char start = framecount;
	char end = frames;
	
	while (1) {
		if (frames < 256) {
			end = frames;
			while ( (framecount - start) < end);
			return;		
		}
		else
		{
			while(start == framecount);	//wait until frames advances past framecount
			while(start != framecount); //wait until frames is back to where we started
			frames -= 256;  //remove 256 frames from wait		
		}
	}
}

