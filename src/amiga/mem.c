#include <stdio.h>
#include "serial.h"
#include "mem.h"
#include "amigalib.h"
#include "debug.h"

int init_mem(void)
{
	struct alib_memnode *mem;

	printf("chip memory top: %lx\n", (unsigned long)execbase->chipmem_top);
	printf("ext memory top: %lx\n", (unsigned long)execbase->extmem_top);

	printf("Memory ranges:\n");
	mem = execbase->memlist.head;
	while(mem->n_next) {
		char *stype;
		if(mem->attrib & ALIB_MEMF_CHIP) {
			stype = "chip";
		} else if(mem->attrib & ALIB_MEMF_FAST) {
			stype = "fast";
		} else {
			stype = "unknown";
		}
		printf(" %06lx - %06lx: %s\n", (unsigned long)mem->start,
				(unsigned long)mem->end, stype);
		mem = mem->n_next;
	}

	return 0;
}
