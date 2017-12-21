#include	"my_link.c"



// ls_file: displays data from the input iNodes
//	-- a helper function to ls_dir and ls where it will ls a file (or dir but not the contents of dir)
//Input: ino is current inode, pino is parent inode
void ls_file(int ino, int pino) {
	char *name;	//name of the inode
	int symb = 0;	//symbolic file indicator
	int parent_ino;
	char buf[BLKSIZE];
	int nameLength;
	DIR *dp;
	char *cp;
	MINODE *mip;

	mip = iget(mtable[0].dev, ino);

	char *t1 = "xwrxwrxwr-------";
	char *t2 = "----------------";

	//writing if its dir, reg, or link...
	if ((mip->INODE->i_mode & 0xF000) == 0x8000) { printf("%c", '-'); }
	if ((mip->INODE->i_mode & 0xF000) == 0x4000) { printf("%c", 'd'); }
	if ((mip->INODE->i_mode & 0xF000) == 0xA000) { printf("%c", 'l'); symb = 1; }
	//if ((mip->INODE->i_mode & 0xF000) == 0xA000) { symb = 1; }	//its a symbolic file/

	//writing the permission bits...
	for (int i = 8; i >= 0; i--)
	{
		if (mip->INODE->i_mode & (1 << i)) { printf("%c", t1[i]); }
		else { printf("%c", t2[i]); }
	}

	printf("%4d ", mip->INODE->i_links_count);
	printf("%4d ", mip->INODE->i_gid);
	printf("%4d ", mip->INODE->i_uid);
	printf("%8d ", mip->INODE->i_size);

	//print the time
	char tmeStr[64];
	time_t inodeTime = mip->INODE->i_ctime;
	strcpy(tmeStr, ctime(&inodeTime));
	tmeStr[strlen(tmeStr) - 1] = 0;
	printf("%s  ", tmeStr);

	iput(mip);	//make sure to throw away in-memory inode

	mip = iget(mtable[0].dev, pino);	//get in-memory inode of my parent

										/* now try to get my name */
	get_block(mtable[0].dev, mip->INODE->i_block[0], buf);	//load in parent directory into buf
	dp = (DIR *)buf;	//creating a DIR struct from buf data block
	cp = buf;

	//looking for my inode number in my parent dir to get my name...
	while (cp < buf + BLKSIZE)
	{
		if (dp->inode == ino)
		{
			//found my inode number so get my name
			nameLength = (int)dp->name_len;
			for (int i = 0; i < nameLength; i++) { putchar(dp->name[i]); }
			break;
		}
		cp += dp->rec_len; // advance cp to the next entry (by rec_len)
		dp = (DIR *)cp;		//have the dir entry follow along
	}

	/* NOTE: I dont know if we need to print if its a symbolic file or not so ill just leave this template*/
	// print -> linkname if it's a symbolic file
	if (symb)
	{
		mip = iget(mtable[0].dev, ino);
		char curChar;
		int j = -1;
		int i = 0;

		printf(" -> ");

		while (1)
		{
			if (i % 4 == 0) { j++; }

			curChar = (mip->INODE->i_block[j] >> ((i % 4) * 8)) & 0xff;

			if (curChar == '\0') { break; }
			putchar(curChar);

			i++;
		}
	}

	printf("\n");
	iput(mip);	//make sure to throw away in-memory inode at the end
}

// ls_dir: iterates through all the dir items and calls ls_file to print
// Input: MiNode
void ls_dir(MINODE *mip) {
	char buf[BLKSIZE];
	char *cp;	//used for traversing through entries
	DIR *dp;
	int j = 0;

	//iterate through the directory (skipping . and ..)
	for (int i = 0; i < 14; i++)
	{
		get_block(mip->dev, mip->INODE->i_block[i], buf);	//load in current directory into buf
		dp = (DIR *)buf;	//creating a DIR struct from buf data block
		cp = buf;

		if (mip->INODE->i_block[i])
		{
			while (cp < buf + BLKSIZE)
			{
				if (j > 1) { ls_file(dp->inode, mip->ino); }//ls the current file we are on

				cp += dp->rec_len; // advance cp to the next entry (by rec_len)
				dp = (DIR *)cp;	//have the dir entry follow along
				j++;
			}
		}
	}
}

// my_ls: parses and checks the input path then calls ls_dir
// Input: char* of the path to display contents of
void my_ls(char *pathname) {
	int ino;
	int i = 0;	//current char of pathname
	int j = 0;	//location of last '/'
	MINODE *mip;

	if (pathname == NULL){
		//ls the current working directory
		ls_dir(running->cwd);
		return;
	}

	ino = getino(pathname);
	if (ino == 0) { return; }	//the pathname doesnt exist

	mip = iget(mtable[0].dev, ino);

	if ((mip->INODE->i_mode & 0xF000) == 0x4000) { ls_dir(mip); }	//'pathname' is a directory so ls everything inside 
	else{
		//if it is not a directory, remove self from pathname string to get the parent inode number
		while (pathname[i])	{
			if (pathname[i] == '/') { j = i; }
			i++;
		}

		if (j == 0) { ls_file(ino, running->cwd->ino); }
		else{
			for (i = j + 1; i < strlen(pathname); i++)
			{
				pathname[i] = '\0';
			}
			ls_file(ino, getino(pathname));
		}
	}

	iput(mip);	//make sure to throw away the in-memory inode
}


