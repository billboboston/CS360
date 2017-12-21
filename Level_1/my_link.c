/* Linux EXT2 file system
* Semester Project for CS_360 (Fall-17)
* Professor: KC Wang
* 
* Authors: Trent & William
*/

#include "my_creat.c"

// my_link: link old_file new_file
void my_Link(char *line) {
	char *old_file;
	char *new_file;
	char *pathnames;

	strtok(line, " ");

	int oino;
	int pino;
	MINODE *omip;
	MINODE *pmip;

	old_file = strtok(NULL, " ");
	new_file = strtok(NULL, " ");

	char *parentDir;
	char *child;
	char *pathnameCpy1;
	char *pathnameCpy2;

	if (!new_file) { printf("Must include a name\n"); return; } // name check

	pathnameCpy1 = malloc(strlen(new_file) * sizeof(char));
	pathnameCpy2 = malloc(strlen(new_file) * sizeof(char));

	strcpy(pathnameCpy1, new_file);
	strcpy(pathnameCpy2, new_file);

	parentDir = dirname(pathnameCpy1);	// parse line for path
	child = basename(pathnameCpy2);		// parse line for file

	// old file: check name & get file & type check
	if ((oino = getino(old_file)) == 0) { printf("%s does not exist, cannot link\n", old_file); return; }
	omip = iget(mtable[0].dev, oino);	// get old_fie
	if ((omip->INODE->i_mode & 0xF000) == 0x4000) { printf("%s is a directory, cannot link\n", old_file); iput(omip); return; }

	// parent node: check name & get file & type check
	if ((pino = getino(parentDir)) == 0) { printf("%s does not exist, cannot link\n", parentDir); iput(omip); return; }
	pmip = iget(mtable[0].dev, pino);
	if ((pmip->INODE->i_mode & 0xF000) != 0x4000) { printf("%s is not a directory, cannot link\n", parentDir); iput(pmip); iput(omip); return; }

	// search for parent
	if (~(search(pmip, child)))	{
		enter_child(pmip, oino, child);
		omip->INODE->i_links_count++;
		omip->dirty = 1;
	}

	printf("Sucessfully linked %s -> %s\n", old_file, new_file);

	iput(omip);
	iput(pmip);
}
