/*
 * xram.c
 *
 * Created: 7/14/2014 11:03:12 PM
 *  Author: mark
 */ 

#include "xram.h"

volatile unsigned char lastpage=0;

void xram_init()
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
	
	#ifdef XRAM_128K
		RAMEX_DDR |= RAMEX_DDR_MASK;  //enable one output
		SETA16LOW;
	#endif
}
