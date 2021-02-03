#include <stdio.h>
#include "gfx.h"
#include "hwregs.h"
#include "mem.h"
#include "copper.h"

static uint32_t bpladdr[6];
static uint32_t *cop_bpladdr;

void *allocfb(int fbwidth, int fbheight, unsigned int flags)
{
	int nbpl = flags & GFX_NBPL_MASK;
	return mem_alloc(fbwidth * fbheight * nbpl >> 3, MEM_CHIP);
}

int setup_gfx(void *fb, int fbwidth, int fbheight, unsigned int flags)
{
	int i, nbpl = flags & GFX_NBPL_MASK;
	unsigned short ham_bit = (flags & GFX_HAM) ? BPLCON0_HOMOD : 0;
	unsigned short dblpf_bit = (flags & GFX_2BG) ? BPLCON0_DBLPF : 0;
	unsigned short bplcon0, diwstart, diwstop, ddfstart, ddfstop, bplmod;
	uint32_t *cop = gfx_coplist;

	printf("setup_gfx: fb=%p, %d bpl\n", fb, nbpl);

	bplcon0 = BPLCON0_COUNT(nbpl) | BPLCON0_COLOR | ham_bit | dblpf_bit;
	diwstart = 0x2c81;
	diwstop = 0x2cc1;
	ddfstart = 0x38;
	ddfstop = 0xd0;

	bpladdr[0] = (uint32_t)fb;

	if(flags & GFX_ILV) {
		bplmod = (fbwidth - 320 + fbwidth * (nbpl - 1)) >> 3;
		printf("bplmod: %u\n", bplmod);

		for(i=1; i<nbpl; i++) {
			bpladdr[i] = bpladdr[i - 1] + (fbwidth >> 3);
			printf("BPLADDR%d: %06lx\n", i, bpladdr[i]);
		}
	} else {
		bplmod = (fbwidth - 320) >> 3;

		for(i=1; i<nbpl; i++) {
			bpladdr[i] = bpladdr[i - 1] + (fbwidth * fbheight >> 3);
		}
	}


	if(flags & GFX_IMMED) {
		REG_BPLCON0 = bplcon0;
		REG_BPLCON1 = 0;

		REG_DIWSTART = diwstart;
		REG_DIWSTOP = diwstop;
		REG_DDFSTART = ddfstart;
		REG_DDFSTOP = ddfstop;

		REG_BPL1MOD = bplmod;
		REG_BPL2MOD = bplmod;
	} else {
		*cop++ = COPPER_MOVE(REGN_BPLCON0, bplcon0);
		*cop++ = COPPER_MOVE(REGN_BPLCON1, 0);
		*cop++ = COPPER_MOVE(REGN_DIWSTART, diwstart);
		*cop++ = COPPER_MOVE(REGN_DIWSTOP, diwstop);
		*cop++ = COPPER_MOVE(REGN_DDFSTART, ddfstart);
		*cop++ = COPPER_MOVE(REGN_DDFSTOP, ddfstop);
		*cop++ = COPPER_MOVE(REGN_BPL1MOD, bplmod);
		*cop++ = COPPER_MOVE(REGN_BPL2MOD, bplmod);
	}

	cop_bpladdr = cop;
	for(i=0; i<nbpl; i++) {
		*cop++ = COPPER_MOVE(REGN_BPL1PTH + i * 4, bpladdr[i] >> 16);
		*cop++ = COPPER_MOVE(REGN_BPL1PTL + i * 4, bpladdr[i]);
	}
	*cop = COPPER_END;

	gfx_copsize = cop - gfx_coplist;
	return 0;
}

void gfx_scroll(int x, int y)
{
	/* TODO */
}
