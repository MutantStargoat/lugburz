#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "treestore.h"
#include "tileset.h"
#include "cmesh.h"

#define MAX_TILES	16

struct tile {
	struct cmesh *mesh;
};

static struct tile tfloors[MAX_TILES];
static struct tile tceil[MAX_TILES];
static struct tile tsides[MAX_TILES];
static int nfloors, nceil, nsides;


int load_tileset(const char *fname)
{
	int len;
	struct ts_node *root;
	struct ts_attr *attr;
	const char *str;
	struct cmesh *mesh;
	char *dirname, *ptr;
	char path[512];
	struct tile *tile;

	if(!(root = ts_load(fname))) {
		fprintf(stderr, "failed to load tileset: %s\n", fname);
		return -1;
	}
	if(strcmp(root->name, "tileset") != 0) {
		fprintf(stderr, "invalid tileset file: %s\n", fname);
		ts_free_tree(root);
		return -1;
	}

	if((str = ts_get_attr_str(root, "name", 0))) {
		printf("Loaded tileset descriptor: %s\n", str);
	}

	if((ptr = strrchr(fname, '/'))) {
		len = ptr - fname;
		dirname = alloca(len + 1);
		memcpy(dirname, fname, len);
		dirname[len] = 0;
	} else {
		dirname = 0;
	}

	nfloors = nceil = nsides = 0;
	attr = root->attr_list;
	while(attr) {
		if(strcmp(attr->name, "floor") == 0) {
			tile = tfloors + nfloors++;
		} else if(strcmp(attr->name, "ceiling") == 0) {
			tile = tceil + nceil++;
		} else if(strcmp(attr->name, "side") == 0) {
			tile = tsides + nsides++;
		} else {
			attr = attr->next;
			continue;
		}

		if(strcmp(attr->val.str, "NULL") == 0) {
			tile->mesh = 0;
			attr = attr->next;
			continue;
		}

		if(dirname) {
			sprintf(path, "%s/%s", dirname, attr->val.str);
		} else {
			strcpy(path, attr->val.str);
		}
		printf("loading mesh: %s\n", path);

		if(!(mesh = cmesh_alloc())) {
			fprintf(stderr, "failed to allocate mesh\n");
			return -1;
		}
		if(cmesh_load(mesh, path) == -1) {
			fprintf(stderr, "failed to load mesh: %s\n", path);
			cmesh_free(mesh);
			return -1;
		}
		tile->mesh = mesh;

		attr = attr->next;
	}

	ts_free_tree(root);
	return 0;
}

void destroy_tileset(void)
{
	int i;

	for(i=0; i<MAX_TILES; i++) {
		if(tfloors[i].mesh) {
			cmesh_free(tfloors[i].mesh);
		}
		if(tceil[i].mesh) {
			cmesh_free(tceil[i].mesh);
		}
		if(tsides[i].mesh) {
			cmesh_free(tsides[i].mesh);
		}
	}

	memset(tfloors, 0, sizeof tfloors);
	memset(tceil, 0, sizeof tceil);
	memset(tsides, 0, sizeof tsides);

	nfloors = nceil = nsides = 0;
}

int tileset_num_floors(void)
{
	return nfloors;
}

int tileset_num_ceilings(void)
{
	return nceil;
}

int tileset_num_sides(void)
{
	return nsides;
}

void draw_floor(int n)
{
	n &= 0xf;
	if(!tfloors[n].mesh) return;

	cmesh_draw(tfloors[n].mesh);
}

void draw_ceil(int n)
{
	n &= 0xf;
	if(!tceil[n].mesh) return;

	cmesh_draw(tceil[n].mesh);
}

void draw_side(int dir, int n)
{
	n &= 0xf;
	if(!tsides[n].mesh) return;

	glPushMatrix();
	glRotatef(90 * dir, 0, 1, 0);

	cmesh_draw(tsides[n].mesh);

	glPopMatrix();
}

void draw_cell(uint32_t desc)
{
	int i;

	if(!(desc & 0x80000000)) {
		return;
	}

	draw_floor((desc >> DESC_FLOOR_SHIFT) & 0xf);
	draw_ceil((desc >> DESC_CEIL_SHIFT) & 0xf);

	for(i=0; i<4; i++) {
		if(desc & 0xf) {
			draw_side(i, desc & 0xf);
		}
		desc >>= 4;
	}
}
