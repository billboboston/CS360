
#include "my_pwd.c"

// isDir_empty: checks through mip and searches for anything other than '.' & '..'
// Input:
// checks through mip and returns false if there is anything more than '.' & '..'
int isDir_empty(MINODE *mip) {
	char buf[BLKSIZE];
	char *cp;	//used for traversing through entries
	DIR *dp;
	int j = 0;

	//iterate through the directory (skipping . and ..)
	for (int i = 0; i < 12; i++) {
		get_block(mip->dev, mip->INODE->i_block[i], buf);	//load in current directory into buf
		dp = (DIR *)buf;	//creating a DIR struct from buf data block
		cp = buf;

		if (mip->INODE->i_block[i])	{
			while (cp < buf + BLKSIZE)	{
				cp += dp->rec_len; // advance cp to the next entry (by rec_len)
				dp = (DIR *)cp;	//have the dir entry follow along
				j++;
			}
		}
	}

	if (j > 2) { return 0; }
	else { return 1; }
}

// rm_child: removes 'name' using the parent iNode Pointer
// Input: parint Inode, name of the child to remove
int rm_child(MINODE *pmip, char *name) {

	// search not necessary b/c name is the item # in the dir
	char buf[BLKSIZE];
	char *cp;	//used for traversing through entries
	char *cpPrev;
	DIR *dp;

	int rmvDirIdealSize;

	//search for my name in parent directory
	for (int i = 0; i < 12; i++)
	{
		//only search in non-empty blocks
		if (pmip->INODE->i_block[i])
		{
			get_block(pmip->dev, pmip->INODE->i_block[i], buf);		//get the data from the i_block and put it into buf

			dp = (DIR *)buf;	//creating a DIR struct from buf data block
			cp = buf;

			//search the whole block
			while (cp < buf + BLKSIZE)	{
				if ((strncmp(name, dp->name, dp->name_len) == 0) && (strlen(name) == dp->name_len))	{
					//it found the inode in the directory matching 'name'
					rmvDirIdealSize = (4 * ((8 + dp->name_len + 3) / 4));

					if (dp->rec_len == BLKSIZE)		{
						//it is the only directory on the data block
						char buf2[BLKSIZE];
						bzero(buf2, BLKSIZE);
						bdalloc(pmip->dev, dp->inode);	//deallocating data block	
						pmip->INODE->i_block[i] = 0;

						pmip->INODE->i_size -= BLKSIZE;	//since we are removing a data block from the parent we need to de-increment the size by BLKSIZE

														//if there are more data blockd being used by the parent, shift them over
						int j = i;
						while (pmip->INODE->i_block[j + 1])	{
							pmip->INODE->i_block[j] = pmip->INODE->i_block[j + 1];
							pmip->INODE->i_block[j + 1] = 0;
						}

						return 1;
					}

					if ((dp->rec_len > rmvDirIdealSize) || (cp + dp->rec_len == buf + BLKSIZE))	{
						// it is the last direcotry on the data block (but not only one on the block)

						int rmDirRec_len = dp->rec_len;
						dp = (DIR *)cpPrev;	//go to the entry before me
						dp->rec_len += rmDirRec_len;	//absorb the rmdir rec_len into the previous entry (now last)
						put_block(pmip->dev, pmip->INODE->i_block[i], buf);
						return 1;
					}

					if (dp->rec_len == rmvDirIdealSize)	{
						//it is in the middle or it is the first (but not only one on the block)
						char *cpSft;	//used for traversing through entries
						DIR *dpSft;
						DIR dirCpy;

						cpSft = cp + rmvDirIdealSize;
						dpSft = (DIR *)buf;
						dpSft = (DIR *)cpSft;

						int nextRecLen;
						int nextIdealSize;

						while (1)	{
							nextRecLen = dpSft->rec_len;
							nextIdealSize = (4 * ((8 + dpSft->name_len + 3) / 4));

							memcpy(dp, cpSft, dpSft->rec_len);

							if (nextRecLen > nextIdealSize)	{
								//the one after the current entry being copied is the last entry
								dp->rec_len += rmvDirIdealSize;	//add the deleted space to the end of last entry
								break;
							}

							cpSft += dpSft->rec_len; // shift over one
							dpSft = (DIR *)cpSft;

							cp += dp->rec_len;
							dp = (DIR *)cp;
						}

						put_block(pmip->dev, pmip->INODE->i_block[i], buf);
						return 1;
					}

				}
				cpPrev = cp;
				cp += dp->rec_len; // advance cp to the next entry (by rec_len)
				dp = (DIR *)cp;		//have the dir entry follow along
			}
		}
	}


	return 0;
}

// my_rmdir: checks input pathname and gets the iNode fromt the parent data block
// Input: string with the "pathname/filename"
void my_rmdir(char *pathname) {
	int pino, ino;
	MINODE *pmip;
	MINODE *mip;

	char name[256];

	char buf[BLKSIZE];
	char *cp;	//used for traversing through entries
	DIR *dp;

	char *parentDir;
	char *child;
	char *pathnameCpy1;
	char *pathnameCpy2;

	if (!pathname) { printf("Must include a name\n"); return; }

	pathnameCpy1 = malloc(strlen(pathname) * sizeof(char));
	pathnameCpy2 = malloc(strlen(pathname) * sizeof(char));

	strcpy(pathnameCpy1, pathname);
	strcpy(pathnameCpy2, pathname);

	parentDir = dirname(pathnameCpy1);
	child = basename(pathnameCpy2);

	if ((parentDir == "/") && (child == "/"))
	{
		printf("Must include a name\n");
		return;
	}

	// 1 /* Get in-memory INODE of Pathname */

	if (!(ino = getino(pathname))) { printf("%s does not exist, cannot rmdir\n", pathname); return; }

	//ino = getino(pathname);
	mip = iget(mtable[0].dev, ino);

	if ((mip->INODE->i_mode & 0xF000) != 0x4000) { printf("%s is not a directory, cannot remove\n", pathname); iput(mip); return; }

	// 2	/* Verify INODE is a DIR by (INODE.i_mode filed)*/
	if (mip->refCount > 1) { printf("%s is Busy and cannot be deleted\n", pathname); return; }
	if (!isDir_empty(mip)) { printf("%s is not an empty directory, cannot remove\n", pathname); iput(mip); return; }

	// 3	/* Get parents ino and inode */
	if (!strcmp(parentDir, ".")) { pino = running->cwd->ino; }
	else if (!(pino = getino(parentDir))) { printf("Parent %s does not exist, cannot mkdir\n", parentDir); iput(mip); return; }
	else { pino = getino(parentDir); }
	pmip = iget(mip->dev, pino);

	// 4	/* Get name from parent DIR's data blcok */	
	for (int i = 0; i < 12; i++)
	{
		get_block(pmip->dev, pmip->INODE->i_block[i], buf);	//load in current directory into buf
		dp = (DIR *)buf;	//creating a DIR struct from buf data block
		cp = buf;

		if (pmip->INODE->i_block[i])
		{
			//looking for my inode number to get my name...
			while (cp < buf + BLKSIZE)
			{
				if (dp->inode == ino)
				{
					//found my inode number so get my name
					for (int j = 0; j < dp->name_len; j++) { name[j] = dp->name[j]; }
					name[dp->name_len] = '\0';	//assign NULL terminator to end
					break;
				}
				cp += dp->rec_len; // advance cp to the next entry (by rec_len)
				dp = (DIR *)cp;		//have the dir entry follow along
			}
		}
	}

	// 5	/*remove name from parent directory */
	rm_child(pmip, name);								// *********** !! I Need Help with my RM_Child !! *********

														// 6	/* Deallocate its data blocks and inode */
	bdalloc(mip->dev, mip->INODE->i_block[0]);
	idalloc(mip->dev, mip->ino);
	iput(mip);

	// 7 	/* Dec parent links_count by 1 : mark parent pimp dirty */
	pmip->INODE->i_links_count--;
	pmip->dirty = 1;
	iput(pmip);

}
