#include "../helperFunctions.c"

// my_cat: displays the contents of a file from the input file name
void my_cat(char *filename) {
	// check given filename
	if (filename == NULL) { printf("Error: No Filename given\n");	return; }

	// check if file exists
	int ino;
	ino = getino(filename);
	if (ino == 0) { printf("Error: File does not Exist\n");	return; }

	// open file
	int fd = my_open(filename, 0);	// open filename to read=0
	if (fd != -1) {
		char *buf = malloc(running->fd[fd]->mptr->INODE->i_size + 1);		// if we have errors, check here first lol
																			// Read fd[size] into buf[fd.size+1]
		my_read(fd, buf, running->fd[fd]->mptr->INODE->i_size);
		printf("%s", buf);
		my_close(fd);
	}
}
