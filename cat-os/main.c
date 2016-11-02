/*
 * main.c
 *
 * Created: 10/16/2016 7:32:03 PM
 *  Author: mark
 */ 
#include "cat1.h"

void main(void)
{
	
	init();
	
	
	while (1){
		
		prints(&dev_comm, "Testing write.\nType something:");
		readline(&dev_comm, scratch, sizeof(scratch), 1);
		prints(&dev_comm, "You said");
		prints(&dev_comm, scratch);
		prints(&dev_comm, "\n");
	}
	
}
