/* Linux EXT2 file system
* Semester Project for CS_360 (Fall-17)
* Professor: KC Wang
* 
* Authors: Trent & William
*/

#include	"my_stat.c"

// my_symlink: creates a symbolic link from one file to another
// Input: string of the whole input line including the command
void my_symlink(char *line) {
	char *old_file;
	char *new_file;
	char *pathnames;

	strtok(line, " ");

	int oino;
	int pino;
	int ino;

	MINODE *omip;
	MINODE *pmip;
	MINODE *mip;

	old_file = strtok(NULL, " ");
	new_file = strtok(NULL, " ");

	if ((oino = getino(old_file)) == 0) { printf("%s does not exist, cannot syslink\n", old_file); return; }
	if ((ino = getino(new_file)) != 0) { printf("%s already exists, cannot syslink\n", new_file); return; }

	newCreat(new_file);
	if ((ino = getino(new_file)) == 0) { return; }	//return if it couldnt make a new file

	mip = iget(mtable[0].dev, ino);
	mip->INODE->i_mode = (mip->INODE->i_mode & 0x0FFF) | 0xA000; //change to LNK type
	mip->INODE->i_size = strlen(old_file); //size is the name length of old file

	int j = -1;
	for (int i = 0; i < strlen(old_file); i++)
	{
		if (i % 4 == 0) { j++; }
		mip->INODE->i_block[j] |= (old_file[i] << (i % 4) * 8);
	}

	printf("Sucessfully linked %s -> %s\n", old_file, new_file);
	mip->dirty = 1;
	iput(mip);
}

