#include <stdio.h>
#include "gfx.h"
#include "hwregs.h"
#include "mem.h"
#include "copper.h"

static uint32_t fbaddr, fbwidth, fbpitch;
static uint32_t bpladdr[6];
static uint32_t bplmod;
static int num_bpl;
static uint16_t *cop_bpladdr, *cop_view;

void *allocfb(int fbwidth, int fbheight, unsigned int flags)
{
	int nbpl = flags & GFX_NBPL_MASK;
	return mem_alloc(fbwidth * fbheight * nbpl >> 3, MEM_CHIP);
}

int setup_gfx(void *fb, int width, int height, unsigned int flags)
{
	int i;
	unsigned short ham_bit = (flags & GFX_HAM) ? BPLCON0_HOMOD : 0;
	unsigned short dblpf_bit = (flags & GFX_2BG) ? BPLCON0_DBLPF : 0;
	unsigned short bplcon0, diwstart, diwstop, ddfstart, ddfstop;
	uint32_t *cop = gfx_coplist;

	num_bpl = flags & GFX_NBPL_MASK;
	fbwidth = width;

	printf("setup_gfx: fb=%p, %d bpl\n", fb, num_bpl);

	bplcon0 = BPLCON0_COUNT(num_bpl) | BPLCON0_COLOR | ham_bit | dblpf_bit;
	diwstart = 0x2c81;
	diwstop = 0x2cc1;
	ddfstart = 0x38;
	ddfstop = 0xd0;

	fbaddr = (uint32_t)fb;
	bpladdr[0] = fbaddr;

	if(flags & GFX_ILV) {
		fbpitch = (width * num_bpl) >> 3;
		bplmod = (width - 320 + width * (num_bpl - 1)) >> 3;

		for(i=1; i<num_bpl; i++) {
			bpladdr[i] = bpladdr[i - 1] + (width >> 3);
		}
	} else {
		fbpitch = width >> 3;
		bplmod = (width - 320) >> 3;

		for(i=1; i<num_bpl; i++) {
			bpladdr[i] = bpladdr[i - 1] + (width * height >> 3);
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

		cop_view = 0;
	} else {
		*cop++ = COPPER_MOVE(REGN_BPLCON0, bplcon0);
		cop_view = (uint16_t*)cop;
		*cop++ = COPPER_MOVE(REGN_BPLCON1, 0);
		*cop++ = COPPER_MOVE(REGN_DIWSTART, diwstart);
		*cop++ = COPPER_MOVE(REGN_DIWSTOP, diwstop);
		*cop++ = COPPER_MOVE(REGN_DDFSTART, ddfstart);
		*cop++ = COPPER_MOVE(REGN_DDFSTOP, ddfstop);
		*cop++ = COPPER_MOVE(REGN_BPL1MOD, bplmod);
		*cop++ = COPPER_MOVE(REGN_BPL2MOD, bplmod);
	}

	cop_bpladdr = (uint16_t*)cop;
	for(i=0; i<num_bpl; i++) {
		*cop++ = COPPER_MOVE(REGN_BPL1PTH + i * 4, bpladdr[i] >> 16);
		*cop++ = COPPER_MOVE(REGN_BPL1PTL + i * 4, bpladdr[i]);
	}
	*cop = COPPER_END;

	gfx_copsize = cop - gfx_coplist;
	return 0;
}

void gfx_scroll(int x, int y)
{
	int i;
	uint32_t fboffs = 0;
	uint16_t *cop;
	uint32_t addr;
	unsigned int sub;

	if(y > 0) {
		fboffs = y * fbpitch;
	}

	if(x > 0) {
		fboffs += x >> 3;
	}

	cop = cop_bpladdr;
	for(i=0; i<num_bpl; i++) {
		addr = bpladdr[i] + fboffs;
		cop[1] = addr >> 16;
		cop[3] = addr;
		cop += 4;
	}

	if(!cop_view) return;

	sub = x & 0xf;	/* sub-scroll inside the extra 16-pixel fetched word */
#if 0
	if(!sub) {
		cop_view[1] = 0;		/* BPLCON1 */
		cop_view[7] = 0x38;		/* DDFSTART */
		cop_view[11] = cop_view[13] = bplmod;
	} else {
#endif
		sub ^= 0xf;
		cop_view[1] = sub | (sub << 4);		/* BPLCON1 */
		cop_view[7] = 0x30;					/* DDFSTART */
		cop_view[11] = cop_view[13] = bplmod - 2;
	//}
}
