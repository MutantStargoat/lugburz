#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "rend.h"
#include "tileset.h"

static void cleanup(void);
static int prepare(int width, int height, struct cellgrid *g);
static void draw(struct stamp *stamp, int x, int z);

static struct renderer rend = {
	"gl",
	cleanup,
	prepare, draw
};

static int xres, yres;
static struct cellgrid *grid;
static unsigned int fbo, zbuf, tex;
static uint32_t *framebuf;

int regrend_gl(void)
{
	rend.next = rendlist;
	rendlist = &rend;
	return 0;
}

static void cleanup(void)
{
	if(fbo) {
		glDeleteTextures(1, &tex);
		glDeleteRenderbuffers(1, &zbuf);
		glDeleteFramebuffers(1, &fbo);
	}
	free(framebuf);
}

static int prepare(int width, int height, struct cellgrid *g)
{
	xres = width;
	yres = height;
	grid = g;

	free(framebuf);
	if(!(framebuf = malloc(width * height * sizeof *framebuf))) {
		fprintf(stderr, "failed to allocate %dx%d framebuffer\n", width, height);
		return -1;
	}

	if(!fbo) {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &tex);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
		glGenRenderbuffers(1, &zbuf);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zbuf);
	}

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, zbuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	return 0;
}

static void draw(struct stamp *stamp, int x, int z)
{
	x += grid->ncols / 2;

	if(x < 0 || x >= grid->ncols || z < 0 || z >= grid->nrows) {
		return;
	}

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, xres, yres);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/* CONT... */

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();
}
