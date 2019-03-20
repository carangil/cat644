/*
 * xram.c
 *
 * Created: 7/14/2014 11:03:12 PM
 *  Author: mark
 */ 

#include "xram.h"
#include "kittyos.h"

volatile unsigned char lastpage=0;

void xram_init()
{
	//no write, no read
	RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK | RAMCTRL_OE_MASK;
	
	//set data bus direction
	RAMDATA_DDR = 0x0;  //all input
	RAMDATA_PORT = 0x0; //no pullup
	
	//write enable up, output enable down (RAM output is enabled)
	
	RAMCTRL_DDR =  RAMCTRL_DDR_MASK;
	
	//set to read mode  (write enable high, output enable low)
	RAMCTRL_PORT = (RAMCTRL_PORT&RAMCTRL_MASK) | RAMCTRL_WE_MASK ;

	//set address port direction
	RAMADDR_DDR = 0xFF;  //all output
	
	#ifdef XRAM_128K
		RAMEX_DDR |= RAMEX_DDR_MASK;  //enable one output
		SETA16LOW;
	#endif
	
}

/* copies to and from the 'data' portion of memory.  (Ignores VGA section) */

void memcpyi2x(unsigned int addrin, char* internal, unsigned int cnt) {

	unsigned int addr = addrin << 1;
	
	unsigned int final = addr+ cnt-1;
	unsigned char finalpage = final>>8;
	unsigned char finaloffset=final;

	unsigned char page = addr>>8;
	unsigned char offset = addr;

	if (cnt==0)
		return;

	if (addrin&0x8000)
		SETA16HIGH;
	else
		SETA16LOW;
				
	//select the first page
	SELECT_RAM_PAGE(page);
	START_FAST_WRITE;
	
	for(;;) {
		
		FAST_WRITE(offset,*(internal++));  //write a byte
		
		//check done
		if ((page==finalpage) && (offset==finaloffset)) 
			break;
		
		if(offset == 0xff){
			
			page++;
			SELECT_RAM_PAGE(page);
			
		}
		offset++;
	}
	
	END_FAST_WRITE;
		
}


void memcpyx2i( char* internal, unsigned int addrin,  unsigned int cnt) {
	
	unsigned int addr = addrin<<1;
	unsigned int final = addr+ cnt-1;
	unsigned char finalpage = final>>8;
	unsigned char finaloffset=final;

	unsigned char page = addr>>8;
	unsigned char offset = addr;

	if (cnt==0)
		return;
		
	if (addrin&0x8000)
		SETA16HIGH;
	else
		SETA16LOW;
			
	//select the first page
	SELECT_RAM_PAGE(page);
	//START_FAST_WRITE;
	
	for(;;) {
						
		FAST_READ( *(internal++) ,  offset);
		
		//check done
		
		if ((page==finalpage) && (offset==finaloffset))
		break;
		
		if(offset == 0xff){
			
			page++;
			SELECT_RAM_PAGE(page);
			
		}
		offset++;
	}
	
	
	
}


/* Simple allocator */

//internal to here
unsigned int xpeek(unsigned int addr){
	unsigned int ret;
	
	memcpyx2i( (char*) &ret, addr,  2);
	
	return ret;
}

void xpoke(unsigned int addr, unsigned int val){
	unsigned int ret = val;
	
	memcpyi2x(addr, &ret, 2);
	
}

//copied from mmalloc

#define MASK_FLAGS	0xC000
#define FLAG_FREE	0x8000  /* 1000 0000 0000 0000 */
#define FLAG_HANDLE	0x4000  /* 0100 0000 0000 0000 */
#define MASK_SIZEW	0x01FF  /* 0000 0001 1111 1111 */
#define MASK_REF	0x3E00  /* 0011 1110 0000 0000 */
#define REF_INC		0x0200  /* 0000 0010 0000 0000 */


typedef uint16_t u16;

#define SIZEBYTES(h)  (((h)&MASK_SIZEW)<<1)
#define SIZEW(h)  (((h)&MASK_SIZEW))

static u16 hs;
static u16 he;

void xalloc_init(unsigned int heap_start, unsigned int heap_end){

	u16 heap = heap_start;
	u16 heapsize = heap_end - heap_start + 1; //in words
	u16 sizef;

	hs = heap_start;
	he = heap_end;

	while(heapsize) {

		//take the header
		u16 mh = heap;
		heap += 1;
		heapsize -= 1;

		if (heapsize > 256)
			sizef = 256;
		else
			sizef = heapsize;
	
		xpoke(mh, sizef | FLAG_FREE );
		//*mh = sizef | FLAG_FREE ;

		heap += sizef;
		heapsize-= sizef;

	}
	
}

void xdump(){
	u16 heap = hs;
	DMESGF("mem:\n");
	while(1){
		u16 mh = heap;
		if ((heap-1) >= he) {
			break;
		}
		u16 datastart =  (mh+1); //data starts past header
		u16 dataend = datastart + SIZEW(  xpeek(mh)   );
		DMESGF("h %p %p to %p, %d words, next @ %p flags %x ref %x\r\n",
		mh,
		datastart, dataend, dataend-datastart, dataend, xpeek(mh)&(MASK_FLAGS), xpeek(mh)&MASK_REF);

		heap =  dataend;

		
	}
	DMESGF("--\n");
}

void* xalloc(u16 size){

	if (size > 512)
		return NULL; //too big

	// round up odd allocations
	if (size &1)
		size++;

	//convert size to words
	size = size >> 1;

	u16 wsize;

	u16 heap = hs;

	while(1){
		u16 mh = heap;
		u16 mhnext;
		
		if ((heap-1) >= he) {
			break;
		}

		wsize = SIZEW(xpeek(mh)); //get header size
		DMESGF(" Want %d found %d\r\n", size, wsize);

		if ( ( xpeek(mh) & FLAG_FREE) && (wsize >= size )) {

			//space is free, and big enough
			
			u16 remainder = wsize-size;

			if (remainder == 0) {
				//perfect fit, mark as not free, reference count 1
				xpoke(mh, (xpeek(mh) & MASK_SIZEW) | REF_INC );
				return mh+1;
			}

			//shrink this block
			//(*mh) = size | REF_INC;
			xpoke(mh,  size | REF_INC);

			//create next block with remained
			mhnext = mh+1+size;
			remainder--; //make space for block header
			//(*mhnext) = FLAG_FREE | remainder;
			xpoke(mhnext, FLAG_FREE | remainder);

			return mh+1;
		}

		mhnext = mh + 1 + wsize;  //go to next header

		if ( (xpeek(mh) & FLAG_FREE) &&  (xpeek(mhnext) & FLAG_FREE)   ) {
			//two free blocks in a row.  Combine them
			u16 newsize = SIZEW(xpeek(mh)) + 1 + SIZEW(xpeek(mhnext));
			if (newsize > 256) {

				//DMESGF(" Make block too big! Split\n");
				u16 remainder = newsize - 256 - 1;
				newsize = 256;
				mhnext = mh + 1 + 256;
				//(*mhnext) = remainder  | FLAG_FREE;
				xpoke(mhnext , remainder  | FLAG_FREE);
				
			}
			//(*mh) = newsize | FLAG_FREE;
			xpoke(mh, newsize | FLAG_FREE);
			DMESGF(" Combine blocks to make %d + %d %d\r\n", SIZEW(xpeek(mh)) , SIZEW(xpeek(mhnext)),  newsize);
			xdump();
			continue;  //try again
		}

		heap = mhnext;
	}

	return NULL;

}