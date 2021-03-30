#ifndef BGTILES_H_
#define BGTILES_H_

#include <stdint.h>
#include "rbtree.h"

struct bgtile {
	int id;
	int width, height;
	void *pixels;
	unsigned int gltex;	/* for the dungeon visualization */
	uint32_t sum[4];	/* md5 sum of the pixels */
};

struct bgtileset {
	char *name;
	struct bgtile *tiles;
	int num_tiles;

	struct rbtree *idmap;	/* maps tile ids to indices in the tiles array */
};

int load_bgtiles(struct bgtileset *bgset, const char *dirname);
void destroy_bgtiles(struct bgtileset *bgset);

struct bgtileset *get_bgtileset(const char *name);

struct bgtile *get_bgtile(struct bgtileset *bgset, unsigned int desc, int col, int row);

#endif	/* BGTILES_H_ */
