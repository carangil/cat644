/*
 * sdcard.h
 *
 * Created: 8/21/2014 10:41:40 PM
 *  Author: mark
 */ 


#ifndef SDCINCFILE1_H_
#define SDCINCFILE1_H_

#define SDCARD_CS		0b1000
#define SDCARD_CS_PORT  PORTA
#define SDCARD_CS_DDR	DDRA


#define SDCARD_BLOCKSIZE 512

#define SDCARD_SPI_OPTIONS  SPI_MASTER | SPI_CLK_8

void sdcard_init(unsigned long* pcapacity);
uchar sdcard_write_block(blockdevice_t* bdev, long blockaddr, char* buffer, unsigned int numbytes);
uchar sdcard_read_block(blockdevice_t* bdev,long blockaddr, char* buffer, unsigned int numbytes);


#endif /* INCFILE1_H_ */