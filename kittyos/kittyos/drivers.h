/*
 * drivers.h
 *
 * Created: 7/14/2014 8:07:59 PM
 *  Author: mark
 */ 
#ifndef DRIVERS_H_
#define DRIVERS_H_
#include "avrstuff.h"

//return values for calls
#define SUCCESS		0
#define ERR			1

//device flag
#define DEV_FLAG_ERR	1
#define DEV_FLAG_UNINIT	2

typedef  struct device_s* devid;

typedef struct device_s {
	uchar (*ioctl)(devid dev, uchar ctlnum, uint16_t param);
	uchar flags;
} device_t;

//serial port, files, keyboard, etc

typedef struct chardevice_s {
	device_t dev;
	void (*write1)(struct chardevice_s* dev, uchar ch);
	uchar (*read1)(struct chardevice_s* dev);
	uchar (*ready)(struct chardevice_s* dev);
} chardevice_t;

//disk 

typedef uint32_t  blocknum;
typedef struct blockdevice_s {
	device_t dev;
	uchar (*readblock)(struct blockdevice_s *dev, blocknum block,  char * data, int numbytes);
	uchar (*writeblock)(struct blockdevice_s *dev, blocknum block, char * data, int numbytes);
} blockdevice_t;

//possible ioctls so far
#define IOCTL_BAUD	1


//define a program as an executable device?
typedef struct exedevice_s{
	device_t dev;
	void* sram;
} exe_t;
#define IOCTL_EXE	2



#endif /* DRIVERS_H_ */