#include "my_mv.c"


//helper that gets returns the first open fd or checks if the file is already open
int get_fd(int ino) {
	int j = -1;
	for (int i = 0; i < 16; i++) {
		if (running->fd[i] == NULL && j == -1) {
			j = i;
		}
		if (running->fd[i] != NULL && running->fd[i]->mptr->ino == ino) {
			return i;
		}
	}
	return j;
}

// R | W | RW | Append
// 0 | 1 | 2  | 3
// my_open: opens a file if it exists and stores it in the fd array
int my_open(char  *filename, int flags) {
	// 1. Get file minode
	if (filename == NULL) { printf("Error Opening File: Bad File Name\n");	return -1; }

	int ino;
	MINODE *mip;
	ino = getino(filename);

	if (ino == 0) {
		newCreat(filename);
		ino = getino(filename);
	}

	// 2. check file permissions
	mip = iget(mtable[0].dev, ino);
	if ((mip->INODE->i_mode & 0xF000) != 0x8000) { printf("Error: Object is not a File, cannot open"); iput(mip); return -1; }

	// 3. allocate an opentable entry 
	OFT *openTable = malloc(sizeof(OFT));

	openTable->mode = flags;
	if (flags == 3) {
		//printf("[open] inode size = %d\n", mip->INODE->i_size);
		openTable->offset = mip->INODE->i_size;
		//printf("[open] file descriptor offset = %d\n", openTable->offset);
	}
	else {
		openTable->offset = 0;
	}

	openTable->refCount = 1;
	openTable->mptr = mip;

	// 4. Search for free FD
	int fd = get_fd(ino);
	if (fd == -1) { printf("Error Opening File: No free openTable slot\n"); iput(mip); return -1; }
	running->fd[fd] = openTable;

	// 5. unlock minode and return fd as a file descriptor
	//fd->mptr->INODE->lock = 0;	// this is how this should work but we dont have a lock member in INODE like in his diagrams
	return fd;
}

// sysCall handler for the my_open functionality
void mySys_open(char *line) {

	char *lineCpy = malloc(strlen(line) * sizeof(char));
	strcpy(lineCpy, line);

	char *filepath;
	int flag;

	strtok(lineCpy, " ");
	filepath = strtok(NULL, " ");
	char *temp = strtok(NULL, " ");

	flag = atoi(temp);
	//sscanf(lineCpy,"%d", flag);

	int fd = my_open(filepath, flag);
	printf(">Opend FileDescriptor: %d\n", fd);
}

