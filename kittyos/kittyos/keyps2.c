#include "avrstuff.h"
#include "keycodes.h"
#include "kittyos.h"
#include "keyps2.h"


volatile unsigned char keybuffer[ 0x10 ];
volatile unsigned char keyreadpos=0;
volatile unsigned char keywritepos=0;

extern chardevice_t dev_keyraw;


uchar key_ioctl(device_t* dev, uchar ctlnum, uint16_t param)
{
	
	if (ctlnum == IOCTL_ENABLE) {
		KEY_DDR &= KEY_MASK;  //make clk and data pins inputs
		KEY_PORT |= ~KEY_MASK;  //make clk and data pullup
		PCMSK0 |= KEY_CLK; //key ps2 causes pin change interrupt interrupt flag (pc7 only)
		PCIFR = 1 ; //clear the flag
		//EIMSK is kept disabled: don't enable the actual interrupts, just set the flags
		dev_keyraw.dev.flags = 0;  //device is inited
		keyreadpos=0;
		keywritepos=0;
	}
	return SUCCESS;
	
}

uchar key_getcode(chardevice_t* dev)
{
	unsigned char c;
	while (keyreadpos == keywritepos); //wait   while empty
	c = keybuffer[keyreadpos];
	keyreadpos = (keyreadpos+1) & 0xF;
	return c;
}

unsigned char key_kbhit(chardevice_t* dev)
{
	return (keyreadpos != keywritepos);
}

//create the chardevice
chardevice_t dev_keyraw = {{key_ioctl, DEV_FLAG_UNINIT}, NULL, key_getcode, key_kbhit };
	
//translation into characters
//xtab must be in prog memory

char xlat(const unsigned char* xtab, unsigned char code)
{

	unsigned char i;
	unsigned char j;
	
	for (i=0; (j = pgm_read_byte(xtab +i +1)); i+=2)
	{
		if (code == j)
		{
			return pgm_read_byte(xtab+i);
		}
	}
	return 0;
}

static uchar shift=0;
static uchar release=0;
static uchar e0=0;
static uchar nextchar=0;

void interp(uchar code){
	uchar c = 0;
	uchar d = 0;
	DMESG("{");
		
	if (code == 0xe0){
		e0=1;
		DMESG("SETE0,");
		return;
	}
		
	if (code == 0xf0) {
		DMESG("SETRELEASE,");
		release=1;	
		return;
	}
	
	
	if (e0){
	
		c = xlat(ktab_eo, code);
		DMESGF("xlateo %x->%x", code,c);
	}
	else {
		c = xlat(ktab, code);
		DMESGF("xlat %x->%x", code,c);
	}


	if (c==K_SHIFT) {
		if (release){
			DMESG("UNSHIFT,");
			shift=0;  //handle letting go of shift key
		}
		else {
			DMESG("SHIFT,");
			shift=1;  //handle pressing shift key
		}
	
		release=0;
		e0=0;
		DMESG("unRELEASE0,unE0");
		return;
	}

	DMESG("nE0");
	e0=0;

	if (release) {
		DMESG("ISREL,unREL");
		release=0;
		return;  //release codes don't generate characters
	}
	release=0;
	

	if (c==0)
		return;



	//generate capitals
	if (shift) {
		DMESG("isshift,");
		if ((c>='a') && (c<='z'))	{
			DMESGF("alpha %c,", c);
			c=c-'a'+'A';
		} else {
			d = xlat(ktab_shift, c); //shift the key
			DMESGF("shifted %c->%c", c, d);
			if(d)
				c=d;
		}
	}
		
	DMESGF("out %c}", c);
	nextchar=c;
		
}



uchar key_getchar(chardevice_t* dev){

	uchar c;

	while(1){
		
		if (nextchar)	 {
			c = nextchar;
			nextchar=0;
			return c;
		}
		
		//get and interpret next keycode
		interp(key_getcode(NULL));
		
	}
}

uchar key_charkbhit(chardevice_t* dev){

	while(1){

		//if we have a character, then its ready
		if (nextchar){
			return 1;
		}
			
		//if we have a keycode interpret it
		if (key_kbhit(NULL))
			interp(key_getcode(NULL));
		else
			return 0;
		
	}

}

chardevice_t dev_keychar = {{NULL, 0}, NULL, key_getchar, key_charkbhit };