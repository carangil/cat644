/*
 * COMM.h
 *
 * Created: 7/14/2014 7:54:12 PM
 *  Author: mark
 */ 


#ifndef COMM_H_
#define COMM_H_



typedef struct comm_ioctl_parms_s
{
	uint16_t baud;
	unsigned char enable_115200;
} comm_ioctl_parms_t;

unsigned char comm_ioctl(devid dev, unsigned char num, void* parms);

#define COMM_IOCTL_INITBAUD 1

void comm_putc(devid dev, uchar ch);
uchar comm_getc(devid dev);
uchar comm_kbhit(devid dev);

#endif /* COMM_H_ */