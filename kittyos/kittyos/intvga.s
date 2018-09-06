
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
.global TIMER1_COMPA_vect

#define GOAL 20

TIMER1_COMPA_vect:

push zl  
in zl, io(SREG)
push zl


nop
lds zl, TCNT1L            
subi zl, GOAL-1

//each loop takes 3 cycles, and runs until we are no more than 3 past the goal
again:
subi zl, -3
brmi again

//at this point we are at GOAL counter value, or +1 or +2

//start counting here: 0
breq  over0
//+1
subi zl, 1
//+2
breq over1
//+3
rjmp over2

over0:
//+2  (from jump)
nop
//+3
nop
//+4
nop
//+5

over1:
//+5 if from above
//if from over1 is +4, but was one over, so +5 eqivalent
nop
nop
over2:
//+7 if from above, +5 if from being over 2, so +7 equicalent
equalized:
nop  //interrupt latency is equalized here


//restore machine

pop zl
out io(SREG), zl
pop zl

reti