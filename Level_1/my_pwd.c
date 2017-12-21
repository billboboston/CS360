#include "my_mkdir.c"

// r_pwd: Recursively prints bakc to root from the Working Directory
// Input: MINODE of the current direrctory
void r_pwd(MINODE *wd) {
	if (wd == root) { return; }

	/* search for my current inode number */
	char buf[BLKSIZE];
	char *cp;	//used for traversing through entries
	int parent_ino;
	DIR *dp;
	MINODE *mip;

	get_block(wd->dev, wd->INODE->i_block[0], buf);	//load in current directory into buf
	dp = (DIR *)buf;	//creating a DIR struct from buf data block
	cp = buf;
		
	while (cp < buf + BLKSIZE)	{
		if ((strncmp("..", dp->name, 2) == 0) && (dp->name_len == 2)){		//looking for parent inode number...
			parent_ino = dp->inode;		//found my parent inode number
			break;
		}
		cp += dp->rec_len; // advance cp to the next entry (by rec_len)
		dp = (DIR *)cp;		//have the dir entry follow along
	}

	// get current iNode
	mip = iget(mtable[0].dev, parent_ino);	//write parent directory into memory
	get_block(mip->dev, mip->INODE->i_block[0], buf);	//load in parent directory into buf
	dp = (DIR *)buf;	//creating a DIR struct from buf data block
	cp = buf;

	char name[256];

	
	while (cp < buf + BLKSIZE){ //searcing through current iNode for cur name
		if (dp->inode == wd->ino){
			for (int i = 0; i < dp->name_len; i++) { name[i] = dp->name[i]; }		//found my inode number so get my name
			name[dp->name_len] = '\0';	//assign NULL terminator to end
			break;
		}
		cp += dp->rec_len; // advance cp to the next entry (by rec_len)
		dp = (DIR *)cp;		//have the dir entry follow along
	}

	rpwd(mip);
	iput(mip);	//remove mip
	printf("/%s", name);
}

// pwd: dir checks to call r_pwd
// Input: MINODE of the CWD
void pwd(MINODE *wd){
	if (wd == root) { printf("/"); }
	else { r_pwd(wd); }
	printf("\n");
}
