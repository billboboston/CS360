#include "types.h"

/************************************************************************
*						Helper functions
***********************************************************************/

// tests if a bit is avalable in the buf
int tst_bit(char *buf, int bit) {
	return buf[bit / 8] & (1 << (bit % 8));
}

//sets a given bit in the buf
void set_bit(char *buf, int bit) {
	buf[bit / 8] |= (1 << (bit % 8));
}

//takes in block number then reads block from device into buf
int get_block(int dev, int blk, char *buf){
	lseek(dev, (long)blk*BLKSIZE, SEEK_SET);
	return read(dev, buf, BLKSIZE);
}

//takes in block number and writes the data from buf onto the devive
int put_block(int dev, int blk, char *buf){
	lseek(dev, (long)blk*BLKSIZE, SEEK_SET);
	return write(dev, buf, BLKSIZE);
}

//decrements the free inodes count in the SUPER and Group Descriptor blocks
	//Used for creating a file or directory on the file system
int decFreeInodes(int dev){
	char buf[BLKSIZE];
	SUPER *spTmp;
	GD *gpTmp;

	get_block(dev, 1, buf);
	spTmp = (SUPER *)buf;
	spTmp->s_free_inodes_count--;
	put_block(dev, 1, buf);
	get_block(dev, 2, buf);
	gpTmp = (GD *)buf;
	gpTmp->bg_free_inodes_count--;
	put_block(dev, 2, buf);
}

//decrements the free data blocks count in the SUPER and Group Descriptor blocks
	//Used for creating a file or directory on the file system
int decFreeDataBlocks(int dev){
	char buf[BLKSIZE];
	SUPER *spTmp;
	GD *gpTmp;

	get_block(dev, 1, buf);
	spTmp = (SUPER *)buf;
	spTmp->s_free_blocks_count--;
	put_block(dev, 1, buf);
	get_block(dev, 2, buf);
	gpTmp = (GD *)buf;
	gpTmp->bg_free_blocks_count--;
	put_block(dev, 2, buf);
}

//allocates an inode in the imap block on the device
	//RETURNS: the inode number of the newly alloced inode, 0 if there are no more free inodes 
int ialloc(int dev){
	char buf[BLKSIZE];
	get_block(dev, mtable[0].imap, buf);
	//scan the whole imap block
	for (int i = 0; i < mtable[0].ninodes; i++) {
		//if an unused (0) inode is found, allocate it and mark it in-use (1)
		if (tst_bit(buf, i) == 0) {
			set_bit(buf, i);		//sets the first unused (0) inode to in-use (1)
			put_block(dev, mtable[0].imap, buf);	//write updated imap block back to the device
			decFreeInodes(dev);	// update free inode count in SUPER and GD
			return (i + 1);
		}
	}
	return 0; // out of FREE inodes
}

//allocates a data block in the bmap block on the device
	//RETURNS: the data block number of the newly alloced data block, 0 if there are no more free data blocks 
int balloc(int dev){
	char buf[BLKSIZE];
	get_block(dev, mtable[0].bmap, buf);
	//scan the whole bmap block
	for (int i = 0; i < mtable[0].nblock; i++) {
		//if an unused (0) data block is found, allocate it and mark it in-use (1)
		if (tst_bit(buf, i) == 0) {
			set_bit(buf, i);		//sets the first unused (0) data block to in-use (1)
			put_block(dev, mtable[0].bmap, buf);	//write updated bmap block back to the device
			decFreeDataBlocks(dev);	// update free data block count in SUPER and GD
			return (i + 1);
		}
	}
	return 0; // out of FREE data blocks
}

void clr_bit(char *buf, int bit) // clear bit in char buf[BLKSIZE]
{   buf[bit / 8] &= ~(1 << (bit % 8)); }

// INODE  de-Allocate
void idalloc(int dev, int ino){
	int i;
	char buf[BLKSIZE];
	if (ino > mtable[0].ninodes){ // niodes global
		printf("inumber %d out of range\n", ino);
		return;
	}
	// get inode bitmap block
	get_block(dev, mtable[0].imap, buf);
	clr_bit(buf, ino-1);
	// write buf back
	put_block(dev, mtable[0].imap, buf);
	// update free inode count in SUPER and GD
	incFreeInodes(dev);
}

// Data Block de-Allocate
void bdalloc(int dev, int bno){
	int i;
	char buf[BLKSIZE];
	if (bno > mtable[0].nblock){ // niodes global
		printf("bnumber %d out of range\n", bno);
		return;
	}
	// get inode bitmap block
	get_block(dev, mtable[0].bmap, buf);
	clr_bit(buf, bno-1);
	// write buf back
	put_block(dev, mtable[0].bmap, buf);
	// update free inode count in SUPER and GD
	incFreeBlocks(dev);
}

// increments free INODES when an item is removed
void incFreeInodes(int dev){
	char buf[BLKSIZE];// inc free inodes count in SUPER and GD
	SUPER *spTmp;
	GD *gpTmp;

	get_block(dev, 1, buf);
	spTmp = (SUPER *)buf;
	spTmp->s_free_inodes_count++;
	put_block(dev, 1, buf);
	get_block(dev, 2, buf);
	gpTmp = (GD *)buf;
	gpTmp->bg_free_inodes_count++;
	put_block(dev, 2, buf);
}

// incriments Free datablocks when an item is removed
void incFreeBlocks(int dev){
	char buf[BLKSIZE];// inc free inodes count in SUPER and GD
	SUPER *spTmp;
	GD *gpTmp;

	get_block(dev, 1, buf);
	spTmp = (SUPER *)buf;
	spTmp->s_free_blocks_count++;
	put_block(dev, 1, buf);
	get_block(dev, 2, buf);
	gpTmp = (GD *)buf;
	gpTmp->bg_free_blocks_count++;
	put_block(dev, 2, buf);
}

//load INODE of (dev,ino) into a minode[]; return mip->minode[]
MINODE* iget(int dev, int ino)
{
	//search minode[ ] array for an item pointed by mip with the SAME (dev,ino)
	for (int i = 0; i < NMINODE; i++)
	{
		if ((minode[i].refCount != 0) && (minode[i].dev == dev) && (minode[i].ino == ino))
		{
			//it was found in the in memory inode array
			minode[i].refCount++;
			return &minode[i];
		}
	}
	/*if we got down to here it wasnt found in the in-memory inode array, so we need to put it in there then*/

	//search for an open spot in the in memory inode array (the first spot with refCount == 0)
	int i = 0;
	while (i < NMINODE)
	{
		if (minode[i].refCount == 0)
		{
			minode[i].refCount = 1;
			minode[i].dev = dev;
			minode[i].ino = ino;
			minode[i].dirty = 0;
			minode[i].mounted = 0;
			minode[i].mptr = NULL;
			break;
		}
		i++;
	}

	/*
	* now we will use the mailman algorithm to compute:
	*	blk (the block containing THIS inode)
	*	disp (which inode in the above block)
	*/
	int blk = ino / 8;
	int disp = ino % 8;
	if (disp == 0) { blk--; disp = 8; }

	get_block(dev, mtable[0].iblock + blk, minode[i].buf);	//load the block from the given inode into buf starting from the first iblock/
	minode[i].INODE = (INODE *)minode[i].buf; //ip now points at #1 node and we want it to point to the disp inode

	//Searching for the exact inode...
	int j = 1;
	while ((j != disp) && (j < 9))
	{
		minode[i].INODE++;
		j++;
	}

	return &minode[i];
}

//dispose of a minode[] pointed by mip
void iput(MINODE *mip)
{
	INODE * ip2;
	char buf[BLKSIZE];
	mip->refCount--;	//we are done using this inode so de-incriment refCount by one

	if (mip->refCount > 0) { return; }	//someone else is still using it, dont delete it
	if (mip->dirty == 0) { return; }	//inode was never modified, no need to write back to disk

	/* --Now we write the inode back to the disk--

	* now we will use the mailman algorithm to and mip->ino compute:
	*	blk (the block containing THIS inode)
	*	disp (which inode in the above block)
	*/
	int blk = mip->ino / 8;
	int disp = mip->ino % 8;
	if (disp == 0) { blk--; disp = 8; }

	get_block(mip->dev, mtable[0].iblock + blk, buf);	//grab the block containing the inode from the disk
	ip2 = (INODE *)buf;	//point the inode pointer to the buf with the offset of disp to get the exact inode

	int j = 1;
	while ((j != disp) && (j < 9))
	{
		ip2++;
		j++;
	}

	*ip2 = *mip->INODE;         // copy INODE into *ip

	put_block(mip->dev, mtable[0].iblock + blk, buf);
}

//returns the inode number of where 'name' exists in the directory of mip (returns 0 if 'name' does not exist)
int search(MINODE *mip, char *name)
{
	char *cp;	//used for traversing through entries
	char temp[256];		//used to store the name of the current entry
	char buf[BLKSIZE];
	DIR *dp;
	//
	//do indirect blocks first (i_block[0] - i_block[11]
	for (int i = 0; i < 14; i++)
	{
		//only search in non-empty blocks
		if (mip->INODE->i_block[i])
		{
			get_block(mip->dev, mip->INODE->i_block[i], buf);		//get the data from the i_block and put it into buf

			dp = (DIR *)buf;	//creating a DIR struct from buf data block
			cp = buf;

			//search the whole block
			while (cp < buf + BLKSIZE)
			{
				if ((strncmp(name, dp->name, dp->name_len) == 0) && (strlen(name) == dp->name_len)) { return dp->inode; } //it found the inode in the directory matching 'name' so return the value
				
				cp += dp->rec_len; // advance cp to the next entry (by rec_len)
				dp = (DIR *)cp;		//have the dir entry follow along
			}
		}
	}
	/* TODO: also implement indirect blocks (i_block[12]) and double indirect blocks (i_block[13]) */
	return 0;
}

//return the inode number of where the inputted pathname is located
int getino(char *pathname){
	char *pthCpy = malloc(strlen(pathname) * sizeof(char));
	strcpy(pthCpy, pathname);

	int i, n, ino, blk, disp;
	char buf[BLKSIZE];
	char *pch;
	MINODE *mip;
	dev = root->dev; // only ONE device so far

	if (strcmp(pthCpy, "/") == 0) { return 2; }	//if they were looking for root, return inode 2 becuase that is were the root is located
	if (pthCpy[0] == '/') { mip = iget(dev, 2); n = 0; }	//if absolute, start from root, also set number of names (n) to 0
	else { mip = iget(dev, running->cwd->ino); n = 1; }	//if relative, start from current working directory (cwd), also set number of names (n) to 1

	/* Now tokenize the pthCpy */
	i = 0;
	while (pthCpy[i]) { if (pthCpy[i] == '/') { n++; } i++; }	//count how many names there are in the pthCpy

	char **names = malloc((n + 1) * sizeof(char *));	//need to create an array of strings to hold all of the names

	pch = strtok(pthCpy, "/");

	for (i = 0; i < n; i++)
	{
		names[i] = (char *)malloc(strlen(pch) + 1);
		names[i] = pch;
		pch = strtok(NULL, "/");
	}
	names[n + 1] = NULL;

	/* Now get the inode number */

	for (i = 0; i < n; i++)
	{
		ino = search(mip, names[i]);		//get the inode number of the name

		if (ino == 0)
		{
			iput(mip);
			return 0;
		}

		iput(mip);
		mip = iget(dev, ino);	//go to the found inode
	}
	iput(mip);
	return ino;		//return the final found inode number
}

//Initialize the data structires of LEVEL-1 with an inputted device 
void init()
{
	printf("Initalizing procs...");
	//initialize both PROCs, P0 with uid=0, P1 with uid=1, all PROC.cwd = 0:
	for (int i = 0; i < 16; i++){
		proc[0].fd[i] = NULL;
		proc[1].fd[i] = NULL;
	}
	proc[0].uid = 0;
	proc[1].uid = 1;
	proc[0].cwd = NULL;
	proc[1].cwd = NULL;
	printf("Done\n");

	printf("Initalizing in-memory inode table...");
	//initialize minode[100] with all refCount = 0 (show that everything is empty)
	for (int i = 0; i < NMINODE; i++)
	{
		minode[i].refCount = 0;
	}
	printf("Done\n");

	printf("Initalizing root to NULL...");
	root = NULL;	//no root yet
	printf("Done\n");
}
