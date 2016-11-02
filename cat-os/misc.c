/*
 * misc.c
 *
 * Created: 7/14/2014 8:26:58 PM
 *  Author: mark
 */ 

#include "misc.h"

//place to write out messages before printing
char scratch[SCRATCH_SIZE]; 

void prints( chardevice_t* dev, char* str) {
	
	if (!dev)    /* so we can handle 'null' output devices */
		return;
		
	while(*str)
	{
		dev->putc(dev, *str);
		str++;
	}
	
}

//reads the string from rom
void printsP( chardevice_t* dev, char* str) {
	
	unsigned char c=1;
	
	if (!dev)    /* so we can handle 'null' output devices */
		return;
	
	while(c=pgm_read_byte(str++))
	{
		dev->putc(dev, c);
	}
	
}




void readline( chardevice_t* dev, char* str, uint16_t buffersize, unsigned char echo) {
		uint8_t c=0;
		uint16_t cnt = 1; //1 instead if 0 to reserve space for terminator
				
		while ((cnt < buffersize))
		{
			c = dev->getc(dev);
			
			
			if(echo)
				dev->putc(dev,c); //echo it
			
			if (c=='\n')
			break; //done
			
			*(str++) = c;
			cnt++;
		}
		
		*str = 0; //null terminate
	
}

