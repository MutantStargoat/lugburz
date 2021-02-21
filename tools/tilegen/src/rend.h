#ifndef REND_H_
#define REND_H_

#include <stdint.h>

struct stamp {
	int x, y, w, h;
	uint32_t *pixels;
};

struct cellgrid {
	int ncols, nrows;
	uint32_t *desc;
};

struct renderer {
	const char *name;

	void (*cleanup)(void);

	int (*prepare)(int width, int height, struct cellgrid *grid);
	void (*draw_stamp)(struct stamp *stamp, int x, int z);

	struct renderer *next;
};

struct renderer *rendlist;

int reg_rend(struct renderer *rend);

#endif	/* REND_H_ */
