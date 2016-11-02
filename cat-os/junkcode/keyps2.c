/*
 * keyps2.c
 *
 * Created: 7/14/2014 8:48:10 PM
 *  Author: mark
 */ 

#include <avr/interrupt.h>
#include "drivers.h"
#include "keyps2.h"

#include "comm.h"

#include "misc.h"

void key_init()
{
	
	KEY_DDR &= KEY_MASK;  //keyboard are inputs
	KEY_PORT |= KEY_CLK |KEY_DATA ; //pullup
	
	//enable interrupt
	PCICR = (1 << PCIE0);  //enable pin change interrupt
	PCMSK0 = (1<<PCINT7);	 //enable PCINT7	
	

}




volatile char keyb_readpos=0;  //written by reader
volatile char keyb_writepos=0; //written by driver
volatile char keyb[KEYB_SIZE];
volatile char keybits=0;
volatile char keybitcnt=0;


unsigned char nextkey(){
	unsigned char x;
	
	//sprintf(str, "[%d:%d]", keyb_readpos, keyb_writepos);
	//debug(str);
	
	if (keyb_writepos == keyb_readpos)
		return 0;
	
	x = keyb[keyb_readpos++];
	if (keyb_readpos >= KEYB_SIZE)
		keyb_readpos = 0;
	
	return x;
}

char shiftstate=0;
int releasestate=0;

extern unsigned char ktab[];
extern unsigned char ktab_shift[];

//translate keycode into character
char xlat(unsigned char* xtab, unsigned char code)
{

	char i;
	
	for (i=0; ktab[i]; i+=2)
	{
		if (code == xtab[i+1])
		{
			return xtab[i];
		}
	}
	return 0;
}



unsigned char keypushback=0;

unsigned char nextkeychar(){
	
	unsigned char ch=0;
	unsigned char chs=0;
	unsigned char code;

	if (keypushback) {
		//if we alrady have a character, return it
		ch = keypushback;
		keypushback = 0;
		return ch;
	}
	
	//get next keycode
	
	code = nextkey();
	
	if (!code)
		return 0;  //no keys pressed
	
	if (code ==0xf0)	{
		//key released
		//next code returned will be key's code
		releasestate=1;
		return 0;
	}
	
	ch = xlat(ktab, code);

	if (!releasestate) //not releasing (so, a keypress)
	{
		
		if (ch == K_SHIFT)
		{
			shiftstate=1;
			//all further characters will be shifted
			return 0;  //no char yet
		}

		if (shiftstate){
			
			//if shifted, make a lowercase capital,
			//or look up other characters by table
			
			if ((ch>='a') && (ch <='z'))
			chs = ch-'a'+'A';
			else
			chs = xlat(ktab_shift, ch);
		}
		
		//if a valid character shifted , return the char
		if (chs)
		return chs;
		
		//otherwise return the orginal looked-up char
		return ch;
	}
	
	//following is for is a key was released
	
	if (ch==K_SHIFT)
	shiftstate=0;  //stops shifting characters
	
	releasestate=0;
	
	return 0;
}

//blocks until a key is eaten
unsigned char key_getc(uchar minor){
	
	unsigned char x=0;
	
	while(!x)
	{
		x = nextkeychar();
		
	}
	
	return x;
}

//check if key available
unsigned char key_kbhit(uchar minor) {
	
	unsigned char x = nextkeychar();
	
	if (x) {
		keypushback = x;
		return 1;
	}
	
	return 0;
}

unsigned char key_ioctl(uchar minor, unsigned char num, void* parm)
{
	if (num == KEY_IOCTL_INIT)
	{
		key_init();
	}
	else if(num == KEY_IOCTL_INTERRUPT_ENABLE)
	{
			PCICR |= (1 << PCIE0);  //enable pin change interrupt
	}
	else if (num == KEY_IOCTL_INTERRUPT_DISABLE)
	{
			PCICR &= ~(1 << PCIE0);  //enable pin change interrupt
		
	}
	else
		return IOCTL_ERR;
	
	return IOCTL_SUCCESS;
	
}



ISR (PCINT0_vect)
{

	
	if (!(KEY_PIN & KEY_CLK))
	{
		char inbit = (KEY_PIN&KEY_DATA)<<1;
		
	
//		debugkeys[debugkeypos]=inbit;
	//	debugkeypos++;
		//debugkeypos = debugkeypos % 11;
		
		
		//this was a falling edge
		
		if (keybitcnt==0)
		{
			if (inbit==0)
				keybitcnt++;
		}
		else if (keybitcnt<9)
		{
			keybits = keybits>>1;
			keybits |= inbit;
			keybitcnt++;
		}
		else if (keybitcnt==9)
			keybitcnt++;
		else { //cnt is 10 (11th bit of frame)
			if (inbit==128)  
			{
				keyb[keyb_writepos] = keybits;  //write in buffer
				if (keyb_writepos==(KEYB_SIZE-1))
					keyb_writepos=0;
				else
					keyb_writepos++;
				
			}
			
			keybitcnt=0;
		}
		
	}
	
}