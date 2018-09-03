/*
 * test.c
 *
 * Created: 9/3/2018 12:50:59 AM
 *  Author: mark
 */ 


#include "kittyos.h"


unsigned int test_main(environment_t* env){
	static char x[20];
	prints(env->out, "Hello world\n");
	reads(env->in, x, sizeof(x), 1);
	prints(env->out, " You said'");
	prints(env->out, x);
	prints(env->out, "\n");
	
	return 3;
		
}