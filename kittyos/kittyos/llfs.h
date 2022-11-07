/*
 * llfs.h
 *
 */ 


#ifndef LLFS_H_
#define LLFS_H_


#define BLOCKSIZE 512

typedef uint32_t blocknum;
typedef char bool;

typedef struct blockHeaderS{
	blocknum	next;		//next block at this level
	blocknum	childFirst;	//first block on next level
	blocknum	parent;
	uint16_t	used;		//number of bytes used in this block
	uint8_t		flags;		//not deviced what is needed yet
	uint8_t		datastart;	//offset into block to the data itself
} blockHeader;

typedef struct blockInfoS{
	blocknum b;
	blocknum next;
	blocknum childFirst;
	blocknum parent;
	blocknum prev;
} blockInfo;

typedef struct block0S {
	uint16_t sum;
	blocknum start;
	blocknum free;
	blocknum end;
} block0t;

//extern blockdevice_t* LLFS_block_bdev;

void mount();
void initDisk(blocknum blocks);
int blockRoot(blockInfo* bi);
void blockSetData(blockInfo* bi, void* data, int len);
void blockAppendData(blockInfo* bi, void* data, int len);
void blockAddFirstChild(blockInfo* bi,void* data, int len);
void blockInsert(blockInfo* bi,void* data, int len);
uint16_t blockReadData(blockInfo* bi,void* data, uint16_t len);

int blockFirstChild(blockInfo* bi);
int blockNext(blockInfo* bi);


#endif /* LLFS_H_ */