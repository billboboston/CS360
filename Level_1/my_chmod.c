/* Linux EXT2 file system
* Semester Project for CS_360 (Fall-17)
* Professor: KC Wang
* 
* Authors: Trent & William
*/

#include "my_cd.c"

// my_chmod: only takes in the octal number see: < www.onlineconversion.com/html_chmod_calculator.htm >
void my_chmod(char *line) {
	char *Mode;
	char *pathname;
	
	// parsing input string
	strtok(line, " ");
	pathname = strtok(NULL, " ");
	Mode = strtok(NULL, " "); // getting mode # as a string

	if (pathname == NULL) { printf("Error: No Filename given\n");	return; }

	if (Mode == NULL) { // default value is -rw-------
		Mode = "200";
	}

	int i;
	sscanf(Mode, "%o", &i); // convert mode into an octal numer(how the data is used)
	int ino;
	ino = getino(pathname);

	if (ino == 0) { printf("Error: File does not Exist\n");	return; }

	MINODE *mip;
	mip = iget(mtable[0].dev, ino); // get node that is being modified
	mip->INODE->i_mode = (mip->INODE->i_mode & 0xFE00) | i; // setting last 9 bytes (or 3 sections in octal) of the i_mode member

	mip->dirty = 1; // marking dirty bc node was changed
	iput(mip); 

}
