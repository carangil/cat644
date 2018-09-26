/*
 * spi.h
 *
 * Created: 8/21/2014 9:30:59 PM
 *  Author: mark
 */ 


#ifndef SPI_H_
#define SPI_H_

#include "avrstuff.h"



//return true when byte is ready to read
#define SPI_READY (SPSR & _BV(SPIF))

//read/write from this to do spi operations
#define SPI_RW SPDR



/* inputs to spi_init function */
#define SPI_MASTER  _BV(MSTR)
#define SPI_CLK_4    0
#define SPI_CLK_16   _BV(SPR0)
#define SPI_CLK_64   _BV(SPR1)
#define SPI_CLK_128  (_BV(SPR0) | _BV(SPR1))

#define SPI_CLK_2    ((_BV(SPI2X)<<8) | SPI_CLK_4)
#define SPI_CLK_8    ((_BV(SPI2X)<<8) | SPI_CLK_16)
#define SPI_CLK_32    ((_BV(SPI2X)<<8) | SPI_CLK_64)




#define SPI_CPHA1   SPCR |=  ( _BV(CPHA));
#define SPI_CPHA0   SPCR &= ~( _BV(CPHA));

void spi_init(unsigned int settings);
//void spi_disable();
//void spi_enable();



typedef struct {
	chardevice_t chardev;
	uchar state;
	uchar locked;
} spi_device_t;

#define SPI_STATE_IDLE 0
#define SPI_STATE_EXCHANGING 1


extern spi_device_t dev_spi0;

#endif /* SPI_H_ */