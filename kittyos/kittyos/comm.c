/*
 * comm.c
 *
 * Created: 7/14/2014 7:59:02 PM
 *  Author: mark
 */ 

#include "avrstuff.h"
#include "drivers.h"
#include "comm.h"





//internal: use ioctl interfact instead
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



uchar comm_ioctl(device_t* dev, uchar ctlnum, uint16_t param)
{
	if (ctlnum == IOCTL_BAUD) {
		
		if (param == COMM_11520)
			comm_init_115200();

		else
			comm_init(param);
		
		dev->flags = 0;
		
		return 0;
	}
		
	//unknown ioctl command		
	return ERR;
}


#define COMM_SEND_READY  ( UCSR0A& _BV(UDRE0) )

void comm_putc(chardevice_t* dev, unsigned char c)
{
	//wait for it
	while(!COMM_SEND_READY);
	
	//write to output register
	UDR0 = c;
}


#define COMM_READ_READY ( UCSR0A & _BV(RXC0) )

uchar comm_getc(chardevice_t* dev)
{
	while(!COMM_READ_READY);
	return UDR0;

}

unsigned char comm_kbhit(chardevice_t* dev)
{
	return COMM_READ_READY;	
}

//create the chardevice

chardevice_t dev_ser0 = {{comm_ioctl, DEV_FLAG_UNINIT}, comm_putc, comm_getc, comm_kbhit };
