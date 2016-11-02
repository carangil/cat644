/*
 * filesystem.h
 *
 * Created: 8/25/2014 9:06:51 PM
 *  Author: mark
 */ 


#ifndef BKFILESYSTEM_H_
#define BKFILESYSTEM_H_

#include "avrstuff.h"
#include "drivers.h"
#include "avrstuff.h"

#define LLFS_BLOCKSIZE 512

typedef struct llfs_checksum_s {
	uchar sumodd;
	uchar sumall;	
} llfs_checksum_t;

#define llfs_MAGIC 0xC644

typedef struct llfs_block0_s {
	llfs_checksum_t	cs;
	int				magic;
	blocknum		size;	/* number of blocks on card */
	blocknum		fence;	/* this blocknum or higher is considered 'free' */
	blocknum		reserved;  /*No blocks before 'reserved' may be allocated dynamically*/
	blocknum		deletechain;  /* Points to first deleted block */	 
	uchar			mounted;	  /* written to true when mounted */
}llfs_block0_t;

typedef struct llfs_datablock_s {
	llfs_checksum_t	cs;
	blocknum		next;
	char			data[0];  /* expands to end of block */
} llfs_datablock_t;


#define MOUNT_SUCCESS	1
#define MOUNT_FAIL		0

typedef struct llfs_mountpoint_s
{
	blockdevice_t* dev;		
	llfs_block0_t	block0;
} llfs_mountpoint_t;

void llfs_updatechecksum(uchar* data, unsigned int blocksize);  /*update checksum of a block */
uchar llfs_verifychecksum(uchar* data, unsigned int blocksize);  /*check if checksum is OK */


uchar llfs_mount(llfs_mountpoint_t* mp, blockdevice_t* dev);
uchar llfs_unmount(llfs_mountpoint_t* mp);
uchar llfs_format(blockdevice_t* dev, blocknum size, blocknum reserved);
uchar llfs_deformat(blockdevice_t* dev);  //intentionally ruins a format



typedef struct llfs_filedev_s
{
	chardevice_t		chardev;  //regular chardev interface
	blocknum			firstblock;
	blocknum			lastblock;
	unsigned int		lastblock_offset;  /*how many bytes of the last block are used*/
	
	blocknum			curblock;    //block we are in
	unsigned int		curoffset;	 //offset we are in
	llfs_mountpoint_t*	mp;
} llfs_filedev_t;


//allocates a block, returns the block number. new block can be from freelist, or the fence can be advanced
uchar llfs_newblock(llfs_mountpoint_t* mp, blocknum* block);

#define LLFS_OPEN_START	1
#define LLFS_OPEN_END	2
#define LLFS_CREATE		3

uchar llfs_openblock(llfs_filedev_t* chardev, uchar flags);


#endif /* FILESYSTEM_H_ */