
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
//#define VGAKEYPOLL


#ifdef VGAKEYPOLL
#include "keyps2.h"
#endif

#define io(addr) _SFR_IO_ADDR(addr)

.data

#ifdef VGAKEYPOLL
ps2portbuffer:
.byte 0
#endif

#ifdef DEBUGCOUNTING
debuglines:
.word 0
#endif

.text





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


#define HSYNCTIMERTOGGLE   (1<<COM1B0)
#define HSYNCTIMERLOW   (1<<COM1B1)
#define HSYNCTIMERHIGH  (1<<COM1B1) | (1<<COM1B0)

#define VSYNCTIMERLOW   (1<<COM1A1)
#define VSYNCTIMERHIGH  (1<<COM1A1) | (1<<COM1A0)


.global TIMER1_CAPT_vect
 TIMER1_CAPT_vect :
 
#define TFLAG 6

push zh  //start as late as 11
push zl  
in zl, io(SREG)
push zl

countUntil 28

#ifdef VGAKEYPOLL  //this delays everything 3 clocks
in zl, io(KEY_PIN)
sts ps2portbuffer,zl
#endif

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



/*
//countUntil 30

	//setup hsync pulse at correct place
	ldi zl, 0
	sts OCR1BH+1, zl
	ldi zl, HSYNCGOESHIGH
	sts OCR1BL ,zl  //set timer position for hsync
	ldi zl, HSYNCTIMERHIGH  //hsync high at trigger value.  vsync returns to normal use, which should be setup earlier
	sts TCCR1A, zl          //timer set for hsync, lets do other stuff



	//get last page user program switched to
	lds zl, row   
	lds zh, row+1 
	
	//divide by 2
	lsr zh  //shift top byte right, discard into carry
	ror zl   //shift low byte right, bring in carry to bit 7

	cpi zl, 240  //top 480 lines are considered active

	brlo activeline
	
	rjmp novideoline

	activeline:
	*/
	//we are active video line


	skipline:

	sts evenodd, zh  //store new even/odd value

	lds zl, drawrow
	inc zl
	sts drawrow, zl

	nop
	jmp blanklines



	//TODO: CRAP, realized this is only running 512 lines not 525... need to come up with a way to count the extra lines at the end of blanking
	//     maybe after we finish the vertical blanking pulse, we can use bits of the even/odd counter to track extra lines
	//     just need a few more lines ...
.global vidfull
vidfull:  //starts at 34

	//toggle hsync



	lds zh, evenodd
	lds zl, drawrow
	subi zh, -128 //flip top bit of this word
	sbrs zh,7	//if high bit is one, don't increment (will increment if high bit is 0)
	inc zl
	sts drawrow, zl
	sts evenodd, zh
	lds zh, vcnt
	cp zl, zh
	
	prebrshblanklines:
	brsh blanklines  //clock 48

	

	lds zh, vscroll
	add zl, zh
	
	ldi zh, 1<<FOC1B
	sts TCCR1C ,zh   //hsync goes high here ... beginning of front porch   //clock 53
	
	

  

	//save ram WE/OE/PL bits
	
	in zh, io(RAMCTRL_PORT)
	push zh

		//save whatever data dir
		in zh, io(RAMDATA_DDR)
		push zh

			//save ram data that was being asserted
			in zh, io(RAMDATA_PORT)
			push zh
							
				//save the address being set
				in zh, io(RAMADDR_PORT)
				push zh

					//save the ram bank select bit
					in zh,io(RAMEX_PORT)
					push zh
						//TODO: not yet selecing a ram bank: need to pick one explicitly

						//zl still has draw row
						
						sbi io(RAMCTRL_PORT), RAMCTRL_WE_BITNUM
									
						ldi zh, 0
						

						out io(RAMDATA_DDR) ,zh //ramdata is now input (floating)
						cbi io(RAMCTRL_PORT), RAMCTRL_OE_BITNUM;  //OE low, lets output
						
						out io(RAMADDR_PORT), zl //output rownum address

						sbi io(RAMCTRL_PORT), RAMCTRL_PL_BITNUM //latch
						cbi io(RAMCTRL_PORT), RAMCTRL_PL_BITNUM //latch off			
						lds zh, hscroll  //start value for x		
						cbi io(VGA_DAC_PORT), VGA_DAC_BITNUM  //dac on  //fisrt pixel (0)

						PX  //1
						PX  //2
						PX  //3
						PX4 //8
						PX4 //12
						PX4 //16
						PX16 //32
						PX32 //64
						PX32 //96					
						PX32 //128

						//other 128 pixels
						PX32 
						PX32 
						PX32 						
						#ifndef DEBUGCOUNTING
						PX32 
						//Notch out some pixels on right side of screen if we are doing extra debug tracking 
						#endif
						
						//end output
						sbi io(VGA_DAC_PORT), VGA_DAC_BITNUM  //stop ramdac
						sbi io(RAMCTRL_PORT), RAMCTRL_OE_BITNUM  //float ramdata
						
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
		


	
//restore machine
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
