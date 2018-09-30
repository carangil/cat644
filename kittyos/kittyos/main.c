/*
 * kittyos.c
 *
 * Created: 9/2/2018 10:16:59 PM
 * Author : mark
 */ 

#include "kittyos.h"
#include "comm.h"
#include "vga.h"
#include "keyps2.h"
#include "spi.h"
#include "sdcard.h"



//way to output messages to console

char dmesg_buf[CONFIG_DMESG_SIZE];

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

//The main system mux, which lets the user select all other devices
//also tests the mux enumerate & select subdevice concept, which will be used later to build directories on disk
char* devlistnames[]={"ser0", "spi0", "ps2", "key", "scr",  NULL};	
device_t* devlist []={&dev_ser0.dev, &dev_spi0.chardev.dev, &dev_keyraw.dev, &dev_keychar.dev, &dev_scr.dev };
uchar devtypes[] = { DEVICE_CHAR, DEVICE_CHAR, DEVICE_CHAR, DEVICE_CHAR, DEVICE_CHAR};

device_t* main_select(muxdevice_t* mux, uint16_t subdevice){
	return devlist[subdevice];	
}

uchar main_next(muxdevice_t* mux, deviceinfo_t* info){
	info->selector++;
	if (devlistnames[info->selector] == NULL)
		return 0;  //no more devices
	
	strncpy(&info->name, devlistnames[info->selector], sizeof(info->name));
	info->devtype = devtypes[info->selector];
	return 1; //got a device
}

uchar main_first(muxdevice_t* mux, deviceinfo_t* info){
	info->selector=0xffff;
	return main_next(mux,info);
	
}

uchar (*next)(struct muxdevice_s* mux, deviceinfo_t* info);

muxdevice_t mainmux = { {NULL,0}, main_select, main_first, main_next};
	
	
//findDevice helper function
//takes a name and find it in the main mux


device_t * findDevice(char* name){
	
	muxdevice_t* m = &mainmux;
	deviceinfo_t info;
	deviceinfo_t di;
	
	if ( m->first(m, &di) ){
		
		do {
			if (!strcmp(di.name, name)) {
				return m->select(m, di.selector);
			}
			
		} while (m->next(m, &di));
	
	}
	
	return NULL;
}



//configure serial port
#define CONFIG_SER0_BAUD	9600

extern instr0();


unsigned char vstack[100];
extern char program[];

int main(void)
{	
//	static char buf[50];
	
	
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
	   DMESGF("SER0 at %d\n", CONFIG_SER0_BAUD);
   }
     //try outputing on a port

	xram_init();  
	vga_init();
	sei();
		
	//switch output to screen, using the device syscall
	
	dev_dmesg = findDevice("scr");

	
	clearscreen(GREEN);
	dev_keyraw.dev.ioctl(&dev_keyraw.dev, IOCTL_ENABLE, 1);


	

	DMESG("Debug to screen.");
	
	
	DMESGF("instr0:%x\n", (unsigned int) instr0);	
	
	{
		deviceinfo_t di;
		if ( mainmux.first(&mainmux, &di) ){
			
			do {
				DMESGF("DEVICE %s=T%x S%x,\n", di.name, di.devtype, di.selector);
			} while (mainmux.next(&mainmux, &di));
		}
		
		
		
	}
	DMESG("--\n");
	
	   	environment_t env;
	   	env.in = &dev_keychar;
	   	env.out = &dev_scr;
	   	//env.cmdline= " testing";
	   	//env.in = &dev_ser0;
	   	//env.out = &dev_ser0;


	
	
	interpreter(program, vstack + sizeof(vstack), NULL);
	
	//dev_spi0.chardev.dev.ioctl(&dev_spi0.chardev.dev, IOCTL_LOCK, SPI_MASTER | SPI_CLK_8 );
	
	//{
		//unsigned long cap;
		//sdcard_init(&cap);
		//DMESGF("SD:%ld\n", cap);
	//}
	
	
	
	//dev_spi0.chardev.write1(NULL, 'A');
	//while(1);
	


	
	//  vga_slow();
	 //..SELECT_RAM_PAGE(0x123);
	 //SELECT_RAM_BANK(0);
	 
	 clearscreen(GREEN);
	 drawchar(0,0,'x',RED,BLUE);;
	 drawchar(256-8,200,'y',RED,BLUE);
	 drawchar(256-16,100,'a',RED,BLUE);
		
	DMESG("Start user program\n");
    DMESGF("Prog returned %d\nHLT\n", test_main(&env));
		
   vga_fast();
   
   while(1){
	   
	   vscroll++;
	   hscroll-=2;
	   vga_delay(20);
	   
   }
   
  
}


