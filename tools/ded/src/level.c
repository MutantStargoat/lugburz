#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "level.h"
#include "bgtiles.h"

#define MAGIC	"LBLVL0BE"

struct header {
	char magic[8];
	unsigned char lvl;				/* 0xff for undefined */
	unsigned char pad0;
	unsigned char width, height;
	unsigned char sx, sy;			/* start pos */
	char tileset[32];				/* tileset name */
} __attribute__ ((packed));


int init_level(struct level *lvl, int w, int h)
{
	if(!(lvl->cells = calloc(w * h, sizeof *lvl->cells))) {
		fprintf(stderr, "init_level: failed to allocate %dx%d cells\n", w, h);
		return -1;
	}
	lvl->width = w;
	lvl->height = h;
	lvl->start[0] = w / 2;
	lvl->start[1] = h / 2;
	return 0;
}

#define BSWAP32(x)	(((x) >> 24) | ((x) << 24) | (((x) & 0xff0000) >> 8) | (((x) & 0xff00) << 8))

int load_level(struct level *lvl, const char *fname)
{
	FILE *fp;
	struct header hdr;
	int i, ncells;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "load_level: failed to open: %s: %s\n", fname, strerror(errno));
		return -1;
	}
	if(fread(&hdr, sizeof hdr, 1, fp) < 1) {
		fprintf(stderr, "load_level: failed to read file header: %s\n", fname);
		fclose(fp);
		return -1;
	}
	if(memcmp(hdr.magic, MAGIC, sizeof hdr.magic) != 0) {
		fprintf(stderr, "load_level: invalid or corrupted file: %s\n", fname);
		fclose(fp);
		return -1;
	}

	lvl->bgset = get_bgtileset(hdr.tileset);

	if(init_level(lvl, hdr.width, hdr.height) == -1) {
		fclose(fp);
		return -1;
	}

	lvl->start[0] = hdr.sx;
	lvl->start[1] = hdr.sy;
	lvl->id = hdr.lvl;

	ncells = lvl->width * lvl->height;
	if(fread(lvl->cells, 8, ncells, fp) < ncells) {
		fprintf(stderr, "load_level: invalid or corrupted file, failed to read cell data: %s\n", fname);
		free(lvl->cells);
		free(lvl);
		fclose(fp);
		return -1;
	}
	fclose(fp);

#ifdef TARGET_LEND
	for(i=0; i<ncells; i++) {
		lvl->cells[i].desc = BSWAP32(lvl->cells[i].desc);
		lvl->cells[i].link = BSWAP32(lvl->cells[i].link);
	}
#endif

	return 0;
}


int save_level(struct level *lvl, const char *fname)
{
	int i, ncells;
	FILE *fp;
	struct header hdr = {0};

	if(lvl->width <= 0 || lvl->height <= 0 || lvl->width >= 256 || lvl->height >= 256) {
		fprintf(stderr, "save_level: invalid level dimensions: %dx%d\n", lvl->width, lvl->height);
		return -1;
	}
	if(lvl->id < 0 || lvl->id >= 256) {
		fprintf(stderr, "save_level: invalid level id: %d\n", lvl->id);
		return -1;
	}
	if(lvl->start[0] < 0 || lvl->start[0] >= lvl->width ||
			lvl->start[1] < 0 || lvl->start[1] >= lvl->height) {
		fprintf(stderr, "save_level: invalid start position: %d,%d\n", lvl->start[0], lvl->start[1]);
		return -1;
	}

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "save_level: failed to open file: %s: %s\n", fname, strerror(errno));
		return -1;
	}

	memcpy(hdr.magic, MAGIC, sizeof hdr.magic);
	hdr.lvl = lvl->id;
	hdr.width = lvl->width;
	hdr.height = lvl->height;
	hdr.sx = lvl->start[0];
	hdr.sy = lvl->start[1];
	if(lvl->bgset) {
		strncpy(hdr.tileset, lvl->bgset->name, sizeof hdr.tileset - 1);
	}

	if(fwrite(&hdr, sizeof hdr, 1, fp) < 1) {
		fprintf(stderr, "save_level: failed to write header: %s\n", fname);
		fclose(fp);
		return -1;
	}

	ncells = lvl->width * lvl->height;

#ifdef TARGET_LEND
	for(i=0; i<ncells; i++) {
		struct gridcell tmp = lvl->cells[i];

		tmp.desc = BSWAP32(tmp.desc);
		tmp.link = BSWAP32(tmp.link);

		if(fwrite(&tmp, sizeof tmp, 1, fp) < 1) {
			fprintf(stderr, "save_level: failed to write cell data (%d): %s\n", i, fname);
			fclose(fp);
			return -1;
		}
	}
#else
	if(fwrite(lvl->cells, sizeof *lvl->cells, ncells, fp) < ncells) {
		fprintf(stderr, "save_level: failed to write cell data: %s\n", fname);
		fclose(fp);
		return -1;
	}
#endif

	fclose(fp);
	return 0;
}
