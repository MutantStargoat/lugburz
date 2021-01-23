#include "gfx.h"
#include "hwregs.h"

int init_gfx(void)
{
	int i;

	REG_BPLCON0 = BPLCON0_COUNT(NBPL) | BPLCON0_COLOR;
	REG_BPLCON1 = 0;
	REG_DIWSTART = 0x2c81;
	REG_DIWSTOP = 0x2cc1;
	REG_DDFSTART = 0x38;
	REG_DDFSTOP = 0xd0;

	for(i=0; i<NBPL; i++) {
		bplptr[i] = framebuf + SCANSZ * i;
	}
	REG_BPL1MOD = SCANSZ * (NBPL - 1);
	REG_BPL2MOD = SCANSZ * (NBPL - 1);

	for(i=0; i<32; i++) {
		REG_COLOR_PTR[i] = 0xf0f;
	}

	return 0;
}
