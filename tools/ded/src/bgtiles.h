#ifndef BGTILES_H_
#define BGTILES_H_

#include "rbtree.h"

struct bgtile {
	int id, col, row;
	int width, height;
	void *pixels;
	unsigned int gltex;	/* for the dungeon visualization */
	uint32_t sum[4];	/* md5 sum of the pixels */
};

struct bgtileset {
	struct bgtile *tiles;
	int num_tiles;

	struct rbtree *idmap;	/* maps tile ids to indices in the tiles array */
};

int load_bgtiles(struct bgtileset *tset, const char *dirname);
void destroy_bgtiles(struct bgtileset *tset);

struct bgtile *get_bgtile(struct bgtileset *tset, unsigned int id);

#endif	/* BGTILES_H_ */
