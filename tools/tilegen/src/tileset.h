#ifndef TILESET_H_
#define TILESET_H_

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

struct tile {
	struct cmesh *mesh;
};

int load_tileset(const char *fname);
void destroy_tileset(void);

int tileset_num_floors(void);
int tileset_num_ceilings(void);
int tileset_num_sides(void);

struct tile *tileset_floor(int idx);
struct tile *tileset_ceiling(int idx);
struct tile *tileset_side(int idx);

void draw_floor(int n);
void draw_ceil(int n);
void draw_side(int dir, int n);
void draw_cell(uint32_t desc);


#endif	/* TILESET_H_ */
