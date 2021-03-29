#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <imago2.h>
#include "bgtiles.h"
#include "rbtree.h"
#include "md5.h"

int load_bgtiles(struct bgtileset *tset, const char *dirname)
{
	int i, newsz;
	DIR *dir;
	struct dirent *dent;
	int max_tiles;
	struct rbtree *ddup;
	struct md5_state md;
	char *path;
	int pathsz = 0;
	struct bgtile bgtile;
	unsigned char *pptr;

	if(!(ddup = rb_create(hashcmp))) {
		fprintf(stderr, "failed to create tile deduplication tree\n");
		return -1;
	}

	tset->tiles = 0;
	tset->num_tiles = 0;
	max_tiles = 0;

	if(!(dir = opendir(dirname))) {
		fprintf(stderr, "failed to open tileset dir: %s: %s\n", dirname, strerror(errno));
		goto fail;
	}

	while((dent = readdir(dir))) {
		if(sscanf(dent->d_name, "s%08x_c%dr%d", &bgtile.desc, &bgtile.col, &bgtile.row) < 3) {
			continue;
		}

		newsz = strnewsz(dirname) + strnewsz(dent->d_name) + 2;
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
			/* TODO cont. */
		}
	}
	closedir(dir);

	rb_free(ddup);
	return 0;

fail:
	rb_free(ddup);
	for(i=0; i<tset->num_tiles; i++) {
		free(tiles[i].pixels);
		glDeleteTextures(1, &tiles[i].gltex);
	}
	return -1;
}
