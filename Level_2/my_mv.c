#include "my_lseek.c"

// my_mv: moves file source to file destination
void my_mv(char *line) {

	char *lineCpy = malloc(strlen(line) * sizeof(char));
	strcpy(lineCpy, line);

	my_cp(lineCpy); // copies source to destination

	char *srcPath;
	strtok(line, " ");
	srcPath = strtok(NULL, " ");

	myUnlink(srcPath); // removes the source file

}

