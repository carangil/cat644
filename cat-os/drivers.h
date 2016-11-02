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


/* read/write function rules:
	if BUFFER is null, read returns 1 byte, as its return value
	if BUFFER is non-null, up to 'count' ready bytes are read into buffer.  return value is number of bytes read
	
	if BUFFER is null, write writes 1 byte (the value of countchar).  return value is '1' if successful
	if BUFFER is non-null, write will write up to 'n' bytes
*/


#define CHARDEV_FLAG_ERR			1  /* device is in error state */

typedef  void* devid;


typedef struct device_s {
	uchar (*ioctl)(devid dev, uchar ctlnum, void* parameter);
	uchar flags;
} device_t;


typedef struct chardevice_s {
	uchar (*ioctl)(devid dev, uchar ctlnum, void* parameter);
	uchar flags;
	void (*putc)(devid dev, uchar ch);
	uchar (*getc)(devid dev);
	uchar (*kbhit)(devid dev);
} chardevice_t;


typedef uint32_t  blocknum;
typedef struct blockdevice_s {
	uchar (*ioctl)(devid dev, uchar ctlnum, void* parameter);
	uchar flags;
	uchar (*readblock)(devid dev, blocknum block,   char * data, int numbytes);
	uchar (*writeblock)(devid dev, blocknum block,  char * data, int numbytes);
} blockdevice_t;




#define INVALIDdevnum 0xFF

#endif /* DRIVERS_H_ */