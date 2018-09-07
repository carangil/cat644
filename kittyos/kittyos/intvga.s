
/*
 * intvga.s
 *
 * Created: 9/4/2018 8:39:13 PM
 *  Author: mark
 */ 
 #include <avr/io.h>
#include "xram.h"
#include "vgadefs.h"

#define io(addr) _SFR_IO_ADDR(addr)


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





blanklines:

nop
nop

ldi zh, 1<<FOC1B
sts TCCR1C ,zh   //hsync goes high here ... beginning of front porch   //clock 54


lds zl, drawrow
cpi zl, 245
breq setvsyncdown

//otherwuse just have it come up 
ldi zl, HSYNCTIMERTOGGLE | VSYNCTIMERHIGH
sts TCCR1A, zl


rjmp videxit

setvsyncdown:
//set for vsync to go down when timer hits
ldi zl, HSYNCTIMERTOGGLE | VSYNCTIMERLOW
sts TCCR1A, zl

lds zh, framecount
lds zl, evenodd
sbrc zl, 0
subi zh, -1
sts  framecount,zh


rjmp videxit

//should start with HSYNC already low

.global TIMER1_CAPT_vect
 TIMER1_CAPT_vect :
 
#define TFLAG 6

push zh  //start as late as 11
push zl  
in zl, io(SREG)
push zl

countUntil 28
lds zl, vidptr
lds zh, vidptr+1
ijmp

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

.global vidactive
vidactive:  //starts at 34

	//toggle hsync



	//load row
	lds zh, evenodd
	lds zl, drawrow
	inc zh
	andi zh, 1
	brne odd
	inc zl
	odd:
	sts drawrow, zl 
	sts evenodd, zh
	lds zh, vcnt
	cp zl, zh
	brsh blanklines

	lds zh, vscroll
	add zl, zh


	ldi zh, 1<<FOC1B
	sts TCCR1C ,zh   //hsync goes high here ... beginning of front porch   //clock 54
	
	

  

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

						//other 128:
						PX32 
						PX32 
						PX32 
						PX32 
						
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
pop zl
out io(SREG), zl
pop zl
pop zh
reti
