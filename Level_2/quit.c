#include " my_write.c"

// ---------- QUIT
void quit()
{
	//save all minodes with refCount > 0 and is DIRTY
	for (int i = 0; i < NMINODE; i++)
	{
		while ((minode[i].refCount > 0) && minode[i].dirty) { iput(&minode[i]); }	//use while because refCount may be greater than 1
	}
}