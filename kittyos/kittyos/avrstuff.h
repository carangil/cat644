/*
 * avrstuff.h
 *
 * Created: 7/14/2014 8:02:41 PM
 *  Author: mark
 */ 


#ifndef AVRSTUFF_H_
#define AVRSTUFF_H_

#define F_CPU 20000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdio.h>

void debug(char* x);
typedef unsigned char uchar;
typedef unsigned int u16;


#endif /* AVRSTUFF_H_ */
