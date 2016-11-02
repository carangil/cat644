/*
 * cat1.c
 *
 * Created: 6/1/2013 5:41:14 AM
 *  Author: Mark Sherman
 */ 



#include "avrstuff.h"
#include "drivers.h"

#include "comm.h"
#include "keyps2.h"
#include "xram.h"
#include "vga.h"

#include "misc.h"

#include "spi.h"
#include "sdcard.h"
#include "filesystem.h"
#include "w5100.h"

#define CONFIG_SDCARD_BLOCKS  

extern chardevice_t dev_multi;


/* Character devices */
chardevice_t  dev_comm = {comm_putc, comm_getc, comm_kbhit,  comm_ioctl, 0};

chardevice_t  dev_key = {NULL, key_getc, key_kbhit, key_ioctl, 0};

chardevice_t  dev_keyincommout = {comm_putc, key_getc, key_kbhit, key_ioctl, 0 };

/* Block devices*/
blockdevice_t dev_sdcard = {sdcard_read_block, sdcard_write_block, sdcard_ioctl, 0};


/* Mount points */
llfs_mountpoint_t mp_sdcard;


/* Create a composite character device reading from either keyboard or serial input */

uchar multi_kbhit(uchar minor){
	/* is either input device ready? */
	return dev_key.kbhit(0) || dev_comm.kbhit(0);	
}

uchar multi_getc(uchar minor) {
	/* return whichever returns a key first */
	while (1) {
		if (dev_key.kbhit(0))
			return dev_key.getc(0);
		
		if (dev_comm.kbhit(0))
			return dev_comm.getc(0);
	}
}

chardevice_t  dev_multi_input = {NULL, multi_getc, multi_kbhit, NULL, 0};



char str[50];  //scratchpad string

static unsigned char debug_on=1;

void debug(char* x)
{
	if (debug_on) {
		prints(&dev_comm, x);
		prints(&dev_comm, "\r\n");
	}
	
}


//ethernet
	#define ETHPORT PORTA
	#define ETHDDR	DDRA
	#define ETHCS	0b100
	
	//from internet
	void  W51_write(unsigned int  addr, unsigned char  data)
	{
		
		ETHPORT &= (~ETHCS);									// enable the W5100 chip
		
	//	_delay_ms(10);
		spi_inout(W5100_WRITE_OPCODE);					// need to write a byte
		spi_inout((addr & 0xff00) >> 8);				// send MSB of addr
		spi_inout(addr & 0xff);							// send LSB
		spi_inout(data);									// send the data
		ETHPORT |= ETHCS;									// done with the chip
	//	_delay_ms(10);
	}


	unsigned char  W51_read(unsigned int  addr)
	{
		unsigned char				val;

		ETHPORT &= (~ETHCS);
	//	_delay_ms(10);

		// enable the W5100 chip
		spi_inout(W5100_READ_OPCODE);					// need to read a byte
		spi_inout((addr & 0xff00) >> 8);				// send MSB of addr
		spi_inout(addr & 0xff);							// send LSB
	//	SPI_CPHA1; //switch phase
		val = spi_inout(0x00);							// need to send a dummy char to get response
		//SPI_CPHA0;
		ETHPORT |= ETHCS;									// done with the chip
	//	_delay_ms(10);
		return  val;								// tell her what she's won
	}
	//end internet
	char x[20];
	void eth_test ()
	{
		sdcard_disable();
			
//SPI_CPHA1; //switch phase
		ETHPORT |= ETHCS;  /*raise ethernet CS pin*/
		ETHDDR |= ETHCS;   /*ethernet CS pin is output */
		_delay_ms(100);
		
		prints(&dev_comm, "about to init\r\n");
		W51_write(W5100_MR, W5100_MR_SOFTRST);
		
		//AND CONFIG
		
		
		W51_write(W5100_GAR + 0, 192);	// set up the gateway address
		W51_write(W5100_GAR + 1, 168);
		W51_write(W5100_GAR + 2, 1);
		W51_write(W5100_GAR + 3, 254);
	//	_delay_ms(1);
		
		
		sprintf(x, "%d.%d.%d.%d\r\n", W51_read(W5100_GAR + 0),W51_read(W5100_GAR + 1) ,W51_read(W5100_GAR + 2) ,W51_read(W5100_GAR + 3)  );
		prints(&dev_comm, x);
		
		//_delay_ms(1);

		W51_write(W5100_SHAR + 0, 0x00);	// set up the MAC address
		W51_write(W5100_SHAR + 1, 0x16);
		W51_write(W5100_SHAR + 2, 0x36);
		W51_write(W5100_SHAR + 3, 0xde);
		W51_write(W5100_SHAR + 4, 0x58);
		W51_write(W5100_SHAR + 5, 0xf6);
	//	_delay_ms(1);

		W51_write(W5100_SUBR + 0, 255);	// set up the subnet mask
		W51_write(W5100_SUBR + 1, 255);
		W51_write(W5100_SUBR + 2, 255);
		W51_write(W5100_SUBR + 3, 0);
	//	_delay_ms(1);

		W51_write(W5100_SIPR + 0, 192);	// set up the source IP address
		W51_write(W5100_SIPR + 1, 168);
		W51_write(W5100_SIPR + 2, 1);
		W51_write(W5100_SIPR + 3, 252);
	//	_delay_ms(1);

		W51_write(W5100_RMSR, 0x55);					// use default buffer sizes (2K bytes RX and TX for each socket
		W51_write(W5100_TMSR, 0x55);
		

		while(1){
			
		//	sdcard_disable();  //make sure sdcard CS is high
			sprintf(x, "dis sd %d.%d.%d.%d  ", W51_read(W5100_GAR + 0),W51_read(W5100_GAR + 1) ,W51_read(W5100_GAR + 2) ,W51_read(W5100_GAR + 3)  );
			prints(&dev_comm, x);

			sprintf(x, "dis sd %d.%d.%d.%d  ", W51_read(W5100_SIPR + 0),W51_read(W5100_SIPR + 1) ,W51_read(W5100_SIPR + 2) ,W51_read(W5100_SIPR + 3)  );
			prints(&dev_comm, x);
	

			sprintf(x, "dis sd %d.%d.%d.%d\r\n", W51_read(W5100_SUBR + 0),W51_read(W5100_SUBR + 1) ,W51_read(W5100_SUBR + 2) ,W51_read(W5100_SUBR + 3)  );
			prints(&dev_comm, x);
			
		}
		
	}


void delay100(){
	char i;
	for (i=0;i<10;i++){
		_delay_ms(10);
	}
	
}

void eth_pre() {
	
	ETHPORT |= ETHCS;  /*raise ethernet CS pin*/
	ETHDDR |= ETHCS;   /*ethernet CS pin is output */
	delay100();
	while(1){
		ETHPORT &= ~ETHCS; //low ethernet pin
		delay100();	
		ETHPORT |= ETHCS;   /*ethernet CS pin is output */
		delay100();	
		prints(&dev_comm, "alive ");	
	}
	
}

//end ethernet

int main(void)
{
		char dpage=0;
		unsigned char i=0;
		unsigned char j=0;
		unsigned long capacity = 0;
		
		comm_ioctl_parms_t commparm = {38400,0};
		hires=0;
		/* Disable JTAG port */
		MCUCR |= _BV(JTD);
		MCUCR |= _BV(JTD);
	
	
		DDRA=0;    //all input
		PORTA=0xff;   //pullup
	
	
		
	
		xram_init();  //enable external ram
		
		dev_comm.ioctl(0, COMM_IOCTL_INITBAUD, &commparm);
		dev_key.ioctl(0, KEY_IOCTL_INIT, NULL);		
		dev_key.ioctl(0, KEY_IOCTL_INTERRUPT_DISABLE, NULL); /*disable keyboard interrupt */
			prints(&dev_comm, "comminit\r\n");
		
//	spi_init(SPI_MASTER | SPI_CLK_8);
	//	spi_enable();
	//	spi_disable();
		//sdcard_enable();
		/*
		while (1){
			_delay_ms(100);
			PORTB = 0x0B;
			_delay_ms(100);	
			PORTB=0;
			
		}
		*/
		
		
		//sdcard_disable();  //make sure sdcard CS is high
			SDCARD_CS_PORT |=  SDCARD_CS;
			SDCARD_CS_DDR |= SDCARD_CS;
		
				ETHDDR |= ETHCS;
				ETHPORT |= ETHCS;
				
				
						
		
#if 	1
	vga_init();
		sei(); //enable interrupts
	
	
		VGA_DISPLAY(0);
		clearscreen(RED);
		vga_delay(60); //wait 60 frames		
		clearscreen(GREEN);
		vga_delay(60); //wait 60 frames
		clearscreen(BLUE);
		vga_slow();
		drawchar(20,20, 'H', RED, BLUE);
		drawchar(30,25, 'i', RED, BLUE);
		//eth_pre();//check pin
		_delay_ms(10);
		
		prints(&dev_comm, "Test\r\n");
		
		
	#endif	
		spi_init(SPI_MASTER | SPI_CLK_128);
		spi_enable();
	//	eth_test();
		
		dev_sdcard.ioctl(0, SDCARD_IOCTL_INIT, &capacity);
		
		sprintf(str, "SDsz:%ld", capacity); debug(str);
		
		spi_init(SPI_MASTER | SPI_CLK_2);
		//spi_init(SPI_MASTER | SPI_CLK_8);
						
		if (MOUNT_SUCCESS == llfs_mount(&mp_sdcard, &dev_sdcard)){
			debug("SDmntS");
		
			
		//	llfs_deformat(&dev_sdcard);
			
		} else {
			
			debug("SDmntF?");
		
			if (dev_multi_input.getc(0)=='Y') 
			{
				if (llfs_format(&dev_sdcard, capacity, 0x10)) 
					debug ("FORMAT SUCCESS\n");			
			}
			else
				debug ("SDfFAIL\n");
		}
		
		//spi_init(SPI_MASTER | SPI_CLK_2);
		spi_enable();
			//	spi_init(SPI_MASTER | SPI_CLK_64);
				spi_init(SPI_MASTER | SPI_CLK_32);
		eth_test();
		
		SELECT_RAM_BANK(0);
		clearscreen(0);
		SELECT_RAM_BANK(1);
		
		
		clearscreen(0);	
		
			
		{
			
			char x=0;
			char y=0;
			char pos[] = {0,0};
			char dir=1;
			char scr=0;
			char cposr[]={100,100};
				
			char cpos=100;
					
			while(1) {
				
				VGA_DISPLAY(scr); //show 1 page
	
				SELECT_RAM_BANK(scr^1); //draw on other page;

							
				//undraw what we drew on this page last time
				for (x=30+pos[scr^1];x<100+pos[scr^1];x+=12) {
					for (y=10;y<80;y+=12) {
						drawchar(x,y, ' ', BLACK, BLACK);
						
						
					}
				}
				
				//undraw player position
				
				drawchar(cposr[scr^1], 210, '^', BLACK, TRANSPARENT);
				
				
				//incrment positions
				pos[scr^1] = pos[scr]+dir;
				
				if(pos[scr^1] > 100)
					dir=-1;
					
				if (pos[scr^1] < 1)
					dir = 1;
				
				while(dev_multi_input.kbhit(0)) {
					char c = dev_multi_input.getc(0);
					if (c)
						dev_comm.putc(0,c);
					if (c =='a'	)
						cpos-=5;
						
					if (c =='d'	)
						cpos+=5;



					if (c =='x'	) {
						debug("press q to exit keyboard test:");
						while(c!='q') {
							c = dev_multi_input.getc(0);
							if(c)
								dev_comm.putc(0,c);
						}
							
					}					
				}
							
				//draw new stuff
				
				for (x=30+pos[scr^1];x<100+pos[scr^1];x+=12) {
					for (y=10;y<80;y+=12) {
						drawchar(x,y,0xea, YELLOW, TRANSPARENT);
						drawchar(x,y, '^' , RED, TRANSPARENT);	
					}
				}
				
				//draw player position
				cposr[scr^1] = cpos;
				drawchar(cpos, 210, '^', GREEN, TRANSPARENT);
				
					
				scr ^=1; //flip roles
				
			}
			
			
		}
			
		
}
