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


void memcpyi2x(unsigned int addr, char* internal, unsigned int cnt) {
	

	unsigned int final = addr+ cnt-1;
	unsigned char finalpage = final>>8;
	unsigned char finaloffset=final;

	unsigned char page = addr>>8;
	unsigned char offset = addr;

	if (cnt==0)
		return;
		
	//select the first page
	SELECT_RAM_PAGE(page);
	START_FAST_WRITE;
	
	for(;;) {
		
		FAST_WRITE(offset,*(internal++));  //write a byte
		
		//check done
		if ((page==finalpage) && (offset==finaloffset)) 
			break;
		
		if(offset == 0xff){
			
			page++;
			SELECT_RAM_PAGE(page);
			
		}
		offset++;
	}
	
	END_FAST_WRITE;
		
}


void memcpyx2i( char* internal, unsigned int addr,  unsigned int cnt) {
	

	unsigned int final = addr+ cnt-1;
	unsigned char finalpage = final>>8;
	unsigned char finaloffset=final;

	unsigned char page = addr>>8;
	unsigned char offset = addr;

	if (cnt==0)
		return;
	
	//select the first page
	SELECT_RAM_PAGE(page);
	//START_FAST_WRITE;
	
	for(;;) {
						
		FAST_READ( *(internal++) ,  offset);
		
		//check done
		
		if ((page==finalpage) && (offset==finaloffset))
		break;
		
		if(offset == 0xff){
			
			page++;
			SELECT_RAM_PAGE(page);
			
		}
		offset++;
	}
	
	
	
}


