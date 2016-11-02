/*
 * cat1.c
 *
 * Created: 6/1/2013 5:41:14 AM
 *  Author: Mark Sherman
 */ 

#define F_CPU 20000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "keycodes.h"


//Serial port

void comm_init(uint16_t baud)
{
	//baud rate calculation (based on code from http://www.embedds.com/programming-avr-usart-with-avr-gcc-part-1/)
	UBRR0 = (uint16_t)(((F_CPU / (baud * 16UL))) - 1) ;
	
	// Set frame format to 8 data bits
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
		 
	//start
	UCSR0B|= _BV(RXEN0) | _BV(TXEN0);
}


void comm_init_115200()
{
	//baud rate calculation (based on code from http://www.embedds.com/programming-avr-usart-with-avr-gcc-part-1/)
	UBRR0 = 0xA ;  //assume 20 mhz
	
	// Set frame format to 8 data bits
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	
	//start
	UCSR0B|= _BV(RXEN0) | _BV(TXEN0);
}

#define COMM_SEND_READY  ( UCSR0A& _BV(UDRE0) )

void comm_putc(unsigned char c)
{
	//wait for it
	while(!COMM_SEND_READY);
	
	//write to output register
	UDR0 = c;
}


#define COMM_READ_READY ( UCSR0A & _BV(RXC0) )

unsigned char comm_getc()
{
	
	while(!COMM_READ_READY);
	
	return UDR0;
}

//takes a reader/writer function function
//writer function null for no echo

void readline(unsigned char (*reader)() , void (*writer)(unsigned char), char* str, uint8_t siz)
{
	uint8_t c=0;
	uint16_t cnt = 0;
		
	
	while ((cnt < siz))
	{
		c = reader();
		
		
		if(writer)
			writer(c); //echo it
			
		if (c=='\n')
			break; //done
		
		*(str++) = c;	
		cnt++;
	}
	
	*str = 0; //null terminate
}

void prints(void (*writer)(unsigned char), char* str)
{
	while(*str)
	{
		writer(*str);
		str++;
	}
	
}



//external ram

//ram control (write/read)  signals
#define RAMCTRL_PORT  PORTD
#define RAMCTRL_DDR   DDRD
#if 0
#define RAMCTRL_DDR_MASK 0b11001000  /*three outputs*/

#define RAMCTRL_MASK     0b00110111   /*all bits not used for ram control*/
#define RAMCTRL_WE_MASK  0b01000000    
#define RAMCTRL_OE_MASK  0b10000000
#define RAMCTRL_PL_MASK  0b00001000
#endif

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

//select different pages of 256 bytes each
// (go through funcion so we can switch to using multiplexing high address later if we want



void initram()
{
	//no write, no read
	RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK | RAMCTRL_OE_MASK;
		
	//set data bus direction
	RAMDATA_DDR = 0x0;  //all input
	RAMDATA_PORT = 0x0; //no pullup
	
	//write enable up, output enable down (RAM output is enabled)
	
	RAMCTRL_DDR =  RAMCTRL_DDR_MASK;
	
	//set to read mode  (write enable high, output enable low)
	RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK ;

	//set address port direction
	RAMADDR_DDR = 0xFF;  //all output
}


//set address on port (low part of address)

#define SETADDRESSLOW(addr)  RAMADDR_PORT = addr;

//select which 'page'
//the high part of the address is set and strobed
//after this the low part of the address will have to be set again later

volatile unsigned char lastpage=0;




#define SELECT_RAM_PAGE(zzzp) \
  lastpage=(zzzp); \
  SETADDRESSLOW((zzzp)); \
  RAMCTRL_PORT |= RAMCTRL_PL_MASK;    /*latch output*/ \
  asm volatile("nop"); \
  asm volatile("nop");  \
   asm volatile("nop");  \
  RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_PL_MASK); 
 

  
 //this version doesn't save the selected address (for use in interrupts)
 
#define SLOW_SELECT_RAM_PAGE_NOSAVE_MACRO(zzzp) \
  SETADDRESSLOW((zzzp)); \
  RAMCTRL_PORT |= RAMCTRL_PL_MASK;    /*latch output*/ \
  asm volatile("nop"); \
  asm volatile("nop");  \
   asm volatile("nop");  \
  RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_PL_MASK); 

#define SELECT_RAM_PAGE_NOSAVE_MACRO(zzzp) \
SETADDRESSLOW((zzzp)); \
RAMCTRL_PORT |= RAMCTRL_PL_MASK;    /*latch output*/ \
RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_PL_MASK);


//write one byte
//strobes high part of address
//also returns ram back to 'reading' mode

void slowwrite(unsigned int address, unsigned char value)
{
	//return;
	
	
		
	SELECT_RAM_PAGE(address >> 8);
	//select_ram_page(address>>8);
	
	SETADDRESSLOW(address);
	//no output
	//settle address
	asm volatile("nop");   //delay 1 cycle (50 ns)
	asm volatile("nop");   //delay 1 cycle (50 ns)
	
	RAMCTRL_PORT   |= RAMCTRL_OE_MASK; //disable OE
	
	//write enable

	//write!
	
	//cli();
	
	RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_WE_MASK);   //write enable down
	
	RAMDATA_PORT = value;  //output the value
	RAMDATA_DDR = 0xff; //all output
	
	asm volatile("nop");   //delay 1 cycle (50 ns)
	asm volatile("nop");   //delay 1 cycle (50 ns)
	asm volatile("nop");   //delay 1 cycle (50 ns)
	
	//no input/output
	RAMCTRL_PORT   |= RAMCTRL_WE_MASK;  //write enable up (stop writing)
		
	//sei();
	
	RAMDATA_DDR = 0x00;	 //all input
	RAMDATA_PORT = 0;    //no pullup
	
	//back to default reading mode
	RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_OE_MASK);
	
	
}


//begin a series of fast writes
//when writing fast, data bus is left in output state between writes
//OE is also kept high
//fast writes also only change the low address (must call set select ram page to move hi part)

#define START_FAST_WRITE  \
	 RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK | RAMCTRL_OE_MASK ; \
	 RAMDATA_DDR = 0xff;
	 
//fast write into the currently enabled page

#define FAST_WRITE(zlowaddr,zbyte) \
	RAMADDR_PORT = (zlowaddr); \
	RAMDATA_PORT = (zbyte); \
	RAMCTRL_PORT = RAMCTRL_PORT & (~RAMCTRL_WE_MASK); \
/*	asm volatile("nop"); */  \
/*  asm volatile("nop"); */  \
/*  asm volatile("nop"); */  \
	RAMCTRL_PORT |= RAMCTRL_WE_MASK;  //we up
	
//return ram to reading move
#define END_FAST_WRITE  \
	RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK  ;


//strobes hi part of address
unsigned char slowread(unsigned int address)
{
	SELECT_RAM_PAGE(address >> 8);
	
	SETADDRESSLOW(address & 0xFF);
	
	asm volatile("nop");   //delay 1 cycle (50 ns)
	asm volatile("nop");   //delay 1 cycle (50 ns)
	asm volatile("nop");   //delay 1 cycle (50 ns)

	return RAMDATA_PIN;
}

//keyboard input (ps/2)

#define KEY_PORT  PORTA
#define KEY_DDR   DDRA
#define KEY_PIN   PINA
#define KEY_MASK  0b00111111
#define KEY_CLK   0b10000000
#define KEY_DATA  0b01000000


//#define KEY_INTERRUPT 


//if enable interript is not set, we need to poll
void init_key()
{
	//KEY_PORT &= KEY_MASK; //no pullups
	
	KEY_DDR &= KEY_MASK;  //keyboard are inputs
	KEY_PORT |= KEY_CLK |KEY_DATA ; //pullup
	
	
	#if 0
		//debug only: no pullup
		KEY_PORT=0;
	#endif
		
	#ifdef KEY_INTERRUPT
	
	PCICR |= (1 << PCIE0);  //enable pin change interrupt
	PCMSK0 |= (1<<PCINT7);	 //enable PCINT7
	
	#endif
}

#define KEYB_SIZE 10
volatile char keyb_readpos=0;  //written by reader
volatile char keyb_writepos=0; //written by driver
volatile char keyb[KEYB_SIZE];
volatile char keybits=0;
volatile char keybitcnt=0;

#define SETA16LOW  RAMEX_PORT &= RAMEX_A16_LOW_MASK
#define SETA16HIGH RAMEX_PORT |= RAMEX_A16_HIGH_MASK 

#define A16(n) (n ?  (SETA16HIGH) : (SETA16LOW) )

void ramex_init()
{
	RAMEX_DDR |= RAMEX_DDR_MASK;  //enable one output
	SETA16LOW;
	
	
}



volatile char debugkeys[11];
volatile char debugkeypos=0;


unsigned char nextkey(){
	unsigned char x;
	
	if (keyb_writepos == keyb_readpos)
		return 0;
		
	x = keyb[keyb_readpos++];
	if (keyb_readpos >= KEYB_SIZE)
		keyb_readpos = 0;
	
	return x;
}


volatile unsigned char keybh=0;
ISR (PCINT0_vect)
{
	keybh=1;
	
	if (!(KEY_PIN & KEY_CLK))
	{
		char inbit = (KEY_PIN&KEY_DATA)<<1;
		
	
		debugkeys[debugkeypos]=inbit;
		debugkeypos++;
		debugkeypos = debugkeypos % 11;
		
		
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
	
}

//poll keyboard with no interrupts
char oldclock = 1;

void inline  keypoll(unsigned char portin) {
	
		char inbit = (portin & KEY_DATA)<<1;
		
		/*
		debugkeys[debugkeypos]=inbit;
		debugkeypos++;
		debugkeypos = debugkeypos % 11;
		*/
		
		
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


//try some vga

#define VGA_DDR			DDRD
#define VGA_DDR_MASK	0b00110100 /*three outputs*/
#define VGA_PORT		PORTD

#define VGA_MASK        0b11001011  /*all other bits*/
#define VGA_HSYNC_MASK  0b00100000  /* d.5*/
#define VGA_VSYNC_MASK  0b00010000  /*d.4*/

/* VGA DAC might be on different port */
//#define VGA_DAC_PORT	PORTD
//#define VGA_DAC_DDR		DDRD
//#define VGA_DAC_MASK    0b00000100  /*d.2*/

#define VGA_DAC_PORT	PORTA
#define VGA_DAC_DDR		DDRA
#define VGA_DAC_MASK    0b00010000  /*a.4*/


int mwscnt=0;
//which page to draw
volatile char vga_a16=0;

#define VGA_SCREEN(n) vga_a16= (n) << 5 ;  //if ramex a16 is a.5


volatile unsigned char fpend=0;
volatile unsigned int syncend=0;
volatile unsigned int bpend=0;
volatile unsigned int vgalen_full=0;
volatile unsigned int vgalen_half=0;

static char vsyncstate=VGA_VSYNC_MASK; 

//char bob=0;
//volatile unsigned char pageselok=1;

volatile unsigned char lscroll=0;

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


#if 1

static unsigned char hires=0;

#define PX1(cnt)  __asm__ __volatile__ (\
"subi %[j], -1 \n" \
"out 0x5, %[j] \n" \
: : [j] "a" (cnt));

#define PX9(cnt)  __asm__ __volatile__ (\
"subi %[j], -9\n" \
"out 0x5, %[j] \n" \
: : [j] "a" (cnt));

unsigned char kp=0;

unsigned char active_video=1;

ISR (TIMER1_COMPA_vect)
{
	unsigned char i;
	unsigned char laddr;
	unsigned char lddr;
	unsigned char ldata;
	unsigned char lctrl;

	//	char full;
	
	
	
	asm volatile ("revida:");
	wait_until(60);
	kp = KEY_PIN;  //capture ps/2 port
	
		

	//fpend=TCNT1L;
	
	//front porch is just function startup code
	
	//go directly into sync

	//VGA_PORT ^= VGA_HSYNC_MASK ;   //set hsync low
	//VGA_PORT = VGA_PORT& (~VGA_HSYNC_MASK) ;   //set hsync low
	
	
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); //10 cycles
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); //10 cycles
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n no p\n nop\n nop\n nop\n"); //10 cycles
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); //10 cycles
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); //10 cycles
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); //10 cycles
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); //8 cycles
	//	VGA_PORT |= VGA_HSYNC_MASK;   //set hsync high
	
	//hsync up was here
	

	
	
	//vsync is happening after sync pulse
	VGA_PORT = (VGA_PORT & (~VGA_VSYNC_MASK)) | vsyncstate; //maybe maybe vsync low
	
	//	syncend=TCNT1L;
	//BACK PORCH  burn 32 clocks.  add nops until bpend = syncend =32
	//save this anyway (whether we need it or not; we want to burn a little time here anyway
	
	//	if (pageselok)
	{
		laddr = RAMADDR_PORT;
		lddr =  RAMDATA_DDR;
		ldata = RAMDATA_PORT;
		//lctrl = RAMCTRL_PORT & (~RAMCTRL_MASK);  //grab our control bits
		lctrl = RAMCTRL_PORT;
	}
	
	
	
	//hsync up
	TCCR1A = _BV(COM1A0)|_BV(COM1A1); //set on compare match
	TCCR1C = _BV(FOC1A); //force match (so its set high)
	TCCR1A = _BV(COM1A1);   //clear on our next match
	//here about 70 clocks
	
	
	//	asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); //10 cycles
	//asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n ");
	
	//bpend = TCNT1L;
	
	//full=0;
	
	#if 0
		i = KEY_PIN & KEY_CLK;
	
		if ((!i) && (i != oldclock))
		{
			keypoll();
			oldclock=i;
			goto skipvid;
		}
		oldclock=i;
	#endif
	
	i = lscroll;
	
	//if ((mwscnt<480)  && (mwscnt & 1) )
	if ((mwscnt<480)&&active_video)
	{
		//	VGA_PORT ^= VGA_DAC_MASK;  //low the dac mask to enable dac (doing this early we we can watch time burn on the edges of the screen);
		
		
		
		//	RAMCTRL_PORT = (RAMCTRL_PORT & RAMCTRL_MASK) | RAMCTRL_WE_MASK|RAMCTRL_OE_MASK ;  //we high, oe low, we are reading
		
		RAMCTRL_PORT |= RAMCTRL_WE_MASK; //no writing
		RAMCTRL_PORT |= RAMCTRL_OE_MASK; //no reading
		
		//settle ram control signals
		asm volatile("nop\n nop\n ");
		
		RAMDATA_DDR = 0x00;  //input
		RAMDATA_PORT = 0x00;  //floating
		//RAMCTRL_PORT = (RAMCTRL_PORT & RAMCTRL_MASK) | RAMCTRL_WE_MASK;  //we high, oe low, we are reading
		
		RAMCTRL_PORT = RAMCTRL_PORT &(~RAMCTRL_OE_MASK);  //ok to read
		
		//if (pageselok)
		//{
		SELECT_RAM_PAGE_NOSAVE_MACRO(mwscnt>>1); // so this while burning time
		
		
		//	}
		
		//pixel 0
		
		if (hires){
			char la16=PORTA;
			
			A16(0);
				
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
			A16(1);PX9(i);
			
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
					
			
		}
		//new
		TCCR0A=0; //portb timer off
		
		RAMCTRL_PORT |= RAMCTRL_OE_MASK; //disable read
		RAMDATA_DDR = 0xff;  //all output
		RAMDATA_PORT=0;//zero
		
		//end new
		
		VGA_DAC_PORT |= VGA_DAC_MASK;  //high the dac mask to disable dac
		
		//put things back how they were
		
		//if (pageselok)
		//{
		SELECT_RAM_PAGE_NOSAVE_MACRO(lastpage); // do this while burning time
		//	}
		
		RAMADDR_PORT = laddr;
		
		//let address settle before fiddling with control bits
		
		RAMCTRL_PORT |= RAMCTRL_OE_MASK; //disable read
		
		//put old data back on (if we were doing that)
		RAMDATA_DDR = lddr;  //
		RAMDATA_PORT = ldata;
		
		//go back to doing whatever
		//RAMCTRL_PORT = (RAMCTRL_PORT & RAMCTRL_MASK) | lctrl;
		asm volatile("nop\n nop\n"); //settle address before possible going back to writing
		RAMCTRL_PORT = lctrl;  //this also messes with some vga bits, but whatever
		
		
		//full=1;
		mwscnt++;
		//VGA_PORT |= VGA_DAC_MASK;  //high the dac mask to disable dac  (doing this late to watch time burn on the other side )
	}
	else
	{
		//keypoll();
	//	skipvid:
		//look at next line
		mwscnt++;
		
		if (( mwscnt == 492)||(mwscnt==493))
		vsyncstate = 0;  //vsync low
		else vsyncstate = VGA_VSYNC_MASK;
	
	#if 0
	#ifdef KEY_INTERRUPT	
		if (mwscnt==494)
		{
			//enable keyboard interrupt


			KEY_DDR &= KEY_MASK;  //input
			KEY_PORT |= KEY_CLK; //pullup
			PCICR |= (1 << PCIE0);  //enable keyboard interrupt
	
			keybitcnt=0;  //reset input stream
		}
		
		if (mwscnt==524)
		{
			//disable keyboard interrupt
			PCICR &= ~(1 << PCIE0);  //disable pin change interrupt (should be off now)
			KEY_PORT &= (~KEY_CLK); //low (to inhibit transmission)
			KEY_DDR |= KEY_CLK;  //output
	
		}
	#endif
	#endif
		
		if (mwscnt==525)  // line 525  is line 0 again
		{
			mwscnt=0;
			//	lscroll --;  //scroll right 1
		}
		vgalen_half = TCNT1;
		
	}
	
	//if (full)
	//vgalen_full = TCNT1;
	
	//keypoll();
	
	
	#if 1
	//look at keyboard before we go back
	
	i = kp & KEY_CLK;
	
	if ((!i) && (i != oldclock))
	{
		oldclock=i;
//		TIMSK1=0; //disable tcnt interrupt
		keypoll(kp);
				
		
		while (TCNT1>50);//wait for counter to roll over
		TIFR1 |= (1 << OCF1A); //clear flag  (setting will clear it)
		asm volatile ("rjmp revida");
		
		//goto revid;  //will wait until the top of the next video interrupt 
	}
	
	TIMSK1 |=_BV(OCIE1A);  //enable tcnt1 interrupt
	
	oldclock=i;
	#endif
	
}

#endif

unsigned char buf[240];
void snd_init()
{
	unsigned char bpos;
	unsigned char color;
	unsigned char s;
	DDRD |= 0b10000000;
	
	OCR2A = 10;
	TCCR2A = (1<<COM2A1) | (1<<WGM21) | (1<<WGM20) ; //non-inverted
	TCCR2B = (1<<CS20);
	
	TCNT2=0xfe;
	
	bpos=0;
	while(1) {
		
		
		slowwrite( (((int)bpos)<<8)  |  buf[bpos], 0);
		
		s = comm_getc();
		
		//draw dot on screen; keep 's' in buffer so we can undraw later
		slowwrite( (((int)bpos)<<8)  |  s, color);  
		buf[bpos] = s;
		bpos++;
		if (bpos==240){
			bpos=0;
			color++;	
		}
		
		OCR2A = s;
	}
	
}


void vga_init()
{
	
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
	
#if 0
	
	TCCR0A = (1<<COM0A0)|(1<<WGM01)|(1<<COM0A1); //set on force
	TCCR0B  |= (1<<FOC0A);  //force
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	
	TCCR0A = (1<<COM0A1)|(1<<WGM01); //clear on force
	TCCR0B |= (1<<FOC0A);  //force
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	
	TCCR0A = (1<<COM0A0)|(1<<WGM01); //toggle OC0A on match,ctc
	TCNT0=0xff; //roll to zero
	
	
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
#endif
	
	/* End try OC0 */	
		
	sei();
}

unsigned char mhash(unsigned int address, char constant)
{
	char col = address & 0xff;
	char row = (address >> 8);
	
	if (col&1)
		return col +constant;
	
	return row + constant;
	
}


unsigned char cat[] =
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,
	0,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,
	0,1,0,0,1,0,0,0,0,0,0,1,0,0,1,0,
	0,1,0,0,0,1,1,1,1,1,1,0,0,0,1,0,
	0,1,0,1,1,1,0,0,0,0,1,1,1,0,1,0,
	0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,
	0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,
	0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,
	0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,
	0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
	0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,
	0,0,1,0,0,0,0,1,1,0,0,0,0,1,0,0,
	0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};






extern unsigned char font[];

unsigned char coltab[] = {
	
	
	0x0, 0x8,
	0x1, 0x9,
	0x2, 0xA,
	0x3, 0xB,
	0x4, 0xC,
	0x5, 0xD,
	0x6, 0xE,
	0x7, 0xF,
	0x10, 0x18,
	0x11, 0x19,
	0x12, 0x1A,
	0x13, 0x1B,
	0x14, 0x1C,
	0x15, 0x1D,
	0x16, 0x1E,
	0x17, 0x1F,
	0x20, 0x28,
	0x21, 0x29,
	0x22, 0x2A,
	0x23, 0x2B,
	0x24, 0x2C,
	0x25, 0x2D,
	0x26, 0x2E,
	0x27, 0x2F,
	0x30, 0x38,
	0x31, 0x39,
	0x32, 0x3A,
	0x33, 0x3B,
	0x34, 0x3C,
	0x35, 0x3D,
	0x36, 0x3E,
	0x37, 0x3F,
	0x40, 0x48,
	0x41, 0x49,
	0x42, 0x4A,
	0x43, 0x4B,
	0x44, 0x4C,
	0x45, 0x4D,
	0x46, 0x4E,
	0x47, 0x4F,
	0x50, 0x58,
	0x51, 0x59,
	0x52, 0x5A,
	0x53, 0x5B,
	0x54, 0x5C,
	0x55, 0x5D,
	0x56, 0x5E,
	0x57, 0x5F,
	0x60, 0x68,
	0x61, 0x69,
	0x62, 0x6A,
	0x63, 0x6B,
	0x64, 0x6C,
	0x65, 0x6D,
	0x66, 0x6E,
	0x67, 0x6F,
	0x70, 0x78,
	0x71, 0x79,
	0x72, 0x7A,
	0x73, 0x7B,
	0x74, 0x7C,
	0x75, 0x7D,
	0x76, 0x7E,
	0x77, 0x7F,
	0x80, 0x88,
	0x81, 0x89,
	0x82, 0x8A,
	0x83, 0x8B,
	0x84, 0x8C,
	0x85, 0x8D,
	0x86, 0x8E,
	0x87, 0x8F,
	0x90, 0x98,
	0x91, 0x99,
	0x92, 0x9A,
	0x93, 0x9B,
	0x94, 0x9C,
	0x95, 0x9D,
	0x96, 0x9E,
	0x97, 0x9F,
	0xA0, 0xA8,
	0xA1, 0xA9,
	0xA2, 0xAA,
	0xA3, 0xAB,
	0xA4, 0xAC,
	0xA5, 0xAD,
	0xA6, 0xAE,
	0xA7, 0xAF,
	0xB0, 0xB8,
	0xB1, 0xB9,
	0xB2, 0xBA,
	0xB3, 0xBB,
	0xB4, 0xBC,
	0xB5, 0xBD,
	0xB6, 0xBE,
	0xB7, 0xBF,
	0xC0, 0xC8,
	0xC1, 0xC9,
	0xC2, 0xCA,
	0xC3, 0xCB,
	0xC4, 0xCC,
	0xC5, 0xCD,
	0xC6, 0xCE,
	0xC7, 0xCF,
	0xD0, 0xD8,
	0xD1, 0xD9,
	0xD2, 0xDA,
	0xD3, 0xDB,
	0xD4, 0xDC,
	0xD5, 0xDD,
	0xD6, 0xDE,
	0xD7, 0xDF,
	0xE0, 0xE8,
	0xE1, 0xE9,
	0xE2, 0xEA,
	0xE3, 0xEB,
	0xE4, 0xEC,
	0xE5, 0xED,
	0xE6, 0xEE,
	0xE7, 0xEF,
	0xF0, 0xF8,
	0xF1, 0xF9,
	0xF2, 0xFA,
	0xF3, 0xFB,
	0xF4, 0xFC,
	0xF5, 0xFD,
	0xF6, 0xFE,
	0xF7, 0xFF
};


void drawchar(unsigned char x, unsigned char y, unsigned char c, unsigned char color, unsigned char colorback)
{
	unsigned int i;
	unsigned char j;
	unsigned char k;
	unsigned char code;
	
	i = ((unsigned int)c)*8;
	
	for (j=0;j<8;j++) 
	{
		code = font[i];
		for (k=0;k<8;k++) 
		{
			if (code&128) 
				slowwrite( (   ((int)(y+j)) << 8) |  (k+x) , color);			
			else
				slowwrite( (   ((int)(y+j)) << 8) |  (k+x) , colorback);			
			code = code << 1;
		}
		i++;
	}		
}

void drawcharhi(unsigned char x, unsigned char y, unsigned char c, unsigned char color, unsigned char colorback)
{
	unsigned int i;
	unsigned char j;
	unsigned char k;
	unsigned char code;
	
	i = ((unsigned int)c)*8;
	
	for (j=0;j<8;j++)
	{
		code = font[i];
		for (k=0;k<8;k++)
		{
			if (code&128)
			slowwrite( (   ((int)(y+j)) << 8) | coltab[ (k+x)] , color);
			else
			slowwrite( (   ((int)(y+j)) << 8) | coltab[ (k+x)] , colorback);
			code = code << 1;
		}
		i++;
	}
}






int screen_x = 0;
int screen_y = 0;

char color=0b111111;
char color2=0;
int screen_writehi(char* string) {


	while(*string)	
	{
		
		if (*string == '\n') {
			screen_x=0;
			screen_y++;
			string++;
			continue;
		}
			
		drawcharhi(screen_x*8, screen_y*8, *string,color, color2);
		screen_x ++;
		
		
		screen_x = screen_x & 31;
		
		screen_y = screen_y & 31;
		
		
		//color2++;
		color ++;
		
//		if (!(color&7))
	//		color++;
			
		//if (color==color2)
			//color2++;
						
		string++;
	}
}


int screen_write(char* string) {


	while(*string)
	{
		
		if (*string == '\n') {
			screen_x=0;
			screen_y++;
			string++;
			continue;
		}
		
		drawchar(screen_x*8, screen_y*8, *string,color, color2);
		screen_x ++;
		
			if (screen_x > 31)
			{
				screen_y++;
				screen_x=0;
			}
		
		screen_y = screen_y & 31;
		
		
		//color2++;
		//color ++;
		
		//		if (!(color&7))
		//		color++;
		
		//if (color==color2)
		//color2++;
		
		string++;
	}
}


void screen_write_one(char c)
{
	char x[2];
	x[0]=c;
	x[1]=0;
	screen_write(x);
}

#define HOME  screen_x=screen_y=0;

char shiftstate=0;
int releasestate=0;

extern unsigned char ktab[];
extern unsigned char ktab_shift[];

char xlat(char* xtab, unsigned char code)
{
	char ch;
	char i;	
	
	for (i=0; ktab[i]; i+=2)
	{
		if (code == xtab[i+1])
		{
			return xtab[i];
		}
	}
	return 0;
}


unsigned char nextkeychar(){
	
	unsigned char ch=0;
	unsigned char chs=0;
	unsigned char code;
	
	
	
	code = nextkey();

	
	if (!code)
		return 0;

//	{
	//	char str[10];
		//sprintf(str, "{c=%x}", code);
	//	prints (comm_putc,str);
	
	//}
	
	if (code ==0xf0)	{ //release
		releasestate=1;
		return 0;		
	}
			
	ch = xlat(ktab, code);


	if (!releasestate)
	{
		
		 if (ch == K_SHIFT) 
		 {
			shiftstate=1;
			return 0;  //no char yet	 
		 }

		
			

		if (shiftstate){
		
			if ((ch>='a') && (ch <='z'))	
				chs = ch-'a'+'A';
			else
				chs = xlat(ktab_shift, ch);
		}
			
		if (chs)
			return chs;
		 
		return ch;
	}
		
	//following is for is a key was released
	
	
	if (ch==K_SHIFT)
		shiftstate=0;
	
	releasestate=0;
		
	return 0;	
}

//blocks until a key is eaten
unsigned char key_getc(){
	
	unsigned char x=0;
#if 0	
	#ifndef KEY_INTERRUPT
		//enable keyboard
		KEY_DDR &= KEY_MASK;  //input
		KEY_PORT |= KEY_CLK; //pullup
		oldclock = 1;
		keybitcnt=0; 
	#endif 
#endif
	
	while(!x)
	{
		
		x = nextkeychar();
		
	}
	
	#if 0
	#ifndef KEY_INTERRUPT
	
		//disable keyboard
		KEY_PORT &= (~KEY_CLK); //low (to inhibit transmission)
		KEY_DDR |= KEY_CLK;  //output
	#endif 
	#endif

	
	return x;
	
}

#define SDCARD_CS		0b1000
#define SDCARD_CS_PORT  PORTA
#define SDCARD_CS_DDR	DDRA
/*
void init_blink(){
	DDRA |= 0b1000	;
}

void blink(char a)
{
	
	if (a)
	PORTA|=0b1000;
	else

	PORTA&=~0b1000;
	
}
*/

#if 1





void spi_init(void)
{

	/* SCLK, MOSI, SS outputs */
	DDRB |= 0b10110000; 
	PORTB=0x00;
	
	
	/*SPRO = clk/16*/
	
	SPCR = _BV(SPE) | _BV(MSTR); /*   | _BV(SPR0) | _BV(SPR1)*/ ;
	SPSR = _BV(SPI2X);
}

uint8_t spi_inout(uint8_t val)
{
	/* Write */
	SPDR = val;
	
	//prints(comm_putc, "s");
	
	/* Wait */
	while (!(SPSR & _BV(SPIF))) 
	{
		//prints(comm_putc, "w");	
		
	}
	
	/* Read byte */
	val = SPDR;
	
	
	//prints(comm_putc, "r");
	
	return val;
}




#endif 

void spi_disable(){
	SPCR &= ~_BV(SPE);
}

void spi_enable(){
	SPCR |= _BV(SPE);
}





void sdcard_enable()
{
	//low bit
	SDCARD_CS_PORT &= ~ SDCARD_CS;
}

void sdcard_disable()
{
	//high bit
	SDCARD_CS_PORT |=  SDCARD_CS;
}

char str[20];

unsigned char sdcard_send(unsigned char cmd, uint32_t parm)
{
	unsigned char resp;
//	printf(comm_putc,"sd send\n");
	
	//sprintf(str, "send %x:", cmd);
	//prints(comm_putc, str);
		
	spi_inout(cmd);
	spi_inout((parm>>24) & 0xff);
	spi_inout((parm>>16) & 0xff);
	spi_inout((parm>>8) & 0xff);
	spi_inout(parm & 0xff);
	spi_inout(0x95);  /* checksum for cmd '40'.  all other cmds this will be ignored if we are in spi mode */
	
	while(1)
	{
		resp = spi_inout(0xff);
		/* 1st bit will go low when response is done */
			
	//	sprintf(str, " %x ",resp);
		//prints(comm_putc, str);
						
		if (! (resp & 0x80 ) )
			return resp;
			
	}
	
}


void sdcard_init()
{
	char r;
	char i;
	
	//for now, just mess with the port
	
	/*bit is high, and output*/
	
	spi_enable();
	
	SDCARD_CS_PORT |=  SDCARD_CS;
	
	SDCARD_CS_DDR |= SDCARD_CS;
	prints (comm_putc,"init1\n");
	
	for(i=0; i<10; i++) // idle for 10 bytes / 80 clocks
		spi_inout(0xFF);
	prints (comm_putc,"init2\n");
	
	sdcard_enable();
	sdcard_send(0x40, 0 );
	sdcard_disable();
	
	/* keep sending 41 until a '0' is received */
	while (1) {
		prints (comm_putc,"init try\n");
		sdcard_enable();
		r = sdcard_send(0x41, 0);
		sdcard_disable();
		if (!r)
			break;
	}
	
	/* set block size */
	sdcard_enable();
	sdcard_send(0x50, 512);
	sdcard_disable();	
	spi_disable();
}

/* buffer must hold 512 bytes */

char sdcard_read_block(long blockaddr, char* buffer) {
	char r;
	int off;
	char i;
	
	spi_enable();
	sdcard_enable();
	
	r = sdcard_send(0x51, blockaddr <<9 );
	
	
		
	while (r!= 0xFE)
	{
		r = spi_inout(0xff)		;
	}
	
	
	SPDR = 0xff;  //write byte
	i=0;
	for (;;) 
	{
	
		//wait for byte;		
		while (!(SPSR & _BV(SPIF)));
		*(buffer++) = SPDR;  //read in the byte

		SPDR = 0xff; //send for next byte	
		while (!(SPSR & _BV(SPIF))); //read in the byte
		*(buffer++) = SPDR;  
		
		SPDR = 0xff; //send for next byte
			
		
		i++;
		if (i==0)		
			break;  //went around 256 times
		
	}
	
	//read dummy checksum
	spi_inout(0xff);
	//spi_inout(0xff);
	
	sdcard_disable();
	spi_disable();
	
	if (r == 0xfe)
		return 1;
	else 
		return 0;
		
}

char buffer[512];

int main(void)
{

	char str[200];
	int i,j;
	char wcount=0;
	
	/* Disable JTAG port */
	MCUCR |= _BV(JTD);
	MCUCR |= _BV(JTD);
		
	//Initialize USART0
	//comm_init(19200);
	comm_init(56000);
	//comm_init_115200();
	prints (comm_putc,"start\n");
	
	spi_init();
	sdcard_init();


	if (sdcard_read_block(0, buffer)) {
		
		for (i=0;i<512;i++) {
			sprintf(str, "%x ", buffer[i] );
			prints(comm_putc, str);
		}
		
	}else
		prints (comm_putc,"fail\n");
	

	init_key();  //initialize keyboard, but with no interrupts

	sei();
	
	#if 0
	//cause pin change interrupt
	//only use in debugger!
//	DDRA=0xff;
	//PORTA=0;
//	PORTA=0xff;
	#endif
	
//keyloop
#if 0
	while(1){
		char i;
		unsigned char x;

		
		sprintf( str, "keybh %d : " , keybh   );
		prints (comm_putc,str) ;
				
		keybh=0;
		
			for (i= 0; i<11;i++)
			{
				sprintf( str, "%d.", debugkeys[ (debugkeypos + i) % 11] );
				prints (comm_putc,str);
			}
			prints(comm_putc,"\r\n");
		_delay_ms(100);
		
		while (1)
		{
				x=nextkeychar();
				if (!x)
					continue;
				//sprintf( str, "%x[%c]", x, x);
				
			//	prints(comm_putc, str);
				//_delay_ms(100);
		}
		
//		while ((x = nextkey()))
	//	{
			
		//	sprintf( str, "keyboard code %x\r\n", x);
			//prints (comm_putc,str);
			
	//	}
		
		
	}
#endif

	initram();
	ramex_init();
	
	vga_init();
	prints (comm_putc,"Testing comm hello ") ;
		
	
	
	VGA_SCREEN(0);
	A16(0);
	
	
	while(1)
	{
		unsigned int i;
		unsigned int j;
		prints(comm_putc, "This version is" __DATE__  " " __TIME__ "\n");
		
		
		prints(comm_putc,"writing color pattern \r\n");
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{
				char c;
				c= i>>2;
				
				if (j<32)
				c&=0b000011;
				else if (j<64)
				c&=0b001100;
				else if (j<128)
				c&=0b110000	;
				
				slowwrite((j<<8) | i, c ) ;
			}
		}
		
		HOME;
		prints(screen_write_one, "Hello type something");
		while(1) {
			
		prints(screen_write_one, "a");	
		}
		while(1) {
			
			
			
			prints(screen_write_one, ">");
			readline(key_getc, screen_write_one, str, sizeof(str)-1);
			
			if (!strncmp(str, "color ", 6))
			j = atoi(str+6);
			if (j)
				color=j;
				
			prints(screen_write_one,"\r\nYou said ");
			prints(screen_write_one,str);
			prints(screen_write_one,"\r\n");
			
			
			if (!strcmp(str, "sound"))
			{
				
				
				//black
				for (j=0;j<240;j++)
				{
					for (i=0;i<256;i++)
					{
						slowwrite((j<<8) | i,  0  ) ;
					}
				}
				
				HOME;
				prints(screen_write_one, "Sound and Video together");
				
				snd_init();
							
				
			}
			
			
			if (!strcmp(str, "cls"))
			{
				for (j=0;j<240;j++)
				{
					for (i=0;i<256;i++)
					{
						
						
						slowwrite((j<<8) | i, 0 ) ;
					}
				}
				
				HOME;
				
			}
			
			if (!strcmp(str, "test"))
			{
				prints(screen_write_one,"\r\nContinuing test program\r\n");
				break;
			}
			
			if (!strcmp(str, "read"))
			{
				
				while (1)
				{
					
				
					while ( PIND & VGA_VSYNC_MASK); //wait until vsync goes low
					active_video=0;
					
					if (sdcard_read_block(0, buffer)) {
						active_video=1;
					/*	for (i=0;i<512;i++) {
							sprintf(str, "%x ", buffer[i] );
							prints(comm_putc, str);
						}
						*/
					
					}else
					prints (comm_putc,"fail\n");
					active_video=1;
				
				}
				
			}
			
		}
		
		
		
		prints(comm_putc,"Hello type something \r\n");
		//lscroll ++;
		
	/*	for (i= 0; i<11;i++)
		{
			sprintf( str, "%d.", debugkeys[ (debugkeypos + i) % 11] );
			prints (str);
		}
		prints("\r\n");
		*/
		
		#if 0
		{
			unsigned char x;
			
			while ((x = nextkey()))	
			{
		
				sprintf( str, "keyboard code %x\r\n", x);
				prints (comm_putc,str);
				
			}
			
		}
		#endif
		
		if ( COMM_READ_READY)
		{
			prints(comm_putc, ">");
			readline(comm_getc, comm_putc, str, sizeof(str)-1);
		
			prints(comm_putc,"\r\nYou said ");
			prints(comm_putc,str);
			prints(comm_putc,"\r\n");
		}
		
		wcount++;
		
		prints(comm_putc,"Writing test pattern 1!\r\n");
		
		_delay_ms(100);
		
		prints(comm_putc, "Testing RAM data lines\n");
		for(i=0;i<255;i++)
		{
			slowwrite(	i, i);			
		}
		for (i=0;i<255;i++)
		{
			j = slowread(i);
			sprintf(str, "%x ", j);
			prints(comm_putc,str);
			
			if	(i!=j) 
				prints(comm_putc, "<-err\n");
			
		}
		
	//	pageselok=0;
		
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{			
				slowwrite((j<<8) | i, 0xff ) ;
			}
		}
		
		prints(comm_putc,"writing color pattern \r\n");
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{
				char c;
				c= i>>2;
				
				if (j<32)
					c&=0b000011;
				else if (j<64)
					c&=0b001100;
				else if (j<128)
					c&=0b110000	;
				
				slowwrite((j<<8) | i, c ) ;
			}
		}
		
		
		
		//pageselok=1;
		_delay_ms(1000);
	//	pageselok=0;

		prints(comm_putc,"writing test pattern 2\r\n");	
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{
				slowwrite((j<<8) | i, 0x01 ) ;
			}
		}
		
		prints(comm_putc,"writing test pattern 3\r\n");
	//pageselok=1;
	_delay_ms(100);
	//pageselok=0;

		
		//green should be faster than all the others
		START_FAST_WRITE;
		for (j=0;j<240;j++)
		{
			//cli();
			SELECT_RAM_PAGE(j);
			//sei();
			for (i=0;i<256;i++)
			{
				FAST_WRITE(i,0x02);
			}
		}
		END_FAST_WRITE;
		
		//pageselok=1;
		_delay_ms(100);
		//pageselok=0;

	
		prints(comm_putc,"writing test pattern 4 perfect?\r\n");
		
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{
				slowwrite((j<<8) | i, 0x04 ) ;
			}
		}
		
	//pageselok=1;
	_delay_ms(100);
	//pageselok=0;

		

		prints(comm_putc,"writing test pattern 5\r\n");
			
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{
				slowwrite((j<<8) | i, (j>>2) ^  (i>>2) ) ;
			}
		}
	
	

	//pageselok=1;
	_delay_ms(100);
	//pageselok=0;

	
		//draw cat
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{
				unsigned char catx = (i ) & 0xf;
				unsigned char caty = (j ) & 0xf;
				slowwrite((j<<8) | i, cat[(caty<<4) | catx] ) ;
			}
		}
		
		//draw cat large, multicolor
		START_FAST_WRITE;
		for (j=0;j<240;j++)
		{
			unsigned char caty = (j/4 ) & 0xf;
			SELECT_RAM_PAGE(j);
			for (i=0;i<256;i++)
			{
				unsigned char catx = (i/4 ) & 0xf;
				
				unsigned char color = (((i/2)+(j/2))%7)+1;
				
				//slowwrite((j<<8) | i,  (cat[(caty<<4) | catx])  *  color) ;
				FAST_WRITE(i,(cat[(caty<<4) | catx])  *  color );
			}
		}
		END_FAST_WRITE;
		
		
		//black
		for (j=0;j<240;j++)
		{
			for (i=0;i<256;i++)
			{
				slowwrite((j<<8) | i,  0  ) ;
			}
		}
			
			hires=1;
			
			//draw a character
			HOME;
			
			for (i=0;i<16;i++)
			{
				sprintf(str, "Hello hires %d!\n" , i);
				
				screen_writehi(str);
				
			}
			
			A16(1);
			//black
			for (j=0;j<240;j++)
			{
				for (i=0;i<256;i++)
				{
					slowwrite((j<<8) | i,  0  ) ;
				}
			}
			HOME;
			
			for (i=0;i<16;i++)
			{
				sprintf(str, "Hello hires side 2 %d  %c %c !\n" , i,1,1);
				
				screen_writehi(str);
				
			}
			A16(0);
			
			for (j=0;j<5;j++)
			{
				
				
				//VGA_SCREEN(1);
				A16(1);
				_delay_ms(300);
			
				//VGA_SCREEN(0);
				A16(0);
				_delay_ms(300);
			}
			hires=0; //back to lowres
			
			
			#if 0
			//clear
			for (j=0;j<240;j++)
			{
				for (i=0;i<256;i++)
				{
					slowwrite((j<<8) | i,  0  ) ;
				}
			}
			
			for (j=0;j<10;j++)
			{
				
			
				for (i=0;i<240; i+=1 )
				{
				
				
					drawchar(i, i, '*', 2, 0);
					drawchar(i, 240-i, '-', 7, 0);
					//_delay_ms(10);
				
					drawchar(i, i, ' ', 2, 0);
					drawchar(i, 240-i, ' ', 3, 0);
				
				}
			
			}
			_delay_ms(1000);
			#endif
			//pageselok=1;
			
		sei();	
	}
}

