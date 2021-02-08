#ifndef TILESET_H_
#define TILESET_H_

enum { DIR_N, DIR_W, DIR_S, DIR_E };
enum {
	TILE_WALL,
	TILE_CORNER,
	TILE_DOOR,
	TILE_EXIT_UP,
	TILE_EXIT_DOWN,

	MAX_TILES
};

int load_tileset(const char *fname);
void destroy_tileset(void);

void draw_tile(int dir, int tile);


#endif	/* TILESET_H_ */
