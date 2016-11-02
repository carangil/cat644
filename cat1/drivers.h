/*
 * drivers.h
 *
 * Created: 7/14/2014 8:07:59 PM
 *  Author: mark
 */ 


#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "avrstuff.h"




#define IOCTL_SUCCESS	0
#define IOCTL_ERR		1



typedef struct chardevice_s {
	void (*putc) (unsigned char);
	unsigned char (*getc)();
	unsigned char (*kbhit)();
	unsigned char (*ioctl)(unsigned char num, void* parameter);
	unsigned char status;
} chardevice_t;



#endif /* DRIVERS_H_ */