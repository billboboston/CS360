/* Linux EXT2 file system
* Semester Project for CS_360 (Fall-17)
* Professor: KC Wang
* 
* Authors: Trent & William
*/

#include "my_ls.c"

// enter_child: inserts child (ino) into the correct spot of the parent iNode(pmip)
// Input: pareint iNode pointer, inode num, child dir name
// NOTE: There are no current handlers to insert the new dir into a full disk and will break the disk
void enter_child(MINODE *pmip, int ino, char *child) {
	char *cp;	//used for traversing through entries
	char buf[BLKSIZE];
	bzero(buf, BLKSIZE);
	DIR *dp;

	int need_length = 4 * ((8 + strlen(child) + 3) / 4);	//how much space our new directory is going to take up in the parent directory (always has to be a multiple of 4)
	int remain;	//LAST entry's rec_len - its ideal_length
	int lastEntryIdealSize;	//the last entry's ideal size
	int last_rec_len;

	for (int i = 0; i < 12; i++)
	{
		if (!pmip->INODE->i_block[i])
		{
			//if we got here that means that there wasnt room in the existing blocks so we need to allocate another one to the parent
			char buf2[BLKSIZE];
			bzero(buf2, BLKSIZE); // optional: clear buf[ ] to 0
			DIR *dp2;
			dp2 = (DIR *)buf2;

			int blk = balloc(pmip->dev);	//allocate new data block for parent

			pmip->INODE->i_block[i] = blk;	//assign new data block nnumber to parent
			pmip->INODE->i_size += BLKSIZE;	//since we are adding another data block to the parent we need to increment the size by BLKSIZE

											//make new dir entry
			dp->inode = ino;	//point the new dir entry to the newly created directory
			dp->name_len = strlen(child);	//make the name as long as the user requested
			dp->rec_len = BLKSIZE;	//since this is the last (and first) entry on the block, it will span the entire data block
			for (int j = 0; j < dp->name_len; j++) { dp->name[j] = child[j]; }	//copy in the directory name

			put_block(pmip->dev, pmip->INODE->i_block[i], buf2);	//we are done modifying the entry, now save it to disk
			return;
		}

		get_block(pmip->dev, pmip->INODE->i_block[i], buf);		//get the data from the i_block and put it into buf/

		dp = (DIR *)buf;	//creating a DIR struct from buf data block
		cp = buf;

		// Finds end of the parent where the child will be placed
		while (cp + dp->rec_len < buf + BLKSIZE){
			cp += dp->rec_len;		// advance cp to the next entry (by rec_len)
			dp = (DIR *)cp;			//have the dir entry follow along
		}

		// dp now points to the last entry in the data block
		lastEntryIdealSize = (4 * ((8 + dp->name_len + 3) / 4));
		remain = dp->rec_len - lastEntryIdealSize;		//this is how much space is left of the particular data block we are on

		// If there is enough space to insert the new directory in this data block, then insert it
		if (remain >= need_length){
			dp->rec_len = lastEntryIdealSize;	//trim the current last directory entry's rec_len to its ideal length
												//dp = (char *)dp + dp->rec_len;	//shift down the rec_len of the last dir entry to create a new dir entry after it
			cp += dp->rec_len;			// advance cp to the next entry (by rec_len)
			dp = (DIR *)cp;				//have the dir entry follow along
			dp->inode = ino;			//point the new dir entry to the newly created directory
			dp->name_len = strlen(child);	//make the name as long as the user requested
			dp->rec_len = remain;		//last directory on data block will span the rest of the data block
			for (int j = 0; j < dp->name_len; j++) { dp->name[j] = child[j]; }	//copy in the directory name

			put_block(pmip->dev, pmip->INODE->i_block[i], buf);	//we are done modifying the entry, now save it to disk
			return;
		}
	}
}

//a helper function to mkdir which creates the new directory
void mkNewDir(MINODE *pmip, char *child)
{
	//Part 1: Allocate Inode and Disk Block:
	MINODE *mip;
	DIR *dp;
	char *cp;
	char buf[BLKSIZE];
	bzero(buf, BLKSIZE);

	int ino = ialloc(mtable[0].dev);
	int blk = balloc(mtable[0].dev);

	if (!ino) { printf("There are no more free inodes, cannot mkdir\n"); return; }
	if (!blk) { printf("There are no more free data blocks, cannot mkdir\n"); return; }

	//Part 2: Create INODE:
	mip = iget(mtable[0].dev, ino);
	mip->INODE->i_mode = 0x41ED; // 040755: DIR type and permissions
	mip->INODE->i_uid = running->uid; // owner uid
	mip->INODE->i_gid = running->pid; // group Id
	mip->INODE->i_size = BLKSIZE; // size in bytes
	mip->INODE->i_links_count = 2; // links count=2 because of . and ..
	mip->INODE->i_atime = mip->INODE->i_ctime = mip->INODE->i_mtime = time(0);
	mip->INODE->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
	mip->INODE->i_block[0] = blk; // new DIR has one data block set as the one we allocated for it	
	for (int i = 1; i < 15; i++) { mip->INODE->i_block[i] = 0; }	//new dir only has one data block so set the rest to empty (0)/
	mip->dirty = 1; // mark minode dirty
	iput(mip); // write INODE to disk

			   //Part 3: Create data block for new DIR containing . and .. entries:
	dp = (DIR *)buf;
	cp = buf;

	// make "." entry:
	dp->inode = ino;	//allocate . to itself (because dir entry "." is a loopback to itself)
	dp->rec_len = 12;	//size of entry, refer to ideal_length algorithm
	dp->name_len = 1;	//name . is only one char long
	dp->name[0] = '.';	//allocate "." as the name	

						//make ".." entry:
	cp += dp->rec_len; // advance cp to the next entry (by rec_len)
	dp = (DIR *)cp;		//have the dir entry follow along
						//dp = (char *)dp + 12;	//shift down 12 from "." entry (rec_len of ".")
	dp->inode = pmip->ino;	//point ".." to the parent directory (because dir entry ".." points to parent directory)
	dp->rec_len = BLKSIZE - 12; //last directory on data block will span the rest of the data block (subtracting 12 because that is the size of ".")
	dp->name_len = 2;	//name .. is 2 chars long
	dp->name[0] = dp->name[1] = '.';	//set the name of the entry to ".."

	put_block(mtable[0].dev, blk, buf); // rewrite updated data block back to the disk

										//Part 4: Enter child into parent directory:
	enter_child(pmip, ino, child);
}

//creates a new directory at 'pathname'
void my_mkdir(char *pathname)
{
	char *parentDir;
	char *child;
	int pino;
	MINODE *pmip;

	if (!pathname)	{
		printf("Must include a name\n");
		return;
	}

	//char *pathnameCpy1 = malloc((strlen(pathname) + 1) * sizeof(char));
	//char *pathnameCpy2 = malloc((strlen(pathname) + 1) * sizeof(char));/

	char *pathnameCpy1;
	char *pathnameCpy2;

	pathnameCpy1 = malloc(strlen(pathname) * sizeof(char));
	pathnameCpy2 = malloc(strlen(pathname) * sizeof(char));

	strcpy(pathnameCpy1, pathname);
	strcpy(pathnameCpy2, pathname);
	//strcpy(parentDir, dirname(pathnameCpy1));
	//strcpy(child, basename(pathnameCpy2));

	parentDir = dirname(pathnameCpy1);
	child = basename(pathnameCpy2);

	//printf("parentDir = %s   child name = %s\n", parentDir, child);

	if ((parentDir == "/") && (child == "/"))	{
		printf("Must include a name\n");
		return;
	}

	if (getino(pathname)) { printf("%s already exists, cannot mkdir\n", pathname); return; }
	if (!strcmp(parentDir, ".")) { pino = running->cwd->ino; }
	else if (!(pino = getino(parentDir))) { printf("Parent %s does not exist, cannot mkdir\n", parentDir); return; }

	pmip = iget(mtable[0].dev, pino);
	if ((pmip->INODE->i_mode & 0xF000) != 0x4000) { printf("Parent %s is not a direcory, cannot mkdir\n", parentDir); iput(pmip); return; }

	//printf("Parent directory inode number is: %d\n", pino);

	mkNewDir(pmip, child);
	pmip->INODE->i_links_count++;
	pmip->dirty = 1;
	iput(pmip);

}
