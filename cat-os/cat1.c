/*
 * cat1.c
 *
 * Created: 6/1/2013 5:41:14 AM
 *  Author: Mark Sherman
 */ 


#include "avrstuff.h"
#include "drivers.h"
#include "misc.h"

/* Hardware specific */
#include "comm.h"
#include "xram.h"



//#include "keyps2.h"

#include "vga.h"

//#include "spi.h"
//#include "sdcard.h"
//#include "filesystem.h"
//#include "w5100.h"



#define CONFIG_SDCARD_BLOCKS  



/* Character devices */
chardevice_t  dev_comm = { comm_ioctl, 0,comm_putc, comm_getc, comm_kbhit};
chardevice_t* dmesg = NULL;  /* OS debug messages will be output here, if this is defined */

//chardevice_t  dev_key = {NULL, key_getc, key_kbhit, key_ioctl, 0};
/* Block devices*/
//blockdevice_t dev_sdcard = {sdcard_read_block, sdcard_write_block, sdcard_ioctl, 0};


/* Mount points */
//llfs_mountpoint_t mp_sdcard;

unsigned char buf[512];
unsigned char buf2[512];

void init(){

		int c=1;
	comm_ioctl_parms_t* commparm = (void*)scratch;
	
	/* Disable JTAG port to get full access on PORTC*/
	MCUCR |= _BV(JTD);
	MCUCR |= _BV(JTD);
	DDRA=0;    //all input
	PORTA=0xff;   //pullup

		
	/* Initialize serial port */
	
	commparm->baud=9600;
	commparm->enable_115200=0;
	if (dev_comm.ioctl(&dev_comm, COMM_IOCTL_INITBAUD, commparm)) {
		dmesg = &dev_comm;  /* Use serial port as dmesg out */
		DMESG("COMM OK\n\r");	
	}
	
	DMESG("XRAM");
	xram_init();
	vga_init();
	sei(); //enable interrupts
	vga_fast();
	clearscreen(c++);
	while(1) {
		unsigned int addr=123;
		unsigned int count=10;
	
		int i;
		
	//	clearscreen(c++);

			
		DMESG("start:");
		readline(&dev_comm, scratch, sizeof(scratch), 1);

		addr = atoi(scratch);
		if (addr==9999) {
			memset(buf, 0, sizeof(buf));
			DMESG("readstart:");
			readline(&dev_comm, scratch, sizeof(scratch), 1);
			addr = atoi(scratch);
			DMESG("count:");
			readline(&dev_comm, scratch, sizeof(scratch), 1);
			count = atoi(scratch);
			
			goto readonly;
		}
				
		DMESG("count:");
		readline(&dev_comm, scratch, sizeof(scratch), 1);
		count = atoi(scratch);
		
		for(i=0;i<count;i++){
			buf[i]=i^addr+random();
		}
		DMESG("copy to\r\n");
		
		memcpyi2x(addr, (void*) buf, count);
		
		readonly:
		
		DMESG("read in\r\n");
		
		
		memcpyx2i((void*)buf2, addr, count);		
		
		for (i=0;i<count;i++){
			
			snprintf(scratch, SCRATCH_SIZE, "%x: %x/%x %x\r\n", addr+i, buf[i], buf2[i], buf[i]^buf2[i]);
			prints(&dev_comm,  scratch );
			
		}
		c++;
		
	
	}
	
	
	
//	xram_test();
	
	
}
	



