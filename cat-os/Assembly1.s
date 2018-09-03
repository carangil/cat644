
/*
 * Assembly1.s
 *
 * Created: 11/23/2016 4:32:58 PM
 *  Author: mark
 */ 

 
#include <avr/io.h>
#include "xram.h"
#include "vgaports.h"



//lowercase version of registers are the IO addresses for in/out/cbi/sbi
#define sreg	_SFR_IO_ADDR(SREG)

#define porta	_SFR_IO_ADDR(PORTA)
#define portb	_SFR_IO_ADDR(PORTB)
#define portc	_SFR_IO_ADDR(PORTC)
#define portd	_SFR_IO_ADDR(PORTD)
#define ddrc	_SFR_IO_ADDR(DDRC)


.global xTIMER1_COMPA_vect
xTIMER1_COMPA_vect:
jmp video256





/* This routine starts with the hsync pulse low, because it is a timer oscillator output
*/


video256:
	push r16
	push R1
	in r16, sreg
	push r16

	//CAPTURE AND MODIFY CURRENT EXTERNAL MEMORY STATE


	//captures asserted ram (and other) control signals 
	in r16, portd
	push r16
		ori r16, (RAMCTRL_WE_MASK | RAMCTRL_OE_MASK) //disable both write and ram output
		out portd, r16  
	
		//captures asserted data direction
		in r16, ddrc
		push r16

			//captures a16 line state
			in r16, porta
			push r16

				//captures asserted address
				in r16, portb
				push r16

					//make data bus an input
					ldi r16,0
					out ddrc, r16  

					//captures asserted data
					in r16, portc
					push r16


						//do stuff here




	
						//PUT STATE BACK

					//put original data back on bus
					pop r16
					out portc, r16

						//fix row latch here TODO


				//put original address back on the bus
				pop r16
				out portb, r16

			//handle A16 control line
			pop r16
			out porta, r16

		//put data bus back in original directionm
		pop r16
		out ddrc, r16

	//put back original control signals
	pop r16
	out portd, r16

	//outro
	pop r16
	out sreg, r16
	pop r1
	pop r16
	reti

