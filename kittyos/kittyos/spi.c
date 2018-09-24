/*
 * spi.c
 *
 * Created: 8/21/2014 10:30:35 PM
 *  Author: mark
 */ 
#include "avrstuff.h"
#include "spi.h"




void spi_init(unsigned int settings)
{

	
	/* SCLK, MOSI, SS outputs */
	DDRB |= 0b10110000;
	PORTB=0x00;
	
	/*SPRO = clk/16*/
	
	//set enable and all settings bits
	SPCR =  _BV(SPE) | (settings & 0xFF);
	SPSR =  settings>>8;
	
}


void spi_disable(){
	SPCR &= ~_BV(SPE);
}

void spi_enable(){
	SPCR |= _BV(SPE);
}


uchar spi_inout(uchar val)
{
	/* Write */
	SPDR = val;
			
	/* Wait */
	while (!SPI_READY) ;
	
	
	/* Read byte */
	val = SPDR;
		
	return val;
}