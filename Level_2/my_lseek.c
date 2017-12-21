#include "my_cp.c"

// my_lseek: finds/gets a number of bytes from an open fd
int my_lseek(int fd, int position) {

	if ((running->fd[fd]->mode == 0) && (position > running->fd[fd]->mptr->INODE->i_size)) { return -1; }

	running->fd[fd]->offset = position;
	return 1;
}

//syscall handeler for lseek
void mySys_lseek(char *line) {

	char *lineCpy = malloc(strlen(line) * sizeof(char));
	strcpy(lineCpy, line);

	char *s_fd;
	char *s_position;

	int fd;
	int position;

	strtok(lineCpy, " ");
	s_fd = strtok(NULL, " ");
	s_position = strtok(NULL, " ");

	fd = atoi(s_fd);
	position = atoi(s_position);

	my_lseek(fd, position);
}