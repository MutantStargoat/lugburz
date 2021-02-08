#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include "treestore.h"
#include "tileset.h"

struct tile {
	struct cmesh *mesh;
};

struct tile tiles[MAX_TILES];


int load_tileset(const char *fname)
{
	return -1;
}

void destroy_tileset(void)
{
}

void draw_tile(int dir, int tid)
{
}
