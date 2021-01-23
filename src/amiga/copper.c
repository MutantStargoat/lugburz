#include "copper.h"
#include "hwregs.h"

uint32_t *copperlist, *copperlist_end;
static uint32_t *copmem, *curlist;
static int mode, copmem_size;

extern uint32_t **_mem_start;

int init_copper(uint32_t *cmem, int maxlist, int nlists)
{
	/* allocate and set new copper lists */
	if(maxlist <= 0) maxlist = 256;
	mode = nlists >= COPPER_DOUBLE ? COPPER_DOUBLE : COPPER_SINGLE;

	copmem_size = maxlist * 4 * mode;
	copmem = cmem ? cmem : *_mem_start;

	curlist = copperlist = copmem;
	*curlist = COPPER_END;

	if(mode == COPPER_DOUBLE) {
		copperlist = curlist + maxlist;
		*copperlist = COPPER_END;
	}
	copperlist_end = copperlist;

	REG32_COP1LC = (uint32_t)curlist;
	REG_COPJMP1 = 0;	/* causes copper to read COP1LC */
	return 0;
}

void cleanup_copper(void)
{
}

void enable_copper(void)
{
	REG_DMACON = SETBITS(DMA_COPPER);
}

void disable_copper(void)
{
	REG_DMACON = CLRBITS(DMA_COPPER);
}

void clear_copper(void)
{
	copperlist_end = copperlist;
	*copperlist_end = COPPER_END;
}

void add_copper(uint32_t cmd)
{
	*copperlist_end++ = cmd;
}

void sort_copper(void)
{
	/* TODO */
}

void swap_copper(void)
{
	if(mode == COPPER_DOUBLE) {
		uint32_t *tmpptr;
		tmpptr = curlist;
		curlist = copperlist;
		copperlist = copperlist_end = tmpptr;

		REG32_COP1LC = (uint32_t)curlist;
		REG_COPJMP1 = 0;
	} else {
		copperlist_end = curlist;
	}
	*copperlist_end = COPPER_END;
}
