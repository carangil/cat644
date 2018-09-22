/*
 * keycodes.c
 *
 * Created: 4/29/2014 7:53:28 PM
 *  Author: mark
 */ 
#include "avrstuff.h"
#include "keycodes.h"


unsigned const char ktab[] PROGMEM = 
{
	'a',0x1C,
	'b',0x32,
	'c',0x21,
	'd',0x23,
	'e',0x24,
	'f',0x2B,
	'g',0x34,
	'h',0x33,
	'i',0x43,
	'j',0x3B,
	'k',0x42,
	'l',0x4B,
	'm',0x3A,
	'n',0x31,
	'o',0x44,
	'p',0x4D,
	'q',0x15,
	'r',0x2D,
	's',0x1B,
	't',0x2C,
	'u',0x3C,
	'v',0x2A,
	'w',0x1D,
	'x',0x22,
	'y',0x35,
	'z',0x1A,
	'0',0x45,
	'1',0x16,
	'2',0x1E,
	'3',0x26,
	'4',0x25,
	'5',0x2E,
	'6',0x36,
	'7',0x3D,
	'8',0x3E,
	'9',0x46,
	
	'`',0x0E,
	'-',0x4E,
	'=',0x55,
	'\\',0x5D,
	'\b',0x66,
	' ',0x29,
	'\t',0x0D,
	K_SHIFT,0x12,
	K_SHIFT,0x59,
	'\n',0x5A,
	K_ESC,0x76,
	'[',0x54,
	']',0x5B,
	';',0x4C,
	'\'',0x52,
	',' ,0x41,
	'.',0x49,
	'/',0x4A,
	
	//numpad
	'0',0x70,
	'1',0x69,
	'2',0x72,
	'3',0x7a,
	'4',0x6b,
	'5',0x73,
	'6',0x74,
	'7',0x6c,
	'8',0x75,
	'9',0x7d,
	'.',0x71,

	0,0
};

unsigned const char ktab_shift[] PROGMEM =
{

	'!', '1',
	'@', '2',
	'#', '3',
	'$', '4',
	'%', '5',
	'^', '6',
	'&', '7',
	'*', '8',
	'(', '9',
	')', '0',
	'_', '-',
	'+', '=',
	'?', '/',
	'<', ',',
	'>', '.',
	':', ';',
	'\"', '\'',
	'|', '\\',	
	0,0
};

unsigned const char ktab_eo[] PROGMEM =
{
	K_LEFT,0x6b,
	K_RIGHT,0x74,
	K_UP, 0x75,
	K_DOWN, 0x72,
	0,0
	
};
