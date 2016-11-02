/*
 * misc.h
 *
 * Created: 7/14/2014 8:23:35 PM
 *  Author: mark
 */ 


#ifndef MISC_H_
#define MISC_H_

#include "drivers.h"

void prints( chardevice_t* dev, char* str);
void readline( chardevice_t* dev, char* str, uint16_t buffersize, unsigned char echo);



#endif /* MISC_H_ */