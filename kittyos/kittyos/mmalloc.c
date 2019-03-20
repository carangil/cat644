#include "kittyos.h"

#ifndef __AVR__
	#include <stdio.h>
	#define DMESGF printf
#endif


//change if needed


#define MASK_FLAGS	0xC000
#define FLAG_FREE	0x8000  /* 1000 0000 0000 0000 */
#define FLAG_HANDLE	0x4000  /* 0100 0000 0000 0000 */
#define MASK_SIZEW	0x01FF  /* 0000 0001 1111 1111 */
#define MASK_REF	0x3E00  /* 0011 1110 0000 0000 */
#define REF_INC		0x0200  /* 0000 0010 0000 0000 */


typedef u16 mheader_t;

#define HSIZE 1024
u16 mheap[HSIZE];  //heap size, in 16-bit words

#define SIZEBYTES(h)  (((h)&MASK_SIZEW)<<1)
#define SIZEW(h)  (((h)&MASK_SIZEW))

void mminit(){
	
	u16 * heap = mheap;
	u16 heapsize = HSIZE; //in words
	u16 sizef;
	

	while(heapsize) {

		//take the header
		mheader_t* mh = (mheader_t* ) heap;
		heap += 1;  
		heapsize -= 1;

		if (heapsize > 256)
			sizef = 256;
		else
			sizef = heapsize;
	
		*mh = sizef | FLAG_FREE ;

		heap += sizef;
		heapsize-= sizef;

	}

}


void mmdump(){
	u16* heap = mheap;
	DMESGF("mem:\n");	
	while(1){
		mheader_t* mh = (mheader_t*) heap;
		if ((heap -  mheap) >= HSIZE ) {
			break;
		}
		char* datastart = (char*) (mh+1); //data starts past header
		char* dataend = datastart + SIZEBYTES(*mh);
		DMESGF("header %p  %p to %p   %d bytes actual, %d bytes should, next header %p flags %x ref %x\n", 
			mh,
			datastart, dataend, dataend-datastart, SIZEBYTES(*mh), dataend, (*mh)&(MASK_FLAGS), (*mh)&MASK_REF);

		heap = (u16*) dataend;

		
	}
	DMESGF("--\n");
}

void* mmalloc(u16 size){

	if (size > 512)
		return NULL; //too big

	// round up odd allocations
	if (size &1)
		size++; 

	//convert size to words
	size = size >> 1;

	u16 wsize;

	u16* heap = mheap;

	while(1){
		mheader_t* mh = (mheader_t*) heap;
		mheader_t* mhnext;
		if ((heap -  mheap) >= HSIZE ) {
			break;
		}

		wsize = SIZEW(*mh); //get header size
		//DMESGF(" Want %d found %d\n", size, wsize);

		if ( (*mh & FLAG_FREE) && (wsize >= size )) {

			//space is free, and big enough
			
			u16 remainder = wsize-size;

			if (remainder == 0) {
				//perfect fit, mark as not free, reference count 1
				(*mh) = ((*mh) & MASK_SIZEW) | REF_INC ;
				return mh+1;
			}

			//shrink this block
			(*mh) = size | REF_INC;

			//create next block with remained
			mhnext = mh+1+size;
			remainder--; //make space for block header
			(*mhnext) = FLAG_FREE | remainder;

			return mh+1;
		}

		mhnext = mh + 1 + wsize;  //go to next header

		if ( (*mh & FLAG_FREE) &&  (*mhnext & FLAG_FREE)   ) {
			//two free blocks in a row.  Combine them
			u16 newsize = SIZEW(*mh) + 1 + SIZEW(*mhnext);
			if (newsize > 256) {

				//DMESGF(" Make block too big! Split\n");
				u16 remainder = newsize - 256 - 1;
				newsize = 256;
				mhnext = mh + 1 + 256;
				(*mhnext) = remainder  | FLAG_FREE;
				
			}
			(*mh) = newsize | FLAG_FREE;
			//DMESGF(" Combine blocks to make %d + %d %d\n", SIZEW(*mh) , SIZEW(*mhnext),  newsize);
			mmdump();
			continue;  //try again
		}

		heap = mhnext;
	}

	return NULL;

}


void mmfree(void* v){

	
	mheader_t* mh = (mheader_t*) v;
	mh-- ; //go to header

	if ( (*mh) & FLAG_FREE) {
//		DMESGF(" EXTRA FREE!\n");
		return;
	}

	(*mh) -=  REF_INC;	
	DMESGF(" %x  %x \n", (*mh)&MASK_REF, REF_INC);

	if ( ((*mh) & MASK_REF) == 0) {
//		DMESGF("MARK FREE\n");
		(*mh) |= FLAG_FREE; //mark as free
		return;
	}


}

void* mmaddref(void* v){

	mheader_t* mh = (mheader_t*) v;
	mh-- ; //go to header

	//check if counter is saturated; if so, can't add reference
	if ( ((*mh) & MASK_REF) == MASK_REF) {
		return NULL;
	}

	(*mh) += REF_INC;

	return v;
}



