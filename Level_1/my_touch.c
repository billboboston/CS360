#include	"my_symlink.c"

// my_touch: updates
// Input: 
void touch(char *pathname) {

	if (pathname == NULL) { printf("Error: No Filename given\n");	return; }

	int ino;
	MINODE *mip;

	ino = getino(pathname);
	if (ino == 0) { my_creat(pathname);	return; } // if file dosent exist create one

	mip = iget(mtable[0].dev, ino); // get iNode from inode number
	mip->INODE->i_atime = time(0);	// update the time to current time

	mip->dirty = 1; // dirty because it was changed
	iput(mip);
}
