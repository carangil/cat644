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


unsigned char comm_ioctl(unsigned char num, void* parms);
#define COMM_IOCTL_INITBAUD 1


void comm_putc(unsigned char c);
unsigned char comm_getc();
unsigned char comm_kbhit();




#endif /* COMM_H_ */