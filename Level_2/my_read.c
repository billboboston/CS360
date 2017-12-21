#include "my_open.c"

// ----------------------------------------- READ
//converts the logical block number to the physical block number
int lb_2_pb(MINODE *mip, int lbk) {
	if (lbk < 12) { //the logical block was a direct block
		if (mip->INODE->i_block[lbk] == 0) {
			mip->INODE->i_block[lbk] = balloc(mtable[0].dev);
		}
		return mip->INODE->i_block[lbk];
	}
	else if (12 <= lbk < 12 + 256) {	//the logical block was an indirect block
		// we didn't do this :)
	}
	else {	//the logical block was a double indirect block
		// we didn't do this :)
	}
	printf("[lb_2_pb] Could not determine physical block number, aborting now\n");
	return -1;
}

int my_read(int fd, char *buf, int nbytes) {
	int count = 0;	//number of bytes read
	int avil;	//bytes avalible in file
	int lbk;	//logical block of where file is
	int blk;	//the physical block number of where the file is
	int startByte;	//the starting byte of the file within the logical block
	char tmpbuf[BLKSIZE];		//temp storage for copying
	int copied = 0;	//number of bytes copied
	int endIndex;

	if (running->fd[fd] == NULL) {
		printf("[my_read] file descriptor invalid, cannot continue\n");
		return -1;
	}

	//Only continue if the file opened had flags for R or RW:
	if (!(running->fd[fd]->mode == 0 || running->fd[fd]->mode == 2)) {
		printf("[my_read] file descriptor incorrect mode, cannot continue\n");
		return -1;
	}

	avil = running->fd[fd]->mptr->INODE->i_size - running->fd[fd]->offset;	//compute amout of bytes left in file

	//read until it has read all of the bytes that the user requested or until the entire file has been read
	while (0 < nbytes || 0 < avil) {
		lbk = running->fd[fd]->offset / BLKSIZE;		//calculates the logical block
		startByte = (running->fd[fd]->offset % BLKSIZE);	//calculates the start bit

		//if ((blk = lb_2_pb(running->fd[fd]->mptr->INODE, lbk)) == -1) { return; }	//get physical block number
		if ((blk = lb_2_pb(running->fd[fd]->mptr, lbk)) == -1) { return; }
		get_block(running->fd[fd]->mptr->dev, blk, tmpbuf);		//get the block data of file

		//determine the last byte to be copied from the current block:
		if (startByte + nbytes <= BLKSIZE) {
			endIndex = startByte + nbytes;
		}
		else { endIndex = BLKSIZE; }

		if (copied == 0) {	//its the first time copying, so use strcpy
			strncpy(buf, tmpbuf + startByte, endIndex - startByte);
		}
		else {	//it isnt the first time copying, so use strcat
			strncat(buf, tmpbuf + startByte, endIndex - startByte);
		}

		copied = endIndex - startByte;	//calculate how many bytes were just copied
		running->fd[fd]->offset += copied;
		count += copied;
		avil -= copied;
		nbytes -= copied;
	}

	return count;
}
