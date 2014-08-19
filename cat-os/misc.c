/*
 * misc.c
 *
 * Created: 7/14/2014 8:26:58 PM
 *  Author: mark
 */ 

#include "misc.h"

void prints( chardevice_t* dev, char* str) {
	
	while(*str)
	{
		dev->putc(*str);
		str++;
	}
	
}


void readline( chardevice_t* dev, char* str, uint16_t buffersize, unsigned char echo) {
		uint8_t c=0;
		uint16_t cnt = 0;
				
		while ((cnt < buffersize))
		{
			c = dev->getc();
			
			
			if(echo)
				dev->putc(c); //echo it
			
			if (c=='\n')
			break; //done
			
			*(str++) = c;
			cnt++;
		}
		
		*str = 0; //null terminate
	
}
