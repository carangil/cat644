/*
 * test.c
 *
 * Created: 9/3/2018 12:50:59 AM
 *  Author: mark
 */ 


#include "kittyos.h"

#include <string.h>
#include "vga.h"

unsigned int test_main(environment_t* env){
	static char buf[20];
	
	static char b[50];
	
	char i;
	
	for (i=0;i<50;i++)
		b[i]= i*i;
			
	SELECT_RAM_BANK(0);
			
	memcpyi2x(0x2f0, b, sizeof (b) );
	prints(env->out, "Hello world\n");
	
	for (i=0;i<50;i++)
		b[i]= 0;
		
	memcpyx2i( b, 0x2f0, sizeof (b) );
	
	for (i=0;i<50;i++){
		prints(env->out, utoa(b[i], buf, 16));
		prints(env->out, "\n");
	}
		
		//try write again after read
		
	for (i=0;i<50;i++)
		b[i]= i*i+i;
		
	SELECT_RAM_BANK(1);
	
	memcpyi2x(0x2f0, b, sizeof (b) );
	for (i=0;i<50;i++)
		b[i]= 0;
	
	SELECT_RAM_BANK(1);
	memcpyx2i( b, 0x2f0, sizeof (b) );
	
	for (i=0;i<50;i++){
		prints(env->out, utoa(b[i], buf, 16));
		prints(env->out, "\n");
	}
	
	SELECT_RAM_BANK(0);
	memcpyx2i( b, 0x2f0, sizeof (b) );
	
	for (i=0;i<50;i++){
		prints(env->out, utoa(b[i], buf, 16));
		prints(env->out, "\n");
	}
		
		
	prints(env->out, "Bye world\n");
	
	reads(env->in, buf, sizeof(buf), 1);
	prints(env->out, " You said'");
	prints(env->out, buf);
	prints(env->out, "\n");

	while(1){
		
		unsigned char i;

		hscroll +=2;
		vga_delay(4);


		if(env->in->ready(env->in)){
			reads(env->in, buf, sizeof(buf), 1);
			if (buf[0]=='s')
				vga_slow();
				
			if (buf[0]=='f')
				vga_fast();
		}
		
		if (env->key->ready(env->key)) {
			snprintf(buf, sizeof(buf) , "[KEY %x]", env->key->read1(env->key));
			prints(env->out, buf);
		}
		
		//snprintf(buf, sizeof(buf), "read:%d ", keyreadpos);
	//	prints(env->out, buf);
	//	snprintf(buf, sizeof(buf), "write:%d ", keywritepos);
	//	prints(env->out, buf);
		/*
		for (i=keyreadpos; i!=keywritepos;i=(i+1)&0xF){
			snprintf(buf, sizeof(buf), " %x  ", keybuffer[i]);
			prints(env->out, buf);
			
		}*/
			//prints(env->out, "\n");
		
	}


	return 3;
		
}