/*
 * drivers.h
 *
 * Created: 7/14/2014 8:07:59 PM
 *  Author: mark
 */ 
#ifndef DRIVERS_H_
#define DRIVERS_H_

#ifndef PCTEST
#include "avrstuff.h"
#endif

//return values for calls
#define SUCCESS		0
#define ERR			1

//device flag
#define DEV_FLAG_ERR	1
#define DEV_FLAG_UNINIT	2
#define DEV_FLAG_PUSHBACK	4  



typedef struct device_s {
	uchar (*ioctl)(struct device_s* dev, uchar ctlnum, uint16_t param);
	uchar flags;
} device_t;

//serial port, files, keyboard, etc

typedef struct chardevice_s {
	device_t dev;
	void (*write1)(struct chardevice_s* dev, uchar ch);
	uchar (*read1)(struct chardevice_s* dev);
	uchar (*ready)(struct chardevice_s* dev);
} chardevice_t;


typedef uint32_t  blocknum;
typedef struct blockdevice_s {
	device_t dev;
	uchar (*readblock)(struct blockdevice_s *dev, blocknum block,  char * data, int numbytes);
	uchar (*writeblock)(struct blockdevice_s *dev, blocknum block, char * data, int numbytes);
	blocknum count; //number of blocks
} blockdevice_t;

//possible ioctls so far
#define IOCTL_BAUD	1


//define a program as an executable device?
typedef struct exedevice_s{
	device_t dev;
	void* sram;
} exe_t;
#define IOCTL_EXE	2


#define IOCTL_ENABLE 3

#define IOCTL_UNLOCK 4
#define IOCTL_LOCK 5
#define IOCTL_INIT 6


#define DEVICE_BASE		0
#define DEVICE_CHAR		1
#define DEVICE_BLOCK	2
#define DEVICE_MUX		3

typedef struct deviceinfo_s{
	char name[29];
	uchar devtype;
	uint16_t selector;
}deviceinfo_t;

typedef struct muxdevice_s {
	device_t dev;
	device_t* (*select)(struct muxdevice_s* mux, uint16_t subdevice);			//selects subdevice by number
	uchar (*first)(struct muxdevice_s* mux, deviceinfo_t* info);
	uchar (*next)(struct muxdevice_s* mux, deviceinfo_t* info);
} muxdevice_t;


#endif /* DRIVERS_H_ */
