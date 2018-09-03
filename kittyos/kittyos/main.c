/*
 * kittyos.c
 *
 * Created: 9/2/2018 10:16:59 PM
 * Author : mark
 */ 

#include "kittyos.h"
#include "comm.h"


//way to output messages to console
#define CONFIG_DMESG_SIZE	32
char dmesg_buf[CONFIG_DMESG_SIZE];
#define DMESGF(...) { snprintf(dmesg_buf, sizeof(dmesg_buf), __VA_ARGS__ ); prints(dev_dmesg, dmesg_buf);}
#define DMESG(x) prints(dev_dmesg, x)
chardevice_t* dev_dmesg = NULL;

//simple i/o

void prints(chardevice_t* dev, char* s){
	
	if (!dev)
		return;
		
	if (dev->dev.flags & DEV_FLAG_UNINIT)
		return;
	
	if (!s)	
		printf("Null");
		
	while(*s) 
		dev->write1(dev, *(s++));
}



uint16_t reads( chardevice_t* dev, char* str, uint16_t buffersize, unsigned char echo) {
	uint8_t c=0;
	uint16_t cnt = 1; //1 instead if 0 to reserve space for terminator
	
	while ((cnt < buffersize))
	{
		c = dev->read1(dev);
		
		DMESGF("{recv %d}", c);
		
		if(echo) {
			dev->write1(dev,c); //echo it	
		}
		
		if (c=='\n')
			break; //done
			
		if (c=='\r')
			break; //done
		
		*(str++) = c;
		cnt++;
	}
	
	*str = 0; //null terminate
	return cnt;
}



//configure serial port
#define CONFIG_SER0_BAUD	9600

int main(void)
{	
	static char buf[50];
	
	
   	/* Disable JTAG port to get full access on PORTC*/
   	MCUCR |= _BV(JTD);
   	MCUCR |= _BV(JTD);
   	DDRA=0;    //all input
   	PORTA=0xff;   //pullup
   
  
   //initialize serial port
  
   DMESG("Kernel start\n");  //testing that unitialized console does nothing bad
  
   if (SUCCESS == dev_ser0.dev.ioctl(&dev_ser0.dev, IOCTL_BAUD, CONFIG_SER0_BAUD)) {
	   
	   // if we initalized serial port, that becomes the output console
	   dev_dmesg = &dev_ser0;
	   DMESGF("SER0 at %d", CONFIG_SER0_BAUD);
   }
   


   	environment_t env;  	
   	env.cmdline= " testing";
   	env.in = &dev_ser0;
   	env.out = &dev_ser0;
	   
    DMESGF("Prog returned %d\nHLT\n", test_main(&env));
         
   while(1){}
   
  
}


