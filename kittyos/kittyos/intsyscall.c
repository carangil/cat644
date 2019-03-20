#include "kittyos.h"
#include "interpreter.h"

#include "intsyscalls.h"

volatile int x;
volatile char xc;

//uchar staticheap[1024*3];

unsigned int syscall(unsigned int arg0, unsigned char callnum, unsigned int arg1)
{
	chardevice_t* cdev;
	
//	DMESGF("SYSCALL %x arg0 %x  arg1 %x\n", (unsigned int) callnum, arg0, arg1);
	
	#if 1
	
		switch (callnum){
			
	//		case SYSCALL_GET_HEAP:
		//		return (unsigned int) staticheap;
				
			case SYSCALL_READ1:
				cdev = (void*) arg1;
				return cdev->read1(cdev);
				
			case SYSCALL_WRITE1:
				cdev = (void*) arg1;
				cdev->write1(cdev,(unsigned char) arg0);
				return SUCCESS;
				
			case SYSCALL_FIND_DEV:
				return findDevice((char*) arg0);
			
		
			
			
			
		}
	#endif

		
	return arg0;  //always return register A, unless other value is wanted
}

