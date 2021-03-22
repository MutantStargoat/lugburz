#ifndef REND_H_
#define REND_H_

#include <stdint.h>

#define REND_CLEAR	0x40000000

struct renderer {
	const char *name;

	int width, height;
	uint32_t *framebuf;

	void (*cleanup)(void);
	int (*prepare)(int width, int height, float fov);
	void (*render)(int x, int y, uint32_t desc);
	void (*show)(void);

	struct renderer *next;
};

struct renderer *rendlist;

int reg_rend(struct renderer *rend);

int save_stamp(struct renderer *rend, const char *outpath, int x, int y, uint32_t desc);

#endif	/* REND_H_ */
