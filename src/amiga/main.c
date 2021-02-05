#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amigalib.h"
#include "hwregs.h"
#include "copper.h"
#include "gfx.h"
#include "game.h"
#include "serial.h"
#include "mem.h"
#include "debug.h"
#include "data.h"

int main(void)
{
	int i, xscroll = 0;

	REG_INTENA = SETBITS(INTEN_VERTB | INTEN_MASTER);
	REG_DMACON = CLRBITS(DMA_ALL);

	ser_init(38400);
	ser_print("lugburz amiga starting up...\n");

	alib_init();

	init_mem();

	game_init();

	setup_gfx(titlescreen_pixels, 624, 256, GFX_5BPL | GFX_ILV);

	for(i=0; i<32; i++) {
		REG_COLOR_PTR[i] = titlescreen_pal[i];
	}

	wait_vblank();
	REG32_COP1LC = (uint32_t)gfx_coplist;
	REG_COPJMP1 = 0;
	REG_DMACON = SETBITS(DMA_BPL | DMA_COPPER | DMA_MASTER);

	for(;;) {
		wait_vblank();

		gfx_scroll(xscroll >> 1, 0);
		game_draw();

		if(xscroll >> 1 < 624 - 320 - 1) {
			xscroll++;
		}
	}
	return 0;
}
