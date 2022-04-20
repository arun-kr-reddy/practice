#include "pgm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int vox();

int main()
{
	PGM src_pgm;
	
	readPGM("./input.pgm", &src_pgm);
	writePGM("./output.pgm", &src_pgm);

	free(src_pgm.ptr);

	printf("done\n");
	return EXIT_SUCCESS;
}
