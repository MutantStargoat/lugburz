#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <imago2.h>
#include "rend.h"

static unsigned char *tmpbuf;
static int tmpsz;

int save_stamp(struct renderer *rend, const char *outpath, int x, int y, uint32_t desc)
{
	int i, sz;
	char *fname;
	DIR *dir;
	uint32_t *sptr, *dptr;

	sz = rend->width * rend->height * 4;
	if(!tmpbuf || tmpsz < sz) {
		free(tmpbuf);
		if(!(tmpbuf = malloc(sz))) {
			fprintf(stderr, "save_stamp: failed to allocate temp image (%dx%d)\n", rend->width, rend->height);
			return -1;
		}
		tmpsz = sz;
	}

	if(outpath) {
		if(!(dir = opendir(outpath))) {
			if(mkdir(outpath, 0775) == -1) {
				fprintf(stderr, "save_stamp: failed to create output directory: %s: %s\n",
						outpath, strerror(errno));
				return -1;
			}
		} else {
			closedir(dir);
		}

		fname = alloca(strlen(outpath) + 64);
		sprintf(fname, "%s/s%08x_c%dr%d.png", outpath, desc, x, y);
	} else {
		fname = alloca(64);
		sprintf(fname, "s%08x_c%dr%d.png", desc, x, y);
	}

	printf("saving stamp: %s ... ", fname);
	fflush(stdout);

	/* flip the image vertically */
	sptr = (uint32_t*)rend->framebuf + rend->width * rend->height;
	dptr = (uint32_t*)tmpbuf;

	for(i=0; i<rend->height; i++) {
		sptr -= rend->width;
		memcpy(dptr, sptr, rend->width * 4);
		dptr += rend->width;
	}

	if(img_save_pixels(fname, tmpbuf, rend->width, rend->height, IMG_FMT_RGBA32) == -1) {
		printf("failed\n");
	} else {
		printf("OK\n");
	}
	return 0;
}
