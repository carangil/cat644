/*
 * misc.h
 *
 * Created: 7/14/2014 8:23:35 PM
 *  Author: mark
 */ 


#ifndef MISC_H_
#define MISC_H_

#include "drivers.h"
#define SCRATCH_SIZE 64

void prints(chardevice_t* dev, char* str);
void readline(chardevice_t* dev, char* str, uint16_t buffersize, unsigned char echo);

extern char scratch[SCRATCH_SIZE];  //scratchpad string


#define DMESG(xxx) printsP(dmesg, PSTR( xxx ) )


#endif /* MISC_H_ */