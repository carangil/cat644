
/*
 * intvga.s
 *
 * Created: 9/4/2018 8:39:13 PM
 *  Author: mark
 */ 
#include <avr/io.h>
#include "xram.h"
#include "vgadefs.h"


//#define DEBUGCOUNTING
#define VGAKEYPOLL

//TFLAG position in SREG
//haven't needed it yet....
#define TFLAG 6

//use one of our single cycle registers that C doesn't touch

#ifdef VGAKEYPOLL
#include "keyps2.h"
#define PREV_PS2CLK GPIOR0
#endif

#define io(addr) _SFR_IO_ADDR(addr)

.data

#ifdef VGAKEYPOLL

bitcount:
.byte 0

bits:
.byte 0


#endif

#ifdef DEBUGCOUNTING
debuglines:
.word 0
#endif

.text




// I used to use TvOut's 'wait_until' function, but decided to try to implement my own logic

.macro countUntil goal
		lds zl, TCNT1L		
		subi zl, \goal - 1 - 7  // (-1 is because LDS takes 2 cycles, but loaded the counter in between those two.  -7 is because it takes 7 clocks for the equalize branching

		//each loop takes 3 cycles, and runs until we are no more than 3 past the goal
	cu_again\@:
		subi zl, -3
		brmi cu_again\@

		//at this point we are at GOAL(-7) counter value, or +1 or +2

		//start counting here: 0
		breq  cu_over0\@  //if went over by zero, branch, and that takes 2 cycles
		//+1   if didn't branch we are +1 whatever we are over by
		subi zl, 1
		//+2   
		breq cu_over1\@  //if we were over by 1, branchm and that takes 2 cycles.  we are at +2 what we are over, so we will land on over1 at +4 what we are over. we are over by 1, so we are at goal+5
		//+3
		rjmp cu_over2\@  //we are at +3 what we are over, and at this point must be over by 2, so we are at goal +5. branch takes 2, so lanf at over2 at goal +7

	cu_over0\@:
		//+2  (from jump)
		nop
		//+3
		nop
		//+4
		nop
		//+5

	cu_over1\@:
		//+5 if from above
		//if from over1 is +4, but was one over, so +5 eqivalent
		nop
		nop
	cu_over2\@:
		//+7 if from above, or +5 if from being over 2, so +7 equicalent
	cu_equalized\@:
		//at equalized we are at goal +7  Since we subtracted 7 from goal at start, we are at goal
.endm


//output 1 pixel
.macro PX
	inc zh
	out io(RAMADDR_PORT), zh
.endm

.macro PX4
	PX
	PX
	PX
	PX
.endm

#if 0
//experimental attempts to get some nops in when outputting pixels
//nop can later be replaced with single-cycle instructions

.macro PX2noppy_odd_even
	nop
	out io(RAMADDR_PIN), zl  //odd px
	subi zh, -2  //skip 2
	out io(RAMADDR_PORT)	,zh
.endm


.macro PX2noppy_even_odd
//untested
	subi zh, -2  //skip 2
	out io(RAMADDR_PORT)	
	nop
	out io(RAMADDR_PIN), zl  //odd px
.endm
#endif

.macro PX16
	PX4
	PX4
	PX4
	PX4
.endm
.macro PX32
	PX16
	PX16
.endm
.macro PX64
	PX32
	PX32
.endm


#define HSYNCTIMERTOGGLE	(1<<COM1B0)
#define HSYNCTIMERLOW		(1<<COM1B1)
#define HSYNCTIMERHIGH		(1<<COM1B1) | (1<<COM1B0)

#define VSYNCTIMERLOW		(1<<COM1A1)
#define VSYNCTIMERHIGH		(1<<COM1A1) | (1<<COM1A0)


// PS2 keyboard code 

gotbyte:
	//bits should contain a byte that needs to be inserted into key buffer

	push xl
	ldi xl, 0  //need a zero

	push yl					//need a register
	lds zl, keywritepos
	lds zh, keyreadpos
	dec zh
	andi zh, 0xF			//buffer is 15 long, wraparound
	cp zl,zh				//if read-1 == write, buffer is full
	breq full 

	//otherwize we write
	ldi zh, lo8(keybuffer)  //get keybuffer low pointer
	add zl, zh				//add offset to pointer
	ldi zh, hi8(keybuffer)	 //get high part of pointer
	adc zh, xl				//add the zero to it, but WITH carry

	lds yl, bits
	st z, yl

	lds zl, keywritepos
	inc zl
	andi zl, 0xF
	sts keywritepos, zl


full:						//runs if full, or if we inserted a byte from above
	pop yl
	pop xl
	ldi zh,0
	sts bitcount, zh


//done handling this invokation of keyboard routine
ps2done:

	//need to increment row counters, possible set up vsync
	//this copied from vidfull.
	
	lds zh, evenodd
	lds zl, drawrow
	subi zh, -128 //flip top bit of this word
	sbrs zh,7	//if high bit is one, don't increment (will increment if high bit is 0)
	inc zl
	sts drawrow, zl
	sts evenodd, zh
	jmp blanklinesalreadyhsync

fardops2:

	sbi io(PCIFR), 0 ; clear flag;
	in zl, io(KEY_PIN);		//read bit, will be in position KEY_DATA_POS
	push zl					//save before timer code

	countUntil 52
	ldi zh, 1<<FOC1B
	sts TCCR1C ,zh   //hsync goes high here ... beginning of front porch   //clock 53

	pop zl //get it back

	sbrc zl, KEY_CLK_POS //skip the next jump if low
	jmp ps2done      //clock is high: this was rising edge, not for us

	//at this point is a low edge

	lds zh, bitcount
	cpi zh, 10


	//below is equivalent to brsh gotbyte; just jumping too far away
	brlo nogotbyte 
	rjmp gotbyte
	nogotbyte:

	cpi zh, 9
	brsh bitinc //parity bit; don't take it in

	//take a bit

	lds zh, bits
	lsr zh
	sbrc zl, KEY_DATA_POS
	ori zh, 0x80
	sts bits,zh

bitinc:

	lds zh, bitcount
	inc zh
	sts  bitcount, zh

	jmp ps2done



//jmp videxit

dops2:
	rjmp fardops2

.global TIMER1_CAPT_vect
 TIMER1_CAPT_vect :
 
	//uncomment to simulate interrupt timing variance in the debugger
	//nop
	//nop

	push zh  //start as late as 10
	push zl  
	in zl, io(SREG)
	push zl

	//was using countUntil, but wanted to save more clocks.  Super tiny version


	#define goal 15


	lds zl, TCNT1L		  //counts +1
	subi zl, goal+3		 //+1
	breq three			//+2 if jump           +1 otherwise
	inc zl
	breq two
	inc zl
	breq one
	nop
	rjmp zero
  
three:
	nop  //22 from jump
two:
	nop  //23 from jump
one:
	nop  //24  from jump
zero:  
	//nop  //25 if from jump  , but don't waste time doing nop because it is equalized now


#ifdef VGAKEYPOLL  //this delays everything 3 clock
	in zl, io(PCIFR)
	andi zl, 1
	brne dops2
#endif

	//lookup which video line driver is being used
	lds zl, vidptr
	lds zh, vidptr+1
	ijmp

setvsyncdown:
	//set for vsync to go down when timer hits
	ldi zl, HSYNCTIMERTOGGLE | VSYNCTIMERLOW
	sts TCCR1A, zl

	lds zh, framecount
	lds zl, evenodd
	sbrs zl, 0			//if zl is odd we skip the next instruction            (if zl is even we add one)
	subi zh, -1			//subtract negative 1 ( add 1)
	sts  framecount,zh

	jmp blankpreexit


.global vidskippy
vidskippy:
//this version skips every other line
	lds zh, evenodd

	subi zh, -128 //flip top bit of this word.   -128 is also unsigned 128
	brsh skipline  // if we flippeed from 0x8* to 0x0*, then zh was 'same or higher' than 128 (aka 0x80).  This means this is an 'even' row, so need to skip this line.  the skipline routine should also increment any 
	
	sts evenodd, zh  //store new even/odd value

	lds zl, drawrow  //get the row number to draw (not row of ram, but row of screen)
	lds zh, vcnt     //get total number of lines to draw
	cp zl, zh		//compare
	jmp prebrshblanklines  //jump (unconditional!)  jumping doesn't mess with flags


blankskip:

	nop
	nop

blanklines:

	nop
	nop

	ldi zh, 1<<FOC1B
	sts TCCR1C ,zh   //hsync goes high here ... beginning of front porch   //clock 53

blanklinesalreadyhsync:

	lds zl, drawrow
	cpi zl, 245
	breq setvsyncdown

	//otherwuse just have it come up 
	ldi zl, HSYNCTIMERTOGGLE | VSYNCTIMERHIGH
	sts TCCR1A, zl


blankpreexit:

	//lets get line number again
	lds zl, drawrow
	cpi zl,255
	brne tovidexit  //videxit too far


	//if we are at row 255, then 255 EVEN should be 510 lines before here
	lds zl, evenodd
	andi zl, 0x7f   //mask off the 0x80 'odd' bit
	inc zl		   //count the extra rows in the low nibble
	cpi zl, 16     //510+15 == 525.  needs to be 16 for stupid reasons
	brne noreset
	ldi zl, 0
	sts drawrow,zl

noreset:
	sts evenodd, zl

tovidexit:
	rjmp videxit

	//should start with HSYNC already low



skipline:

	sts evenodd, zh  //store new even/odd value

	lds zl, drawrow
	inc zl
	sts drawrow, zl

	nop
	jmp blanklines



	
.global vidfull
vidfull:  //starts at 34

	
	lds zh, evenodd
	lds zl, drawrow
	subi zh, -128	//flip top bit of this word  important:  Low bits are not modified!
	sbrs zh,7		//if high bit is one, skip the next increment (will increment if high bit is 0)
	inc zl
	sts drawrow, zl	//store updated row
	sts evenodd, zh	//and even/off flag
	lds zh, vcnt	//get # of lines to output
	cp zl, zh		
	
	prebrshblanklines:
	brsh blanklines  //clock 48    if above compare (or compare before jmp to prebrshblanklines) is unsigned >= , these are blank lines

	//at this point, this is a non-blank line; strict schedule from here on out to output pixels on time
	
	lds zh, vscroll	
	add zl, zh		//add vertical scrolling amount to our screen draw row
	
	ldi zh, 1<<FOC1B
	sts TCCR1C ,zh   //hsync goes high here ... beginning of front porch   //clock 53
	
	
	//Now very carefully save the hardware state.  It might be in the middle of a write, read, changing page, anything
  

	//save ram WE/OE/PL bits
	
	in zh, io(RAMCTRL_PORT)
	push zh

		//save whatever data dir
		in zh, io(RAMDATA_DDR)
		push zh

			//save ram data that was being asserted (if there wasn't this harmlessly saves the pullup state)
			in zh, io(RAMDATA_PORT)
			push zh
							
				//save the address being set
				in zh, io(RAMADDR_PORT)
				push zh

					//save the ram bank select bit
					in zh,io(RAMEX_PORT)
					push zh
				
						//now set the state we want

						//pull up write enable (ending any write cycle that was happening. Must be first thing to do before anything else)
						sbi io(RAMCTRL_PORT), RAMCTRL_WE_BITNUM
						
						//set video ram bank 
						andi zh, RAMEX_A16_LOW_MASK
						out io(RAMEX_PORT), zh  //output zh  :sets video to LOW bank only (for now)

						//note: zl still has draw row from a bunch of lines above
						
									
						ldi zh, 0
						out io(RAMDATA_DDR) ,zh //ramdata is now input (floating or pullup state)

						cbi io(RAMCTRL_PORT), RAMCTRL_OE_BITNUM;  //OE low, ram now takes the bus
						
						out io(RAMADDR_PORT), zl				   //output rownum address. zl can now be discarded
				
						sbi io(RAMCTRL_PORT), RAMCTRL_PL_BITNUM //latch high
						cbi io(RAMCTRL_PORT), RAMCTRL_PL_BITNUM //latch low			

						lds zh, hscroll			//start value for x		
						out RAMADDR_PORT, zh	//output first pixel address
						cbi io(VGA_DAC_PORT), VGA_DAC_BITNUM  //dac on  //first pixel (0)  clk 86 This instruction takes 2 clocks, but 1st clock is R, second is write
						PX // 1
						PX // 2
						PX // 3  done 4 pixels

				//comment out 8px
				//		PX4 //up to 8
				//		PX4 //up to 12
					
						PX4 // to 16
					
						PX16 //to 32
						PX32 // to 64

						PX32 
						PX32 //to 128

						//other 128 pixels
						PX32 
						PX32 
						PX32 						
						#ifndef DEBUGCOUNTING
						PX32 
						//Notch out some pixels on right side of screen if we are doing extra debug tracking 
						#endif
						
						//end output
						sbi io(VGA_DAC_PORT), VGA_DAC_BITNUM		//stop ramdac
						sbi io(RAMCTRL_PORT), RAMCTRL_OE_BITNUM		//ram floating (bus is clear)
						
						//restore the previous page the user program set up
						lds zl, lastpage   //get last page user program switched to
						out io(RAMADDR_PORT), zl //output that address
						sbi io(RAMCTRL_PORT), RAMCTRL_PL_BITNUM  //latch up
						cbi io(RAMCTRL_PORT), RAMCTRL_PL_BITNUM //latch down

					//restore ram bank select
					pop zl
					out io(RAMEX_PORT), zl

				//restore address
				pop zl
				out io(RAMADDR_PORT), zl

			//restore data lines
			pop zl
			out io(RAMDATA_PORT), zl


			//restore data direction
		pop zl
		out io(RAMDATA_DDR), zl


	//restore ram command (oe/we, whatever)
	pop zl
	out io(RAMCTRL_PORT), zl
		


	
//restore cpu state
videxit:


	#ifdef DEBUGCOUNTING
	//tracks a row counter for when testing counting in debugger
	//not normally included because it requires we notch out some of the display pixels to do this

	lds zl, debuglines
	lds zh, debuglines+1
	adiw zl, 1
	sts debuglines,zl
	sts debuglines+1,zh

	#endif


	pop zl
	out io(SREG), zl
	pop zl
	pop zh
	reti
	//better be done in less than 636 cycles