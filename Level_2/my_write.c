#include "my_read.c"

// ----------------------------------------- WRITE 

int my_write(int fd, char *buf, int nbytes) {
	int count = 0;	//number of bytes read
	int avil;	//bytes avalible in file
	int lbk;	//logical block of where file is
	int blk;	//the physical block number of where the file is
	int startByte;	//the starting byte of the file within the logical block
	char tmpbuf[BLKSIZE];		//temp storage for copying
	int copied = 0;	//number of bytes copied
	int endIndex;

	if (running->fd[fd] == NULL) {
		printf("[my_write] file descriptor invalid, cannot continue\n");
		return -1;
	}

	//Only continue if the file opened had flags for W, RW, or APPEND:
	if (running->fd[fd]->mode == 0) {
		printf("[my_write] file descriptor incorrect mode, cannot continue\n");
		return -1;
	}

	//printf("[write] old file size is %d\n", running->fd[fd]->mptr->INODE->i_size);

	//if the file is bigger after writing to it, update file size
	if (running->fd[fd]->mptr->INODE->i_size < running->fd[fd]->offset + nbytes) {
		running->fd[fd]->mptr->INODE->i_size += running->fd[fd]->offset + nbytes;
	}

	//printf("[write] new file size is %d\n", running->fd[fd]->mptr->INODE->i_size);

	avil = strlen(buf);	//compute amout of bytes left in file

						//read until it has read all of the bytes that the user requested or until the entire file has been read
	while (0 < nbytes || 0 < avil) {
		lbk = running->fd[fd]->offset / BLKSIZE;		//calculates the logical block
		startByte = (running->fd[fd]->offset % BLKSIZE);	//calculates the start bit

															//printf("[write] byte offset is %d\n", running->fd[fd]->offset);

															//if ((blk = lb_2_pb(running->fd[fd]->mptr->INODE, lbk)) == -1) { return; }	//get physical block number
		if ((blk = lb_2_pb(running->fd[fd]->mptr, lbk)) == -1) { return; }
		//printf("[write] writing to data block number %d\n", blk);
		get_block(running->fd[fd]->mptr->dev, blk, tmpbuf);		//get the block data of file

		if (startByte + nbytes <= BLKSIZE) {
			endIndex = startByte + nbytes;
		}
		else { endIndex = BLKSIZE; }

		if (copied == 0) {	//its the first time copying, so use strcpy
			strncpy(tmpbuf + startByte, buf, endIndex - startByte);
		}
		else {	//it isnt the first time copying, so use strcat
			strncat(tmpbuf + startByte, buf, endIndex - startByte);
		}

		copied = endIndex - startByte;	//calculate how many bytes were just copied
		running->fd[fd]->offset += copied;
		count += copied;
		nbytes -= copied;
		avil -= copied;
		put_block(running->fd[fd]->mptr->dev, blk, tmpbuf);
	}
	running->fd[fd]->mptr->dirty = 1;

	return count;
}

// syscall handeler for write
void mySys_write(char *pathname) {
	int fd = atoi(pathname);
	if (running->fd[fd == NULL]) { printf("Error: File Descriptor isnt linked to a file"); return; }
	char line[256];
	printf("=================== Text to Write =====================\n");
	fgets(line, 256, stdin);
	my_write(fd, line, strlen(line));

}
