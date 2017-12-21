#include "my_close.c"

// my_cp: copies source to destination using creat
void my_cp(char *line) {
	// check if path is dir 			//TODO
	char *srcPath;
	char *destPath;

	strtok(line, " ");
	srcPath = strtok(NULL, " ");
	destPath = strtok(NULL, " ");

	if (srcPath == NULL) { printf("Error: No Source given\n");	return; }
	if (destPath == NULL) { printf("Error: No Destination given\n");	return; }

	int src_ino = getino(srcPath);
	if (src_ino == 0) { printf("Error: Source File does not Exist\n");	return; }
	MINODE *src_mip = iget(mtable[0].dev, src_ino);

	char *destCpy = malloc(strlen(destPath) * sizeof(char));
	strcpy(destCpy, destPath);

	my_creat(destPath); // creates new empty file at new destination
	int dest_ino;
	if ((dest_ino = getino(destCpy)) == 0) { return; }	//return if it couldnt make a new file
	MINODE *dest_mip = iget(mtable[0].dev, dest_ino);

	dest_mip->INODE->i_size = src_mip->INODE->i_size;
	char buf[BLKSIZE];
	for (int i = 0; i < 14; i++) {
		//only search in non-empty blocks
		if (src_mip->INODE->i_block[i]) {  // adding previous file data to new file data
			dest_mip->INODE->i_block[i] = balloc(mtable[0].dev);
			dest_mip->INODE->i_blocks += 2;
			get_block(mtable[0].dev, src_mip->INODE->i_block[i], buf);
			put_block(mtable[0].dev, dest_mip->INODE->i_block[i], buf);
		}
	}
	dest_mip->dirty = 1;
	iput(dest_mip);
	iput(src_mip);

}
