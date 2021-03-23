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

static void *flip_image(int dir, void *pixels, int width, int height);

static unsigned char *tmpbuf;
static int tmpsz;

int save_stamp(struct renderer *rend, const char *outpath, int x, int y, uint32_t desc)
{
	char *fname;
	DIR *dir;

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

	if(img_save_pixels(fname, tmpbuf, rend->width, rend->height, IMG_FMT_RGBA32) == -1) {
		printf("failed\n");
	} else {
		printf("OK\n");
	}
	return 0;
}

void *vflip_image(void *pixels, int width, int height)
{
	return flip_image(0, pixels, width, height);
}

void *hflip_image(void *pixels, int width, int height)
{
	return flip_image(1, pixels, width, height);
}

static void *flip_image(int dir, void *pixels, int width, int height)
{
	int i, j, imgsz, sz;
	uint32_t *sptr, *dptr;

	imgsz = width * height * 4;
	sz = imgsz * 2;
	if(!tmpbuf || tmpsz < sz) {
		free(tmpbuf);
		if(!(tmpbuf = malloc(sz))) {
			fprintf(stderr, "failed to allocate temp image (%dx%d)\n", width, height);
			return 0;
		}
		tmpsz = sz;
	}

	if(dir == 0) {
		/* flip the image vertically */
		sptr = (uint32_t*)pixels + width * height;
		dptr = (uint32_t*)(pixels == tmpbuf ? tmpbuf + imgsz : tmpbuf);

		for(i=0; i<height; i++) {
			sptr -= width;
			memcpy(dptr, sptr, width * 4);
			dptr += width;
		}
	} else {
		/* flip the image horizontally */
		sptr = (uint32_t*)pixels + width;
		dptr = (uint32_t*)(pixels == tmpbuf ? tmpbuf + imgsz : tmpbuf);

		for(i=0; i<height; i++) {
			for(j=0; j<width; j++) {
				*dptr++ = *--sptr;
			}
			sptr += width * 2;
		}
	}

	return tmpbuf;
}
