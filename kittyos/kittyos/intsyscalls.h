/*
 * intsyscalls.h
 *
 * Created: 9/29/2018 4:31:39 PM
 *  Author: mark
 */ 


#ifndef INTSYSCALLS_H_
#define INTSYSCALLS_H_

unsigned int syscall(unsigned int arg0, unsigned char callnum, unsigned int arg1);

#define SYSCALL_FIND_DEV	1
#define SYSCALL_WRITE1		2
#define SYSCALL_READ1		3
#define SYSCALL_GET_HEAP	4


#endif /* INTSYSCALLS_H_ */