#include	"my_rmdir.c"

// my_stat: displays the statistics of a dir/file
// Input: string pathname to the dir/file to stat
void my_stat(char *pathname)
{
	int ino;
	MINODE *mip;

	if (!pathname) { printf("Must include a name\n"); return; }
	if (!(ino = getino(pathname))) { printf("%s does not exist, cannot stat\n", pathname); return; }
	mip = iget(mtable[0].dev, ino);

	char tmeStr[64];
	time_t inodeTime = mip->INODE->i_atime;
	strcpy(tmeStr, ctime(&inodeTime));
	tmeStr[strlen(tmeStr) - 1] = 0;

	printf("********* stat *********\n");
	printf("dev=%d  ", mip->dev);
	printf("ino=%d  ", ino);
	printf("mod=%x\n", mip->INODE->i_mode);
	printf("uid=%d  ", mip->INODE->i_uid);
	printf("gid=%d  ", mip->INODE->i_gid);
	printf("nlink=%d\n", mip->INODE->i_links_count);
	printf("size=%d  ", mip->INODE->i_size);
	printf("time=%s\n", tmeStr);
	printf("************************\n");
}
