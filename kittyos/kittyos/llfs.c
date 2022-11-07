/*
 * llfs.c
*/
#include <stdint.h>
#include "avrstuff.h"
#include "kittyos.h"
#include "llfs.h"
#include "sdcard.h"
//read/write functions

void blockRead(blocknum b, void* data, int len){
	DMESGF("r%ld;", b);
	sdcard_read_block(NULL, b, data, len);
}
void blockWrite(blocknum b, void* data, int len){
	DMESGF("w%ld;", b);
	sdcard_write_block(NULL, b, data, len);
}



block0t block0;
blocknum upto=0;

void syncInfo(){
	//writes block 0
	blockWrite(0, &block0, sizeof(block0));
	DMESGF(" WRITE FS INFO %ld %ld %ld  upto(%ld)\n", block0.start, block0.free, block0.end, upto);
}

void mount(){
	//reads block 0
	blockRead(0, &block0, sizeof(block0));
	upto = block0.free;
	DMESGF("  READ FS INFO %ld %ld %ld  (%ld)\n", block0.start, block0.free, block0.end, upto);
}

char btempdirty=0;//set to 1 when written to
blocknum btempnum=0;
char btemp[BLOCKSIZE];


void unmount(){
	DMESGF(" Unmounting\n");
	//flush dirty block
	if (btempdirty && (btempnum != 0) ){
		//need to write the old block
		blockWrite(btempnum, btemp, BLOCKSIZE);
	}

	//any 'checked out' freespace needs to be put back
	if (block0.free > upto){
		block0.free = upto;
	}

	syncInfo();

}

blockHeader bh;

void initDisk(blocknum blocks) {

	memset(&bh, 0, sizeof(bh));
	blockWrite(1, &bh, 0); //write a blank block at 1
	block0.start= 1; //start at block 1
	block0.free=2; //free space starts here
	block0.end= blocks;
	syncInfo();
}

blocknum blockAlloc(){
	if ( upto == block0.free){

		if (block0.free < block0.end){
			block0.free += 100;  //checkout 100 blocks
			if (block0.free > block0.end)
			block0.free = block0.end; //don't go past end

			syncInfo();
			
		}
	}

	return upto++;
}

//get root block information
int blockRoot(blockInfo* bi){
	blockHeader bh;
	blockRead(1, &bh, sizeof(bh));//read block 1 header
	bi->b=1;
	bi->childFirst = bh.childFirst;
	bi->parent = 0;
	bi->next = bh.next;
	bi->prev=0;
	return 1;
}

void blockDirty(){
	btempdirty=1;
}

void switchBlock(blocknum bn){  //'0' means allocate a block
	DMESGF(" SWITCH to %ld ...current is %ld\n", bn, btempnum);

	if ((bn != 0) && (bn == btempnum))
	return;  //already loaded

	//requesting different block; load 1
	if (btempdirty && (btempnum != 0) ){
		//need to write the old block
		blockWrite(btempnum, btemp, BLOCKSIZE);
	}

	if (bn){
		blockRead(bn, btemp, BLOCKSIZE);
		btempdirty = 0;
		}else{
		bn = blockAlloc();
		btempdirty = 1;
	}

	btempnum=bn;
}

//write the data section of a block
void blockSetData(blockInfo* bi, void* data, int len){
	switchBlock(bi->b);  //make this block resident
	blockHeader* btemph = (blockHeader*) btemp;

	btemph->used = len;
	btemph->datastart = sizeof(blockHeader);
	memcpy(btemp+ btemph->datastart, data, len); //copy info
	blockDirty(); //block is dirty
}

//ppend to data section of a block, inserting a new block after if necessary.  Caller is left with a pointer to the new block if aan insert has occured (ready to just append more data... this part is just like appending to a regular linear file)
void blockAppendData(blockInfo* bi, void* data, int len){
	switchBlock(bi->b);  //make this block resident
	blockHeader* btemph = (blockHeader*) btemp;

	int total = btemph->used + btemph->datastart + len;
	int tocopy;

	if (total > BLOCKSIZE){
		tocopy = BLOCKSIZE - btemph->datastart - btemph->used;
		} else {
		tocopy=len;
	}
	//DMESGF(" copy %d to existing header %d and used %d to make block %d bytes\n", tocopy, btemph->datastart , btemph->used,   btemph->datastart + btemph->used +tocopy);

	memcpy(btemp+ btemph->datastart + btemph->used, data, tocopy); //copy info
	btemph->used += tocopy;
	blockDirty(); //block is dirty

	//write the remainder
	if (tocopy < len){
		blockInsert(bi, data+tocopy, len-tocopy);
	}

}


//adds a child block to current block. IF there are already child blocks, this is prepended to the entire chain.  Caller is left with a pointer to the new child block, which can be written to
void blockAddFirstChild(blockInfo* bi,void* data, int len){
	blocknum oldFirst;
	blockHeader* btemph = (blockHeader*) btemp;

	switchBlock(0); //new block
	blocknum nb = btempnum;
	blocknum newNext = bi->childFirst;
	btemph->used = len;
	btemph->next = newNext; //next block is whatever chain was already there
	btemph->datastart = (int) sizeof(blockHeader);
	memcpy(btemp + btemph->datastart, data, len); //copy in data

	switchBlock(bi->b);		//switch to parent block, saving the current temp block
	btemph->childFirst = nb;	//first child is the new block
	blockDirty();

	//parent pointer that was passed in now going to be modified to be the child
	bi->b = nb;	//child that was just created
	bi->parent = bi->b;	//child's parent
	bi->childFirst = 0; //was just created
	bi->next = newNext;
	bi->prev = 0;

	//function returns the 'bi' pointing to the newly created block.
}

//inserts a new block AFTER the current block.  Caller is left with a pointer to the new block, where it can easily be appended or another inserted after

void blockInsert(blockInfo* bi,void* data, int len){
	blockHeader* btemph = (blockHeader*) btemp;

	switchBlock(0); //new block
	blocknum nb = btempnum;
	btemph->used = len;
	blocknum remainder = bi->next;
	btemph->next = remainder; //next block is whatever chain was already there
	btemph->datastart = (int) sizeof(blockHeader);
	memcpy(btemp + btemph->datastart, data, len); //copy in data

	switchBlock(bi->b);		//switch to block
	btemph->next = nb;
	blockDirty();

	bi->prev = bi->b;
	bi->b = nb;	//child that was just created
	bi->childFirst = 0; //was just created
	bi->next = remainder; //next one was the remainder of the chain
}

//read data section of a block
uint16_t blockReadData(blockInfo* bi,void* data, uint16_t len){
	blockHeader* btemph = (blockHeader*) btemp;

	switchBlock(bi->b); //make sure that block is resident
	uint16_t min = btemph->used;
	if (len < min)
		min = len;

	//copy out any data in there
	if (min > 0)
		memcpy(data, btemp+ btemph->datastart, min);
	
	return min;
}

int blockFirstChild(blockInfo* bi){

	if (!bi->childFirst){
		return 0; //no children
	}

	blockHeader* btemph = (blockHeader*) btemp;
	bi->parent = bi->b; //going into child, but track the parent
	switchBlock(bi->childFirst);
	bi->b = bi->childFirst;
	bi->childFirst = btemph->childFirst;
	bi->next = btemph->next;
	bi->prev = 0;
	return 1;
}

int blockNext(blockInfo* bi){
	if (!bi->next)
		return 0; //no next block

	blockHeader* btemph = (blockHeader*) btemp;
	switchBlock(bi->next);
	bi->prev = bi->b;
	bi->b = bi->next;
	bi->next = btemph->next;
	//bi->parent is the same
	return 1;
}

void showPos(blockInfo* bi){
	DMESGF( "< block %ld  parent:%ld firstChild:%ld>\r\n", bi->b, bi->parent,  bi->childFirst);
}

