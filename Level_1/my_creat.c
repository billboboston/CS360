#include "my_chmod.c"	

// newCreat: Creation of child file happens here.
// input: takes parent iNode Pointer and the name of the new file
void newCreat(MINODE *pmip, char *child) {
	//Part 1: Allocate Inode and Disk Block:
	MINODE *mip;
	DIR *dp;
	char *cp;
	char buf[BLKSIZE];
	bzero(buf, BLKSIZE);

	int ino = ialloc(mtable[0].dev);

	//printf("[my_creat] allocated inode number %d for %s\n", ino, child);

	//int blk = balloc(mtable[0].dev);								// touch

	//printf("allocated data block number: %d and inode number: %d for %s\n", blk, ino, child);

	if (!ino) { printf("There are no more free inodes, cannot creat\n"); return; }
	//if (!blk) { printf("There are no more free data blocks, cannot creat\n"); return; }								// touch

	//Part 2: Create INODE: Init of iNode Data
	mip = iget(mtable[0].dev, ino);
	mip->INODE->i_mode = 0x81a4; // 040755: DIR type and permissions
	mip->INODE->i_uid = running->uid; // owner uid
	mip->INODE->i_gid = running->pid; // group Id
	mip->INODE->i_size = 0; // size in bytes
	mip->INODE->i_links_count = 1; // links count=2 because of . and ..								// touch
	mip->INODE->i_atime = mip->INODE->i_ctime = mip->INODE->i_mtime = time(0);
	mip->INODE->i_blocks = 0; // LINUX: Blocks count in 512-byte chunks
	mip->INODE->i_block[0] = 0; // new DIR has one data block set as the one we allocated for it	
	for (int i = 1; i < 15; i++) { mip->INODE->i_block[i] = 0; }	//new dir only has one data block so set the rest to empty (0)/
	mip->dirty = 1; // mark minode dirty
	iput(mip); // write INODE to disk

	//Part 3: Create data block for new DIR containing . and .. entries:
	dp = (DIR *)buf;
	cp = buf;

	//Part 4: Enter child into parent directory:
	enter_child(pmip, ino, child);
}

// my_creat: seperates line data to get parent iNode and child string 
//Input: "pathname/filename" of new file.
void my_creat(char *pathname) {
	char *parentDir;
	char *child;
	int pino;
	MINODE *pmip;

	if (!pathname){		// checks for a path and/or fileName
		printf("Must include a name\n");
		return;
	}

	char *pathnameCpy1; 
	char *pathnameCpy2;

	pathnameCpy1 = malloc(strlen(pathname) * sizeof(char));
	pathnameCpy2 = malloc(strlen(pathname) * sizeof(char));

	strcpy(pathnameCpy1, pathname);
	strcpy(pathnameCpy2, pathname);

	parentDir = dirname(pathnameCpy1);		// gets path where file should be placed
	child = basename(pathnameCpy2);			// gets the new file name

	if ((parentDir == "/") && (child == "/")){	// only input a path and no name
		printf("Must include a name\n");
		return;
	}

	if (getino(pathname)) { printf("%s already exists, cannot creat\n", pathname); return; }	// chacks if file already exists
	if (!strcmp(parentDir, ".")) { pino = running->cwd->ino; }									// if CWD is the parent dir to the new file
	else if (!(pino = getino(parentDir))) { printf("Parent %s does not exist, cannot creat\n", parentDir); return; } // else finding new parent dir

	pmip = iget(mtable[0].dev, pino);
	if ((pmip->INODE->i_mode & 0xF000) != 0x4000) { printf("Parent %s is not a direcory, cannot creat\n", parentDir); iput(pmip); return; }

	newCreat(pmip, child); // passes data into the create helper

	pmip->dirty = 1;
	iput(pmip);
}
