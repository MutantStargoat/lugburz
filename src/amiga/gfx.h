#ifndef GFX_H_
#define GFX_H_

#include <stdint.h>

#define SCR_W	320
#define SCR_H	256
#define SCANSZ	(SCR_W / 8)
#define BPLSZ	(SCANSZ * SCR_H)
#define NBPL	5

#define wait_vpos(x) \
	asm volatile ( \
		"0: move.l 0xdff004, %%d0\n\t" \
		"and.l #0x1ff00, %%d0\n\t" \
		"cmp.l %0, %%d0\n\t" \
		"bne 0b\n\t" \
		:: "i"((x) << 8) : "%d0")

#define wait_vblank() wait_vpos(300)

#define wait_blit() \
	asm volatile ( \
		"tst 0xdff002\n\t" \
		"0: btst #6, 0xdff002\n\t" \
		"bne 0b\n\t")

enum {
	GFX_1BPL	= 0x001,
	GFX_2BPL	= 0x002,
	GFX_3BPL	= 0x003,
	GFX_4BPL	= 0x004,
	GFX_5BPL	= 0x005,
	GFX_6BPL	= 0x006,
	GFX_HAM		= 0x080,	/* otherwise 6bpl means EHB */
	GFX_2BG		= 0x100,	/* double playfield mode */
	GFX_ILV		= 0x800,
	GFX_IMMED	= 0x1000,	/* perform setup immediately, only emit ptr copper ops */
};
#define GFX_NBPL_MASK	0x7

#define GFX_MAX_COPLIST		32
uint32_t gfx_coplist[GFX_MAX_COPLIST];
int gfx_copsize;

void *allocfb(int fbwidth, int fbheight, unsigned int flags);
int setup_gfx(void *fb, int fbwidth, int fbheight, unsigned int flags);
void gfx_scroll(int x, int y);

#endif	/* GFX_H_ */
