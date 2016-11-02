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

uchar sdcard_read_block(uchar minor, long blockaddr, char* buffer, unsigned int numbytes);
uchar sdcard_write_block(uchar minor, long blockaddr, char* buffer, unsigned int numbytes);
uchar sdcard_ioctl(uchar minor, uchar num, void* parameter);

//return value is number of blocks
void sdcard_init(uchar minor, unsigned long *capacity);

#define SDCARD_IOCTL_INIT 1


#endif /* INCFILE1_H_ */