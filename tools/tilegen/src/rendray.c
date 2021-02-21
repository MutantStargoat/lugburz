#include <stdio.h>
#include <stdlib.h>
#include "rend.h"
#include "tileset.h"

static void cleanup(void);
static int prepare(int width, int height, struct cellgrid *g);
static void draw(struct stamp *stamp, int x, int z);

static struct renderer rend = {
	"ray",
	cleanup,
	prepare, draw
};

static int xres, yres;
static struct cellgrid *grid;
static uint32_t *framebuf;

int regrend_ray(void)
{
	rend.next = rendlist;
	rendlist = &rend;
	return 0;
}

static void cleanup(void)
{
}

static int prepare(int width, int height, struct cellgrid *g)
{
	return -1;
}

static void draw(struct stamp *stamp, int x, int z)
{
}
