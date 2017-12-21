#include "../helperFunctions.c"


//mounts the disk to the root file system and esteblishes / and CWDs
void Mount_Root(char *device)
{
	printf("Starting mount procedure...");
	char buf[BLKSIZE];
	printf("Done\n");

	printf("Opening the disk...");
	dev = open(device, O_RDWR);
	if (dev < 0) { printf("open %s failed\n", device); exit(1); }
	printf("Done\n");

	printf("Getting the superblock...");
	get_block(dev, 1, buf); //get superblock
	sp = (SUPER *)buf; //assign into super_block struct
	printf("Done\n");

	printf("Checking if disk is a EXT2 file system...");
	//check to see if device is a EXT2 file system
	if (sp->s_magic != 0xEF53) { printf("NOT an EXT2 file system!\n"); exit(2); }
	printf("Done\n");

	printf("Filling out the memtable with the disk information...");
	mtable[0].dev = dev;
	mtable[0].nblock = sp->s_blocks_count;
	mtable[0].ninodes = sp->s_inodes_count;
	mtable[0].mountDirPtr = root;
	strcpy(mtable[0].deviceName, device);
	strcpy(mtable[0].mountedDirName, "/");
	printf("Done\n");

	printf("Getting the group descriptor block...");
	get_block(dev, 2, buf); //get group descriptor block
	gp = (GD *)buf; //assign into group_desc struct
	printf("Done\n");

	printf("Filling out more memtable information...");
	mtable[0].bmap = gp->bg_block_bitmap;
	mtable[0].iblock = gp->bg_inode_table;
	mtable[0].imap = gp->bg_inode_bitmap;

	//memcpy(&imap, &gp->bg_inode_table, sizeof(int));

	printf("Done\n");

	printf("Assigning root to the second inode...");
	root = iget(dev, 2);	//get and set the root inode
	printf("Done\n");

	printf("Assigning both PROCs cwd to root...");
	//Let cwd of both P0 and P1 point at the root minode
	proc[0].cwd = iget(dev, 2);
	proc[1].cwd = iget(dev, 2);
	printf("Done\n");

	printf("Assigning the running PROC to proc[0]...");
	running = &proc[0];
	printf("Done\n\n");
	printf("---- success mounting %s ----\n", device);

}
