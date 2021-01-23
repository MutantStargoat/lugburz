#ifndef GFX_H_
#define GFX_H_

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

unsigned char framebuf[BPLSZ * NBPL];
unsigned char *bplptr[NBPL];

int init_gfx(void);

#endif	/* GFX_H_ */
