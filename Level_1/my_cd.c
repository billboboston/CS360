#include "Mount_Root.c"

//changes working directory to the inputted pathname, if no pathname is given, will change to root
void cd(char *pathname){
	if (pathname == NULL){
		//there was no pathname included so change directory to root
		running->cwd = root;
		return;
	}

	int ino;
	MINODE *mip;

	ino = getino(pathname);
	if (ino == 0) { return; }	//pathname didnt exist

	mip = iget(mtable[0].dev, ino);

	//verify that mip->INODE is a directory...
	if ((mip->INODE->i_mode & 0xF000) != 0x4000) { printf("path %s is not a direcory, cannot cd\n", pathname); iput(mip); return; }

	iput(running->cwd);		//remove old cwd
	running->cwd = mip;		//assign new cwd
}
