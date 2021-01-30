#include <string.h>
#include "amigalib.h"
#include "hwregs.h"
#include "copper.h"
#include "gfx.h"
#include "game.h"
#include "serial.h"
#include "mem.h"

static uint32_t coplist[128];

int main(void)
{
	int i;

	REG_INTENA = SETBITS(INTEN_VERTB | INTEN_MASTER);
	REG_DMACON = CLRBITS(DMA_ALL);

	ser_init(38400);
	ser_print("lugburz amiga starting up...\n");

	alib_init();

	init_mem();

	init_gfx();

	REG_COLOR0 = 0x111;
	REG_COLOR1 = 0x23c;
	REG_COLOR2 = 0xc32;
	REG_COLOR3 = 0x22c;
	REG_COLOR4 = 0xcc2;

	wait_vblank();

	init_copper(coplist, 32, COPPER_SINGLE);
	for(i=0; i<NBPL; i++) {
		uint32_t addr = (intptr_t)bplptr[i];
		int reg = REGN_BPL1PTH + i * 4;
		add_copper(COPPER_MOVE(reg, addr >> 16));
		add_copper(COPPER_MOVE(reg + 2, addr));
	}
	add_copper(COPPER_VWAIT(50));
	add_copper(COPPER_MOVE(REGN_COLOR0, 0xf00));
	add_copper(COPPER_VWAIT(60));
	add_copper(COPPER_MOVE(REGN_COLOR0, 0x111));
	*copperlist_end = COPPER_END;

	game_init();

	wait_vblank();
	REG_DMACON = SETBITS(DMA_BPL | DMA_COPPER | DMA_MASTER);

	for(;;) {
		wait_vblank();
		game_draw();
	}
	return 0;
}
