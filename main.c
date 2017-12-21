/* Linux EXT2 file system
* Semester Project for CS_360 (Fall-17)
* Professor: KC Wang
* 
* Authors: Trent & William
*
*/

// Level 1 Commands: MountRoot : mkdir : rmdir : ls : cd : pwd : creat : link : unlink : symlink : stat : chmod : touch
#include "./Level_1/my_unlink.c"	/// last .c in the Level 1 include stack
// Level 2 Commands: open : close : read : write : lseek : cat : cp : mv : quit
#include "./Level_2/quit.c"			/// last .c in the Level 2 include stack
// Level 3 Commands: mount : umount : (File permission checking)
// ya we didn't do it...


/************************************************************************
 *								Main
************************************************************************/
int main(int argc, char *argv[])
{
	char line[256];			// buffer for user input
	char *lineCpy;			
	char *command;			
	char *pathname;			

	init();					// init of environment

	if (argc > 1) { mount_root(argv[1]); }		// checks input disk name
	else { mount_root("mydisk"); }				// default disk name
	//else { mount_root("diskName"); }

	ls(NULL);				// displays initial disk contents

	while (1) {				// Running Terminal Loop
	
		printf("User@%s:~",argv[1]);	//print detailed command line
		rpwd(running->cwd);
		printf("$ ");

		// -- gets userinput from terminal --
		bzero(line, 256);                // zero out line[ ]
		fgets(line, 256, stdin);         // get a line (end with \n) from stdin
		line[strlen(line) - 1] = '\0';        // kill \n at end

		lineCpy = malloc(strlen(line) * sizeof(char));	
		strcpy(lineCpy, line);		 // deep copy of the input string
		
		
		// seperating command from other values passed in
		command = strtok(line, " ");
		pathname = strtok(NULL, " ");


		/*
		if (command == NULL) { printf("no input detected!\n"); continue; }
		if((strcmp(command, "ls"))		// checks if the command is a valid command
		&& (strcmp(command, "cd"))		
		&& (strcmp(command, "pwd"))			
		&& (strcmp(command, "mkdir")) 
		&& (strcmp(command, "creat"))		// NOTE: for some reason strcmp return false if the strings match...
		&& (strcmp(command, "rmdir"))		   // so this section is checking if command does not match any command
		&& (strcmp(command, "link")) 
		&& (strcmp(command, "unlink"))
		&& (strcmp(command, "symlink")) 
		&& (strcmp(command, "touch"))
		&& (strcmp(command, "chmod"))
		&& (strcmp(command, "stat"))
		&& (strcmp(command, "cat"))
		&& (strcmp(command, "cp"))
		&& (strcmp(command, "mv"))
		&& (strcmp(command, "open"))
		&& (strcmp(command, "close"))
		&& (strcmp(command, "lseek"))
		&& (strcmp(command, "write"))
		&& (strcmp(command, "quit")))
		{ printf("unknown command!\n"); continue; }

		*/

		
		
		//	 if ( command    ==	  "name" )	{ call function(); }
		if (command == NULL) { printf("no input detected!\n"); }	// no input given
		else if (!strcmp(command, "ls"))	{ ls(pathname); }
		else if (!strcmp(command, "cd"))	{ cd(pathname); }
		else if (!strcmp(command, "pwd"))	{ pwd(running->cwd); }
		else if (!strcmp(command, "mkdir"))		{ mkNewDir(pathname);  }
		else if (!strcmp(command, "creat"))		{ my_Creat(pathname); }
		else if (!strcmp(command, "rmdir"))		{ newRmdir(pathname); }
		else if (!strcmp(command, "link"))		{ myLink(lineCpy); }
		else if (!strcmp(command, "unlink"))	{ myUnlink(pathname); }
		else if (!strcmp(command, "symlink"))	{ mySyslink(lineCpy); }
		else if (!strcmp(command, "touch"))		{ touch(pathname); }
		else if (!strcmp(command, "chmod"))		{ my_chmod(lineCpy); }
		else if (!strcmp(command, "stat"))		{ myStat(pathname); }
		else if (!strcmp(command, "cat"))		{ my_cat(pathname); }
		else if (!strcmp(command, "cp"))		{ my_cp(lineCpy); }
		else if (!strcmp(command, "mv"))		{ my_mv(lineCpy); }
		else if (!strcmp(command, "open"))	{ mySys_open(lineCpy); }
		else if (!strcmp(command, "close"))	{ mySys_close(pathname); }
		else if (!strcmp(command, "lseek"))	{ mySys_lseek(lineCpy); }
		else if (!strcmp(command, "write"))	{ mySys_write(pathname); }
		else if (!strcmp(command, "quit"))	{ quit(); return 1; }
		else { printf("unknown command!\n"); }		// bad command given 

	}
	return 1;
}