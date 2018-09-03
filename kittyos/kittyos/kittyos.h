/*
 * kittyos.h
 *
 * Created: 9/3/2018 12:54:56 AM
 *  Author: mark
 */ 


#ifndef KITTYOS_H_
#define KITTYOS_H_
#include "drivers.h"

typedef struct environment_s {
	chardevice_t* in;
	chardevice_t* out;
	char* cmdline; //any parameters given to program?
}environment_t;


void prints(chardevice_t* dev, char* s);
uint16_t reads( chardevice_t* dev, char* str, uint16_t buffersize, unsigned char echo);


#endif /* KITTYOS_H_ */