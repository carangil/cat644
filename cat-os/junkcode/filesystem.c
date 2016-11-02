/*
 * filesystem.c
 *
 * Created: 8/25/2014 10:22:11 PM
 *  Author: mark
 */ 
#include "avrstuff.h"
#include "filesystem.h"

#define CS_BAD	0
#define CS_GOOD	1

char str[20];

/* First sizeof(llfs_checksum_t) bytes of *data is the checksum; don't include it in the calculation */

void llfs_compute_checksum(uchar* data, llfs_checksum_t* cs, unsigned int blocksize)
{
	uchar sumodd = 0;
	uchar sumall = 0;
	int i;
	
	sprintf(str, "i is %d:%d;\n", (int) sizeof(*cs), (int)blocksize);
	debug(str);
	
	
	for (i = sizeof(*cs); i< blocksize;i++)
	{
		
		sumall += data[i];
		
		if (i&1)
			sumodd += data[i];		
	}
	
	sumall = sumall + 1  ;  /* Just so that the checksum of all zeros isn't zero (so a 'blank block) is not valid) */
	
	sprintf(str, "csc %x:%x;\n", (int)sumall, (int)sumodd);
	debug(str);
	cs->sumall = sumall;
	cs->sumodd = sumodd;
		
}


void llfs_updatechecksum(uchar* data, unsigned int blocksize)
{
	llfs_checksum_t cs;
	llfs_compute_checksum(data, &cs, blocksize);
	
	sprintf(str, "cs %x:%x;\n", (int)cs.sumall, (int)cs.sumodd);
	debug(str);
	
	*((llfs_checksum_t*)data) = cs;
}

uchar llfs_verifychecksum(uchar* data, unsigned int blocksize)
{
	llfs_checksum_t cs;
	llfs_compute_checksum(data, &cs, blocksize);
	
	if( ((llfs_checksum_t*)data)->sumall != cs.sumall)
		return CS_BAD;
		
	if( ((llfs_checksum_t*)data)->sumodd != cs.sumodd)
		return CS_BAD;
		
	return CS_GOOD;
		
}

#define BLOCK_INVALID 0

unsigned char blockbuffer[512]; 
blocknum blockbuffer_block = BLOCK_INVALID ; //what block number is currently in the buffer
uchar blockbuffer_minor;  //which file is this blockbuffer valid for?
uchar blockbuffer_dirty;	//set if the buffer is modified



uchar llfs_unmount(llfs_mountpoint_t* mp);


/* fsformat will clear the blockbuffer! */

uchar llfs_format(blockdevice_t* dev, blocknum size, blocknum reserved) {
	
	llfs_block0_t* block0 = (llfs_block0_t*) blockbuffer;
	
	memset(block0, 0, sizeof(llfs_block0_t));
	
	block0->fence = reserved;
	block0->reserved = reserved;
	block0->magic = llfs_MAGIC;
	block0->size = size;
	
	llfs_updatechecksum((char*) block0, sizeof(llfs_block0_t));
	
	return dev->writeblock(dev->minor, 0, (char*) block0, sizeof(llfs_block0_t));
	
}

uchar llfs_deformat(blockdevice_t* dev){
	llfs_block0_t* block0 = (llfs_block0_t*) blockbuffer;
	
	memset(block0, 0, sizeof(llfs_block0_t));
	
	return dev->writeblock(dev->minor, 0, (char*) block0, sizeof(llfs_block0_t));
}


uchar llfs_mount(llfs_mountpoint_t* mp, blockdevice_t* dev)
{
	
	dev->readblock( dev->minor, 0, &( mp->block0), sizeof(llfs_block0_t));
	
	if (mp->block0.magic != llfs_MAGIC) 
	{
		debug ("bad magic\n");
		//return MOUNT_FAIL;
	}
	
	sprintf(str, "reserved %ld;",mp->block0.reserved); debug(str);
	sprintf(str, "fence %ld;",mp->block0.fence); debug(str);
	sprintf(str, "size %ld;",mp->block0.size); debug(str);
	sprintf(str, "magic %x;",mp->block0.magic); debug(str);
	sprintf(str, "cs %x %x;",mp->block0.cs.sumall, mp->block0.cs.sumodd); debug(str);
	

	
	if (llfs_verifychecksum(&mp->block0, sizeof(llfs_block0_t)) == CS_GOOD)
	{
		mp->dev = dev;
		return MOUNT_SUCCESS;		
	}
	
	debug("BAD CHECKSUM\n");
	mp->dev = NULL;
	return MOUNT_FAIL;	
		
}


uchar llfs_newblock(llfs_mountpoint_t* mp, blocknum* block)
{
	
	if (mp->block0.fence >= mp->block0.size)
	{
		//disk is full.
		debug("Disk full. Freechain not implemented");
		return 0;
	}
	
	*block = mp->block0.fence; /* Take the block at the fence */
	(mp->block0.fence)++; /* Advance the fence */
	
	return 1;
}

#define MAXFILE 4


llfs_filedev_t* filenumber[] ={NULL,NULL,NULL, NULL};// up to 5 file handles



uchar llfs_cacheblock(uchar minor, blockdevice_t* dev, blocknum blk)
{
	/* quick return if that's the current block */
	
	if((blockbuffer_block == blk) && (blockbuffer_minor == minor))
		return 1; 
		
	//TODO: if blockbuffer is 'dirty' write it back before reading
		

	if (dev->readblock(dev->minor, blk, blockbuffer, LLFS_BLOCKSIZE))
	{
		
		blockbuffer_minor = minor;
		blockbuffer_block = blk;
		blockbuffer_dirty = 0;
					
		return 1; 
	}
	return 0;
}

void llfs_putc(uchar minor, char ch)
{
	//find file device
	llfs_filedev_t* dev = filenumber[minor];
	llfs_datablock_t* b = (llfs_datablock_t*) blockbuffer;
	
	llfs_cacheblock(minor, dev->mp->dev, dev->curblock); /* Make sure the current block in the file is the one in the cache */
	
	if (dev->curoffset ==  (LLFS_BLOCKSIZE - sizeof(llfs_datablock_t)) )
	{
		debug ("block full, go to next block, not implemented yet");
		
				
	}
	
	b->data[dev->curoffset++] = ch; /* put in block */	
	
	//if((dev->curblock == dev->lastblock) && (dev->curoffset > dev->lastoffset)
	
}


llfs_close(llfs_filedev_t* dev) {
	//
	
	
	
}

uchar llfs_openblock(llfs_filedev_t* dev, uchar flags)
{
	// mountpoint (mp) must be set.
	
	llfs_datablock_t* b = (llfs_datablock_t*) blockbuffer;
	unsigned char fn;
	
	/* find a free file number*/
	for (fn=0;fn<MAXFILE;fn++)
		if (filenumber[fn] == NULL)
		
	if (fn == NULL)
		return 0;  /* Too many open files */
						
	if (flags == LLFS_CREATE)
	{
		if(llfs_newblock( dev->mp,  &(dev->curblock) ))
		{
			//curblock is first block
			dev->firstblock = dev->curblock;
			dev->lastblock = dev->curblock;
			dev->curoffset = 0;
			filenumber[fn] = dev;
			dev->chardev.minor = fn;
			dev->chardev.putc = llfs_putc;
			
			//make sure the 1st block is in the blockcache
			llfs_cacheblock(fn, dev->mp->dev, dev->curblock);
			b->next = BLOCK_INVALID;		//no next block yet
						
			return 1;
		}
			
		
	}
	
}
