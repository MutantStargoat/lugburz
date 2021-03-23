#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "rend.h"
#include "tileset.h"
#include "cmesh.h"

static void cleanup(void);
static int prepare(int width, int height, float fov);
static void render(int x, int y, uint32_t desc);
static void show(void);

static struct renderer rend = {
	"opengl",
	0, 0, 0,
	cleanup,
	prepare,
	render,
	show
};

static unsigned int fbo, tex, zbuf;
static float vfov;

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
		fbo = 0;
	}
	free(rend.framebuf);
	rend.framebuf = 0;
}

static int prepare(int width, int height, float fov)
{
	int status;

	free(rend.framebuf);
	rend.width = width;
	rend.height = height;
	vfov = fov;

	if(!(rend.framebuf = malloc(width * height * sizeof *rend.framebuf))) {
		fprintf(stderr, "failed to allocate %dx%d framebuffer\n", width, height);
		return -1;
	}
	memset(rend.framebuf, 0, width * height * sizeof *rend.framebuf);

	if(!fbo) {
		glGenFramebuffers(1, &fbo);
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGenRenderbuffers(1, &zbuf);
	}

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rend.framebuf);

	glBindRenderbuffer(GL_RENDERBUFFER, zbuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zbuf);

	if((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "FBO incomplete (status: 0x%0x)\n", status);
		return -1;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return 0;
}

static void render(int x, int y, uint32_t desc)
{
	unsigned char *tmp;
	float lpos[] = {0, 1, 0, 1};

	glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glViewport(0, 0, rend.width, rend.height);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if(desc & REND_CLEAR) {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(vfov, (float)rend.width / rend.height, 0.5, 500.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x * 2.0f, -1.0f, -1.0f - (y * 2.0f));

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	draw_cell(desc);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();

	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rend.framebuf);

	if((tmp = vflip_image(rend.framebuf, rend.width, rend.height))) {
		memcpy(rend.framebuf, tmp, rend.width * rend.height * 4);
	}
}

extern float win_aspect;

static void show(void)
{
	float aspect = (float)rend.width / rend.height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0 / win_aspect, 1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(win_aspect > aspect) {
		glScalef(aspect, 1.0, 1.0);
	} else {
		glScalef(win_aspect, win_aspect / aspect, 1.0f);
	}

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(-1, 1);
	glEnd();

	glPopAttrib();
}
