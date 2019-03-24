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


void xalloc_init(unsigned int heap_start, unsigned int heap_end);
void xdump();
unsigned int xalloc(unsigned int);


int main(void)
{	
//	static char buf[50];
	

	
   	/* Disable JTAG port to get full access on PORTC*/
   	MCUCR |= _BV(JTD);
   	MCUCR |= _BV(JTD);
   	DDRA=0;    //all input
   	PORTA=0xff;   //pullup

 
   //initialize serial port
  
   DMESG("Kernel start\r\n");  //testing that unitialized console does nothing bad
  

  
   if (SUCCESS == dev_ser0.dev.ioctl(&dev_ser0.dev, IOCTL_BAUD, CONFIG_SER0_BAUD)) {
	   // if we initalized serial port, that becomes the output console
	   dev_dmesg = &dev_ser0;
	   DMESGF("DMESG=SER0 at %d\r\n", CONFIG_SER0_BAUD);
   }
     
	DMESG("Hello in Kernel.\r\n");
	
	DMESGF("instr0:%x\r\n", (unsigned int) instr0);	

	xram_init();  
	vga_init();
	sei();
		
	//switch output to screen, using the device syscall
	DMESG("Attempt to find 'scr'\r\n");
	dev_dmesg = findDevice("scr");

	
	clearscreen(GREEN);
	dev_keyraw.dev.ioctl(&dev_keyraw.dev, IOCTL_ENABLE, 1);

	DMESG("DMESG=scr\r\n");
	DMESG("VER 0.2 "  __DATE__ " " __TIME__  " \r\n");

dev_dmesg = &dev_ser0;
	mminit();
	//mmdump();
	mmalloc(200);
	void* v1  = mmalloc_handle(200, 0x55);
	void* v2 = mmalloc_handle(230, 0x1033);
	void* v3 = mmalloc_handle(512, 0x5556);
	//mmdump();
	DMESGF("H %x %x %x\r\n", mmgethandle(v1), mmgethandle(v2) ,  mmgethandle(v3)  );
	
	DMESGF("P %p %p %p\r\n", v1,v2,v3);
	
	DMESGF("P %p %p %p\r\n", mmfindhandle(0x55) ,mmfindhandle(0x1033),mmfindhandle(0x5556));
	

	
    xalloc_init(32768,65535);
	//xdump();
	
	//xalloc(100);
	char* s = "This is a string";
	
	char * hp;
	char * hp2;
	
	u16 h = halloc(strlen(s)+1);
	hp = hgrab(h);
	hp2 = hgrab(h); //get again
	DMESGF("hp %p  hp2 %p\r\n", hp, hp2);
	
	strcpy(hp, s);  //copy it
	
	hrelease(hp);
	hrelease(hp2);
	
	char* other = mmalloc(10); //take up some memory (should overhap old hp)
	DMESGF("other %p\r\n", other);
			
	hp = hgrab(h);
	DMESGF("hp %x %s\r\n",hp,hp);
	
	
	#if 1
	{
		deviceinfo_t di;
		if ( mainmux.first(&mainmux, &di) ){
			
			do {
				DMESGF("DEVICE %s=T%x S%x,\n", di.name, di.devtype, di.selector);
			} while (mainmux.next(&mainmux, &di));
		}
				
		
	}
	
	DMESG("--\n");
	#endif
	   	environment_t env;
	   	env.in = &dev_keychar;
	   	env.out = &dev_scr;
	   	//env.cmdline= " testing";
	   	//env.in = &dev_ser0;
	   	//env.out = &dev_ser0;


	
	//interpreter never returns
	DMESGF("Start 16:$%x\n", (unsigned int) program);
	interpreter(program, vstack + sizeof(vstack), NULL);
	
	//dev_spi0.chardev.dev.ioctl(&dev_spi0.chardev.dev, IOCTL_LOCK, SPI_MASTER | SPI_CLK_8 );
	
	//{
		//unsigned long cap;
		//sdcard_init(&cap);
		//DMESGF("SD:%ld\n", cap);
	//}
	
	
	
	//dev_spi0.chardev.write1(NULL, 'A');
	//while(1);
	
	 

   while(1){
	   
	   vscroll++;
	   hscroll-=2;
	   vga_delay(20);
	   
   }
   
  
}


