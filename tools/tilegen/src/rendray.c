#include <stdio.h>
#include <stdlib.h>
#include "rend.h"
#include "tileset.h"

static void cleanup(void);
static int prepare(int width, int height, float fov);
static void render(int x, int y, uint32_t desc);
static void show(void);

static struct renderer rend = {
	"ray",
	0, 0, 0,
	cleanup,
	prepare,
	render,
	show
};

int regrend_ray(void)
{
	rend.next = rendlist;
	rendlist = &rend;
	return 0;
}


static void cleanup(void)
{
}

static int prepare(int width, int height, float fov)
{
	return 0;
}

static void render(int x, int y, uint32_t desc)
{
}

static void show(void)
{
}
