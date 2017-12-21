/*************** types.h file ******************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

#define BLKSIZE     1024
#define NMINODE      100
#define NFD           16
#define NPROC          2


typedef struct minode {
	INODE *INODE;
	char buf[BLKSIZE];
	int dev, ino;
	int refCount;
	int dirty;
	int mounted;
	struct mntTable *mptr;
}MINODE;

typedef struct oft {
	int mode;
	int refCount;
	MINODE *mptr;
	int offset;
}OFT;

typedef struct proc {
	struct proc *next;
	int pid;
	int uid;
	MINODE *cwd;
	OFT *fd[NFD];
}PROC;

typedef struct mntTable {
	int dev;         // dev number: 0=FREE
	int nblock;      // s_blocks_count
	int ninodes;     // s_inodes_count
	int bmap;        // bmap block#
	int imap;        // imap block# 
	int iblock;      // inodes start block#
	MINODE *mountDirPtr;
	char deviceName[64];
	char mountedDirName[64];
}MTABLE;


// --- Global Variable Declarations --- 
MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MTABLE mtable[4];

SUPER *sp;
GD    *gp;
INODE *ip;

int dev;	 //the disk we are using
int nblocks; // from superblock
int ninodes; // from superblock
int bmap;    // bmap block 
int imap;    // imap block 
int iblock;  // inodes begin block

// --- End of Global Variables ---

//================= end of types.h ===================