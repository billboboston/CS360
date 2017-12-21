#include "my_cat.c"


int my_close(int fd) {
	// 1. check fd is a valid opened file descriptor;
	if (running->fd[fd] == NULL) { printf("Error Closing File: file descriptor\n"); return -1; }

	// 2. if (PROC's fd[fd] != 0) 		// redundant check
	// 3. opentable check for RW pipe 	// dont need this

	OFT *openTable = (running->fd[fd]);
	openTable->refCount -= 1;

	// 4. if last process using this OFT
	if (openTable->refCount == 0) {
		iput(openTable->mptr);
		// 5. clear fd[fd]
		running->fd[fd] = NULL;
	}

	// 6. return SUCCESS
	return 1;
}
// syscall handeler for close
void mySys_close(char *filename) {
	int fd = atoi(filename);
	printf("> Closed FileDescriptor: %d\n", fd);
	my_close(fd);
}