#include <stdio.h>
#include "amigalib.h"

int alib_init(void)
{
	execbase = *(void**)4;

	printf("execbase: %lx\n", (unsigned int)execbase);
	printf("memlist: %lx\n", (unsigned int)&execbase->memlist);

	return 0;
}
