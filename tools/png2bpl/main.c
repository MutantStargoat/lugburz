#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "image.h"

int conv_bpl(struct image *img);

void print_usage(const char *argv0);

int main(int argc, char **argv)
{
	int i, mode = 0;
	int text = 0;
	char *fname = 0, *outfname = 0;
	struct image img;
	FILE *out = stdout;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] == 0) {
				switch(argv[i][1]) {
				case 'p':
					mode = 0;
					break;

				case 'c':
					mode = 1;
					break;

				case 'i':
					mode = 2;
					break;

				case 't':
					text = 1;
					break;

				case 'o':
					if(!argv[++i]) {
						fprintf(stderr, "%s must be followed by a filename\n", argv[i - 1]);
						return 1;
					}
					outfname = argv[i];
					break;

				case 'h':
					print_usage(argv[0]);
					return 0;

				default:
					fprintf(stderr, "invalid option: %s\n", argv[i]);
					print_usage(argv[0]);
					return 1;
				}
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				print_usage(argv[0]);
				return 1;
			}
		} else {
			if(fname) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				print_usage(argv[0]);
				return 1;
			}
			fname = argv[i];
		}
	}

	if(!fname) {
		fprintf(stderr, "pass the filename of a PNG file\n");
		return 1;
	}
	if(load_image(&img, fname) == -1) {
		fprintf(stderr, "failed to load PNG file: %s\n", fname);
		return 1;
	}
	if(conv_bpl(&img) == -1) {
		fprintf(stderr, "planar conversion failed\n");
		return 1;
	}

	if(outfname) {
		if(!(out = fopen(outfname, "wb"))) {
			fprintf(stderr, "failed to open output file: %s: %s\n", outfname, strerror(errno));
			return 1;
		}
	}

	switch(mode) {
	case 0:
		fwrite(img.pixels, 1, img.scansz * img.height, out);
		break;

	case 1:
		if(text) {
			for(i=0; i<img.cmap_ncolors; i++) {
				printf("%x%x%x\n", img.cmap[i].r >> 4, img.cmap[i].g >> 4, img.cmap[i].b >> 4);
			}
		} else {
			for(i=0; i<img.cmap_ncolors; i++) {
				fputc(img.cmap[i].r >> 4, out);
				fputc((img.cmap[i].g & 0xf0) | (img.cmap[i].b >> 4), out);
			}
		}
		break;

	case 2:
		printf("size: %dx%d\n", img.width, img.height);
		printf("bit depth: %d\n", img.bpp);
		printf("scanline size: %d bytes\n", img.scansz);
		if(img.cmap_ncolors > 0) {
			printf("colormap entries: %d\n", img.cmap_ncolors);
		} else {
			printf("color channels: %d\n", img.nchan);
		}
		break;
	}

	fclose(out);
	return 0;
}

int conv_bpl(struct image *img)
{
	int i, j, num_bpl, tmp;
	unsigned char *convbuf, *src, *cptr, *dest;

	if(img->bpp > 8) {
		fprintf(stderr, "conv_bpl: source image not paletized\n");
		return -1;
	}
	if(img->cmap_ncolors > 32) {
		fprintf(stderr, "conv_bpl: too many colors in source image: %d\n", img->cmap_ncolors);
		return -1;
	}

	tmp = img->cmap_ncolors - 1;
	num_bpl = 0;
	while(tmp) {
		tmp >>= 1;
		num_bpl++;
	}

	img->scansz = img->width * num_bpl / 8;
	img->pitch = img->scansz;

	if(!(convbuf = malloc(img->scansz))) {
		perror("conv_bpl: failed to allocate conversion buffer");
		return -1;
	}

	tmp = 0;
	src = dest = img->pixels;
	while(src <= img->pixels + (img->height - 1) * img->width) {
		for(i=0; i<num_bpl; i++) {
			cptr = convbuf + i * img->width / 8;
			for(j=0; j<img->width; j++) {
				int pix = j & 7;
				*cptr |= ((src[j] >> i) & 1) << (7 - pix);
				if(pix == 7) {
					*++cptr = 0;
				}
			}
		}
		memcpy(dest, convbuf, img->scansz);
		dest += img->scansz;
		src += img->width;
		tmp++;
	}


	free(convbuf);
	return 0;
}

void print_usage(const char *argv0)
{
	printf("Usage: %s [options] <input file>\n", argv0);
	printf("Options:\n");
	printf(" -p: dump pixels (default)\n");
	printf(" -c: dump colormap (palette) entries\n");
	printf(" -i: print image information\n");
	printf(" -t: dump as text\n");
	printf(" -h: print usage and exit\n");
}
