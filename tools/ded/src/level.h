#ifndef LEVEL_H_
#define LEVEL_H_

#include <stdint.h>

enum { DIR_N, DIR_W, DIR_S, DIR_E };

#define DESC_N_MASK		0x0000000f
#define DESC_W_MASK		0x000000f0
#define DESC_S_MASK		0x00000f00
#define DESC_E_MASK		0x0000f000
#define DESC_FLOOR_MASK	0x000f0000
#define DESC_CEIL_MASK	0x00f00000
#define DESC_TILE_MASK	0x00ffffff
#define DESC_USED		0x80000000

#define DESC_N_SHIFT		0
#define DESC_W_SHIFT		4
#define DESC_S_SHIFT		8
#define DESC_E_SHIFT		12
#define DESC_FLOOR_SHIFT	16
#define DESC_CEIL_SHIFT		20

#define DESC_EXIT_N		(0x1000000 << DIR_N)
#define DESC_EXIT_W		(0x1000000 << DIR_W)
#define DESC_EXIT_S		(0x1000000 << DIR_S)
#define DESC_EXIT_E		(0x1000000 << DIR_E)

#define LINK_LEVEL_MASK	0xff
#define LINK_X_SHIFT	8
#define LINK_X_MASK		0xff00
#define LINK_Y_SHIFT	16
#define LINK_Y_MASK		0xff0000
#define LINK_ACTIVE		0x80000000

struct gridcell {
	uint32_t desc;
	uint32_t link;
};

struct level {
	int width, height;
	struct gridcell *cells;

	int id;
	int start[2];
};


int init_level(struct level *lvl, int w, int h);
int load_level(struct level *lvl, const char *fname);
int save_level(struct level *lvl, const char *fname);

#endif	/* LEVEL_H_ */
