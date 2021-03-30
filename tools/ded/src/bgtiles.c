#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <imago2.h>
#include <GL/gl.h>
#include "bgtiles.h"
#include "rbtree.h"
#include "md5.h"

#define MAX_BGSET	8
static struct bgtileset bgsets[MAX_BGSET];
static int num_bgsets;

static int hashcmp(const void *ap, const void *bp);

struct bgtileset *get_bgtileset(const char *name)
{
	int i;

	for(i=0; i<num_bgsets; i++) {
		if(strcmp(bgsets[i].name, name) == 0) {
			return bgsets + i;
		}
	}

	if(num_bgsets >= MAX_BGSET) {
		fprintf(stderr, "too many tilesets\n");
		return 0;
	}

	if(load_bgtiles(bgsets + num_bgsets, name) == -1) {
		return 0;
	}
	return bgsets + num_bgsets++;
}

int load_bgtiles(struct bgtileset *bgset, const char *dirname)
{
	int i, newsz, col, row;
	uint32_t desc;
	DIR *dir;
	struct dirent *dent;
	int max_tiles;
	struct rbtree *ddup;
	struct rbnode *ddnode;
	struct md5_state md;
	char *path;
	int pathsz = 0;
	struct bgtile bgtile;

	bgset->tiles = 0;
	bgset->num_tiles = 0;
	max_tiles = 0;

	if(!(bgset->name = malloc(strlen(dirname) + 1))) {
		fprintf(stderr, "failed to allocate tileset name buffer\n");
		goto fail;
	}
	strcpy(bgset->name, dirname);

	if(!(bgset->idmap = rb_create(RB_KEY_INT))) {
		fprintf(stderr, "failed to create id -> tile map\n");
		goto fail;
	}

	if(!(ddup = rb_create(hashcmp))) {
		fprintf(stderr, "failed to create tile deduplication tree\n");
		goto fail;
	}

	if(!(dir = opendir(dirname))) {
		fprintf(stderr, "failed to open tileset dir: %s: %s\n", dirname, strerror(errno));
		goto fail;
	}

	while((dent = readdir(dir))) {
		if(sscanf(dent->d_name, "s%08x_c%dr%d", &desc, &col, &row) < 3) {
			continue;
		}

		bgtile.id = desc | (col << 28) | (row << 24);

		newsz = strlen(dirname) + strlen(dent->d_name) + 2;
		if(newsz > pathsz) {
			if(!(path = malloc(newsz))) {
				fprintf(stderr, "failed to allocate path buffer\n");
				goto fail;
			}
			pathsz = newsz;
		}
		sprintf(path, "%s/%s", dirname, dent->d_name);

		if(!(bgtile.pixels = img_load_pixels(path, &bgtile.width, &bgtile.height, IMG_FMT_RGBA32))) {
			fprintf(stderr, "failed to open bgtile: %s\n", path);
			continue;
		}

		md5_begin(&md);
		md5_msg(&md, bgtile.pixels, bgtile.width * bgtile.height * 4);
		md5_end(&md);
		memcpy(bgtile.sum, md.sum, sizeof md.sum);

		/* find if we already have an identical bg tile */
		if((ddnode = rb_find(ddup, bgtile.sum))) {
			/* we found it, associate the existing tile with this descriptor */
			rb_inserti(bgset->idmap, bgtile.id, ddnode->data);	/* data is an index */
			img_free_pixels(bgtile.pixels);

			printf("load_bgtiles: map %08x -> %d\n", bgtile.id, (int)ddnode->data);
		} else {
			/* add the bgtile and insert the index into the tree */
			if(bgset->num_tiles >= max_tiles) {
				int newmax = max_tiles ? max_tiles * 2 : 8;
				void *tmp = realloc(bgset->tiles, newmax * sizeof *bgset->tiles);
				if(!tmp) {
					fprintf(stderr, "failed to resize tileset array (%d)\n", newmax);
					goto fail;
				}
				bgset->tiles = tmp;
				max_tiles = newmax;
			}

			/* also create the corresponding OpenGL texture */
			glGenTextures(1, &bgtile.gltex);
			glBindTexture(GL_TEXTURE_2D, bgtile.gltex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bgtile.width, bgtile.height, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, bgtile.pixels);

			rb_inserti(bgset->idmap, bgtile.id, (void*)bgset->num_tiles);
			bgset->tiles[bgset->num_tiles++] = bgtile;

			printf("load_bgtiles: add %d: %08x - %s\n", bgset->num_tiles - 1,
					bgtile.id, path);
		}
	}
	closedir(dir);
	rb_free(ddup);
	return 0;

fail:
	if(dir) closedir(dir);
	if(ddup) rb_free(ddup);
	if(bgset->idmap) rb_free(bgset->idmap);
	for(i=0; i<bgset->num_tiles; i++) {
		img_free_pixels(bgset->tiles[i].pixels);
		glDeleteTextures(1, &bgset->tiles[i].gltex);
	}
	free(bgset->tiles);
	free(bgset->name);
	return -1;
}

void destroy_bgtiles(struct bgtileset *bgset)
{
	int i;

	for(i=0; i<bgset->num_tiles; i++) {
		img_free_pixels(bgset->tiles[i].pixels);
		glDeleteTextures(1, &bgset->tiles[i].gltex);
	}
	free(bgset->tiles);
	rb_free(bgset->idmap);

	memset(bgset, 0, sizeof *bgset);
}

struct bgtile *get_bgtile(struct bgtileset *bgset, unsigned int desc, int col, int row)
{
	int id;
	struct rbnode *node;

	id = desc | (col << 28) | (row < 24);

	if(!(node = rb_findi(bgset->idmap, id))) {
		return 0;
	}
	return bgset->tiles + (int)node->data;
}

static int hashcmp(const void *ap, const void *bp)
{
	return memcmp(ap, bp, 128);
}
