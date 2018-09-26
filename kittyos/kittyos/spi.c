/*
 * spi.c
 *
 * Created: 8/21/2014 10:30:35 PM
 *  Author: mark
 */ 
#include "avrstuff.h"
#include "kittyos.h"
#include "spi.h"

//raw spi driver

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

//replaced inout with seperate read/write functions


extern spi_device_t dev_spi0;

//atmega644 only has ONE spi port, so dev is ignored for now.
 
uchar spi_ioctl(device_t* dev, uchar ctlnum, uint16_t param)
{
	
	if (ctlnum == IOCTL_LOCK) { 
		
		//The SPI lock makes sure only 1 thread uses SPI at a time, and that each time SPI is used, it is placed in the correct mode and speed
		
		
		//TODO FIXME:	spi interferes with external memory address port
		//				this is currently unavoidable and messes with the display if disk access is not constrained to vertical blanking
		//				BUT while doing SPI no other threads/processes can read/write external memory
		//				spi must either block out other threads completely, OR there needs to be a lock around xmem usage.
		
		if (dev_spi0.locked)
			return ERR;
		dev_spi0.locked=1;
		spi_init(param);
		dev_spi0.state=SPI_STATE_IDLE;
		return SUCCESS;
	}
	
	if (ctlnum == IOCTL_UNLOCK) {
		if (!dev_spi0.locked)
			return ERR;
		dev_spi0.locked=0;
		spi_disable();
		return SUCCESS;
	}
	
	//unknown ioctl command
	return ERR;
}

uchar spi_getc(chardevice_t* dev){
	
	if(dev_spi0.state == SPI_STATE_EXCHANGING){
		//if the last thing we did was send
		//then wait, and return the byte
		while (!SPI_READY) ;
		dev_spi0.state = SPI_STATE_IDLE;
		return SPDR;
	}
	
	dev_spi0.chardev.dev.flags = DEV_FLAG_ERR;
	
	return 0;
}

void spi_putc(chardevice_t* dev, uchar byte){
	
	
	if(dev_spi0.state == SPI_STATE_EXCHANGING){
		//if the last thing we did was send
		//then wait, and return the byte
		while (!SPI_READY);
	}
	
	SPDR = byte;
	
	dev_spi0.state = SPI_STATE_EXCHANGING;
}

uchar spi_ready(chardevice_t* dev){
	//returns 1 is the last state was exchaning (which means something was sent, and a reply is expected), and the exchange is finished
	
	return (dev_spi0.state == SPI_STATE_EXCHANGING) && (SPI_READY);	
	
}


///pi_device_t dev_spi0 ;
spi_device_t dev_spi0 = { {{spi_ioctl, DEV_FLAG_UNINIT}, spi_putc, spi_getc, spi_ready}     ,0,0};
//chardevice_t dev_spichar =  {{spi_ioctl, DEV_FLAG_UNINIT}, spi_putc, spi_getc, spi_ready};

//ardevice_t dev_ser0 = {{comm_ioctl, DEV_FLAG_UNINIT}, comm_putc, comm_getc, comm_kbhit };