/* Linux EXT2 file system
* Semester Project for CS_360 (Fall-17)
* Professor: KC Wang
* 
* Authors: Trent & William
*/

#include	"my_touch.c"

// my_unlink: unlinks link between files or removes/deletes a file
// Input: path of file to remove
void my_unlink(char *inPath)
{
	char *pathname = malloc(strlen(inPath) * sizeof(char));
	strcpy(pathname, inPath);
	int ino;
	int pino;
	MINODE *mip;
	MINODE *pmip;

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

	if ((ino = getino(pathname)) == 0) { printf("%s does not exist, cannot unlink\n", pathname); return; }

	mip = iget(mtable[0].dev, ino);
	if ((mip->INODE->i_mode & 0xF000) == 0x4000) { printf("%s is a directory, cannot unlink\n", pathname); iput(mip); return; }

	pino = getino(parentDir);
	pmip = iget(mtable[0].dev, pino);

	rm_child(pmip, child);		//remove it from parent

	pmip->dirty = 1;
	iput(pmip);

	mip->INODE->i_links_count--;

	if (mip->INODE->i_links_count > 0) { mip->dirty = 1; }	//if links_count > 0 then its still being referenced somewhere so dont delete it
	else
	{
		for (int i = 0; i < 14; i++)
		{
			if (mip->INODE->i_block[i])
			{
				bdalloc(mip->dev, mip->INODE->i_block[i]);	//deallocate all data blocks associated with the inode
			}
		}
		idalloc(mip->dev, ino);		//deallocate the inode
	}
	iput(mip);
}
