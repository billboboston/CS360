#include "my_lseek.c"
// ----------------------------------------- MV
void my_mv(char *line) {

	char *lineCpy = malloc(strlen(line) * sizeof(char));
	strcpy(lineCpy, line);

	my_cp(lineCpy);

	char *srcPath;
	strtok(line, " ");
	srcPath = strtok(NULL, " ");

	myUnlink(srcPath);

}

