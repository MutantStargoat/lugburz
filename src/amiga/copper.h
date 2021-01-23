#ifndef COPPER_H_
#define COPPER_H_

#include "inttypes.h"

#define COPPER_MOVE(reg, data)	(((uint32_t)(reg) << 16) | ((uint32_t)(data) & 0xffff))
#define COPPER_WAIT(x, y) \
	(0x0001fffe | ((uint32_t)((x) + 0x81) << 16) | ((uint32_t)((y) + 0x2c) << 24))
#define COPPER_WAIT_OVERSCAN(x, y) \
	(0x0001fffe | ((uint32_t)(x) << 16) | ((uint32_t)(y) << 24))
#define COPPER_VWAIT(s)			(0x0001ff00 | ((uint32_t)((s) + 0x2c) << 24))
#define COPPER_VWAIT_OVERSCAN(s) \
	(0x0001ff00 | ((uint32_t)(s) << 24))
#define COPPER_OVERFLOW			0xffdffffe
#define COPPER_END				0xfffffffe

extern uint32_t *copperlist, *copperlist_end;

enum {
	COPPER_SINGLE = 1,
	COPPER_DOUBLE = 2
};

int init_copper(uint32_t *cmem, int maxlist, int nlists);
void cleanup_copper(void);

/* enables copper DMA */
void enable_copper(void);
/* disables copper DMA */
void disable_copper(void);

void clear_copper(void);
void add_copper(uint32_t cmd);
void sort_copper(void);	/* TODO */

void swap_copper(void);


#endif	/* COPPER_H_ */
