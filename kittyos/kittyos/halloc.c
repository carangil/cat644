/*
 * halloc.c
 *
 * Created: 3/19/2019 7:12:19 PM
 *  Author: divvy
 */ 

#include "kittyos.h"
#include "xram.h"

u16 halloc(u16 bytes){
		
	//allocate in xram, and use the xram location as the handle
	u16 h = xalloc(bytes);
	DMESGF("new handle %x\r\n", h);
	return h;

}

void* mmfindhandle(u16 handle);


u16 xsize(u16);

void* hgrab(handle_t handle){
	
	void* p = mmfindhandle(handle);	
	
	if (p){
		DMESGF("%x found in heap\r\n",handle);
		return(mmaddref(p));
	}
	DMESGF("%x not found in heap\r\n",handle);
	//not found.
	//get size in xram
	
	u16 size = xsize(handle);
	DMESGF("xobject size %d\r\n", size);
	p = mmalloc_handle(size, handle);	
	
	if (p){
		DMESGF("copy from x\r\n");
		memcpyx2i(p, handle, size);
	}
	
	return p;
}

handle_t hrelease(void* p){
	
	u16 h = mmgethandle(p); //get handle for specified object
	
	DMESGF("found handle %x\r\n", h);
	if (!h)
		return 0;
	int a=mmrefcount(p);
	
	DMESGF(" refcount %d\r\n", a/ MM_REF_ONE );
	
	if ( a == MM_REF_ONE) {
		DMESGF("last reference, copy to x\r\n");
		memcpyi2x(h, p, xsize(h));
	//	memset(p, 0, xsize(h));  //blank it out (not needed; just for testing)
	}
	
	mmfree(p);
	
	return h;
	
}
