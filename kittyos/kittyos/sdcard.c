/*
 * sdcard.c
 *
 * taken from failed "catos"
 *  Author: mark
 */ 

#include "avrstuff.h"
#include "kittyos.h"

#include "spi.h"
#include "sdcard.h"

void sdcard_enable();
void sdcard_disable();

#define SDCARD_SEND_TIMEOUT 0xFF

//only 1 sdcard is supported (currently), and its using the only supported chardev
//TODO: consider making 'exchange' a supported chardev operation, which optizes for spi devices.
// this sdcard driver 
static uchar spi_inout(uchar c){
	uchar d;
	
	dev_spi0.chardev.write1(&dev_spi0.chardev,c);
	return dev_spi0.chardev.read1(&dev_spi0.chardev);
	
}

unsigned char sdcard_send(unsigned char cmd, uint32_t parm)
{
	unsigned char resp = 0;
	unsigned int tries = 0;

	spi_inout(cmd);	
	spi_inout((parm>>24) & 0xff);
	spi_inout((parm>>16) & 0xff);
	spi_inout((parm>>8) & 0xff);
	spi_inout(parm & 0xff);
	spi_inout(0x95);  /* checksum for cmd '0|40'.  all other cmds this will be ignored if we are in spi mode */
	
	while(1)
	{
		resp = spi_inout(0xff);
		/* 1st bit will go low when response is done */
				
		if (! (resp & 0x80 ) )
			return resp;
	
		tries++;
		if ( tries > 1000 )	
			break;
	
	}
	
	return SDCARD_SEND_TIMEOUT;
}

void sdcard_enable()
{
	//low bit	
	SDCARD_CS_PORT &= ~ SDCARD_CS;
}

void sdcard_disable()
{
	//high bit
	
	SDCARD_CS_PORT |=  SDCARD_CS;
	SDCARD_CS_DDR |= SDCARD_CS;
	
	spi_inout(0xff);  //dummy byte to force release

}

unsigned long sdcard_get_size()
{
	unsigned char r;
	unsigned char i;
	
	unsigned int read_bl_len=0;
	unsigned int csize=0;
	unsigned int mult=0;
	unsigned long size;
	
	sdcard_enable();
		
	r = sdcard_send( 0x40 | 9 , 0);  //read CSD
	
	while (r!= 0xFE)
	{
		r = spi_inout(0xff)		;
	}
	
	/*  version 1 CSD (SDCARD not SDHC)
		byte	bits
		0		120
		1		112
		2		104
		3		96
		4		88
		5		80   83-80 is READ_BL_LEN
		6		72     79 ...   73 72
		7		64              
		8		56    (62 to 73 is C_SIZE)    63 62 61 60 59 58 57 56
		9		48       ...   49  48
		10		40      47  ... 40
		11		32
		12		24
		13		16
		14		8
		15		0	
		
	
	*/
	
	for (i=0;i<16;i++)
	{
		r = spi_inout(0xff);
		
		if (i==5)
			read_bl_len = r & 0x0f;
		
		if (i==6)
			csize = r &0b11;  //last two bits of byte
			
		if (i==7) 
			csize = (csize <<8) | r;  /*next 8 bits */
			
		if (i==8)
		
			csize = (csize <<2) | (r>>6);  /*next 2 bits*/
		
		if (i==9)
			mult = r & 0b11;  /* bits 49,48 */
			
		if (i==10)
			mult = (mult<<1) | (r>>7);  /*bit 47*/
				
	}
	
	sdcard_disable();
	
	DMESGF( "csize %d", csize); 
	DMESGF("csizemult %d", mult); 
	DMESGF("readbl %d", read_bl_len);
	
	size = (unsigned long) (csize+1) * (((unsigned long)1)<<(mult+2)) *(1<<read_bl_len) ;
	size = size >>9;  /* blocks of 512 */
	return size;
}

void sdcard_init(unsigned long* pcapacity)
{
	char r=0;
	char i;
	
	int trycount=0;
		
	//spi_enable();
	dev_spi0.chardev.dev.ioctl(&dev_spi0, IOCTL_LOCK, SDCARD_SPI_OPTIONS);
	
	while (r != 0x01 ){
		DMESG("Attempt init sdcard\n");
	
		SDCARD_CS_PORT |=  SDCARD_CS;
		SDCARD_CS_DDR |= SDCARD_CS;
	
		DMESG("init1\n");
	
		for(i=0; i<10; i++) // idle for 10 bytes / 80 clocks
			spi_inout(0xFF);
		
		sdcard_enable();
		r = sdcard_send(0x40, 0 );
		sdcard_disable();
	
		DMESGF("cmd0:%d;\n",r );
		
	
		if (r== SDCARD_SEND_TIMEOUT) {
			DMESG("SDCARDTIMEOUT CMD0;\n");	
		}
	}
	
	
	/* keep sending 41 until a '0' is received */	
	while(1) 
	{
		sdcard_enable();
		r = sdcard_send(0x41, 0);
		sdcard_disable();
		if (!r)
			break;
	}
	
	
	/* set block size */
	sdcard_enable();
	sdcard_send(0x50, 512);
	sdcard_disable();
	
	
	if(pcapacity)
		*pcapacity = sdcard_get_size();
		
	dev_spi0.chardev.dev.ioctl(&dev_spi0, IOCTL_UNLOCK, 0);
}


/* enables sd card and checks for busy. if busy waits until card is available, then returns*/
void sdcard_enable_waitbusy(){  
	uchar x;
	
	dev_spi0.chardev.dev.ioctl(&dev_spi0, IOCTL_LOCK, SDCARD_SPI_OPTIONS);
	sdcard_enable();
		
	while(1){
		
		x = spi_inout(0xff);
		
		sdcard_disable();  //card off
			
		
		sdcard_enable();  //card on
		
		if (x != 0x00)  //was OK
			break;
		DMESGF( "[B%d]", x);
		
	}
	
}


uchar sdcard_read_block(blockdevice_t* bdev,long blockaddr, char* buffer, unsigned int numbytes) {
	char r;

	int i;
	
	if (numbytes > 512)
		return 0;

	sdcard_enable_waitbusy();
	
	r = sdcard_send(0x51, blockaddr <<9 );
	
	while (r!= 0xFE)
	{
		r = spi_inout(0xff)		;
	}
	
		
	for (i=0;i<numbytes;i++)
	{
		*(buffer++) = spi_inout(0xff);
		//SPI_RW = 0xff;  //write dummy byte
		//while (!SPI_READY);  //wait until byte is ready
		//*(buffer++) = SPI_RW;  //read in the byte
	}
	
	for( ; i<512;i++) {
		spi_inout(0xff);
		//SPI_RW = 0xff;
		//while (!SPI_READY);

	}
	
	//read dummy checksum
	
	spi_inout(0xff);
	spi_inout(0xff);
	
	sdcard_disable();
	//spi_disable();
	dev_spi0.chardev.dev.ioctl(&dev_spi0, IOCTL_UNLOCK, 0);
	
	if (r == 0xfe)
		return 1; //success
	else
		return 0;  //fail
	
}


uchar sdcard_write_block(blockdevice_t* bdev, long blockaddr, char* buffer, unsigned int numbytes) {
	unsigned char r;
	int i;
	
	if (numbytes > 512)
		return 0;  //fail
	
	
	sdcard_enable_waitbusy();
	
	
	r = sdcard_send(0x58, blockaddr <<9 );


	spi_inout(0xff); /* dummy */
	
	spi_inout(0xfe); /* data marker */
	
	
	
	for (i=0;i<numbytes;i++)
	{	
		spi_inout( *(buffer++));
	}
	
	for (;i<512;i++) 
	{
		spi_inout(0xab);		/*dummy data for unused part of block */
	}
	
	
	//send dummy checksum
	spi_inout(0xff);
	spi_inout(0xff);
	
	r = spi_inout(0xff);  //get return value
	

	//sdcard_disable();
	dev_spi0.chardev.dev.ioctl(&dev_spi0, IOCTL_UNLOCK, 0);

	r &= 0x1F;  /* Must mask response */

	if (r == 0x05){
		debug("[Wsuccess]");
		return 1;  //success	(data was accepted)
	}
	
	DMESGF("[Wfail %x]",(int)r);

	return 0;  //fail    (data was wrong)
	
}

