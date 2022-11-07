/*
 * kittyos.h
 *
 * Created: 9/3/2018 12:54:56 AM
 *  Author: mark
 */ 


#ifndef KITTYOS_H_
#define KITTYOS_H_
#include "drivers.h"
#include "xram.h"
#include "mmalloc.h"

//configure serial port
#define CONFIG_SER0_BAUD	9600

typedef struct environment_s {
	chardevice_t* in;
	chardevice_t* out;
	
	chardevice_t* key;  //stick raw keyboard here until we have the device list setup
	
	//char* cmdline; //any parameters given to program?
}environment_t;


device_t* findDevice(char* name);

void prints(chardevice_t* dev, char* s);
uint16_t reads( chardevice_t* dev, char* str, uint16_t buffersize, unsigned char echo);

extern chardevice_t dev_keyraw;

//Not a typedef... don't want to have to cast between u16 and handles... clutters the code a bit
#define handle_t u16

handle_t halloc(u16 bytes);
void* hgrab(u16 handle_t);
handle_t hrelease(void* p);







#define CONFIG_DMESG_SIZE	128
extern char dmesg_buf[CONFIG_DMESG_SIZE];
#define DMESGF(...) { snprintf(dmesg_buf, sizeof(dmesg_buf), __VA_ARGS__ ); prints(dev_dmesg, dmesg_buf);}
#define DMESG(x) prints(dev_dmesg, x)
extern chardevice_t* dev_dmesg;

#endif /* KITTYOS_H_ */