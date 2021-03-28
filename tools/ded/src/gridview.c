#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include "ded.h"

static int vp[4];
static float view_aspect;
static float zoom = 1.0f;
static int mouse_x, mouse_y;
static int hover[2];
static float pan[2];

static void fit_view(void);
static void update_hover(void);


int gview_init(void)
{
	fit_view();
	return 0;
}

void gview_shutdown(void)
{
}

static void cell_pos(int cx, int cy, float *x, float *y)
{
	*x = (cx - lvl->width / 2.0f + pan[0]) * zoom;
	*y = (cy - lvl->height / 2.0f + pan[1]) * zoom;
}

static struct gridcell *cell_at(float x, float y, int *cxptr, int *cyptr)
{
	int cx, cy;
	cx = round((x / zoom - pan[0]) + lvl->width / 2.0f);
	cy = round((y / zoom - pan[1]) + lvl->height / 2.0f);

	if(cxptr) *cxptr = cx;
	if(cyptr) *cyptr = cy;
	return lvl->cells + cy * lvl->width + cx;
}

void gview_draw(void)
{
	int i, j, k;
	float x, y, hsz;
	struct gridcell *gptr = lvl->cells;
	float aspect = (float)lvl->width / (float)lvl->height;
	float pixw = 2.0f / vp[2];

	hsz = zoom * 0.5f;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f / view_aspect, 1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for(k=0; k<2; k++) {
		glBegin(GL_QUADS);

		for(i=0; i<lvl->height; i++) {
			for(j=0; j<lvl->width; j++) {
				if(k == 0) {
					if(gptr->desc & DESC_USED) {
						glColor3f(0.8, 0.8, 0.8);
					} else {
						glColor3f(0.1, 0.1, 0.1);
					}
				} else {
					if(j == hover[0] && i == hover[1]) {
						glColor3f(0.8, 0.2, 0.1);
					} else {
						glColor3f(0.3, 0.3, 0.3);
					}
				}

				cell_pos(j, i, &x, &y);
				glVertex2f(x - hsz + pixw, y - hsz + pixw);
				glVertex2f(x + hsz - pixw, y - hsz + pixw);
				glVertex2f(x + hsz - pixw, y + hsz - pixw);
				glVertex2f(x - hsz + pixw, y + hsz - pixw);
			}
		}
		glEnd();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void gview_reshape(int x, int y, int w, int h)
{
	vp[0] = x;
	vp[1] = y;
	vp[2] = w;
	vp[3] = h;
	view_aspect = (float)w / (float)h;
}

#define PAN_DELTA	0.1f

void gview_key(int key, int press)
{
	if(!press) return;

	switch(key) {
	case '0':
		fit_view();
		redisplay();
		break;

	case KEY_LEFT:
		pan[0] += PAN_DELTA / zoom;
		update_hover();
		redisplay();
		break;

	case KEY_RIGHT:
		pan[0] -= PAN_DELTA / zoom;
		update_hover();
		redisplay();
		break;

	case KEY_UP:
		pan[1] += PAN_DELTA / zoom;
		update_hover();
		redisplay();
		break;

	case KEY_DOWN:
		pan[1] -= PAN_DELTA / zoom;
		update_hover();
		redisplay();
		break;
	}
}

static int bnstate[8];

void gview_mouse(int bn, int press, int x, int y)
{
	mouse_x = x;
	mouse_y = y;

	if(bn < 8) bnstate[bn] = press;

	switch(bn) {
	case 3:
		if(press) {
			zoom *= 1.1;
			update_hover();
			redisplay();
		}
		break;

	case 4:
		if(press) {
			zoom *= 0.9;
			update_hover();
			redisplay();
		}
		break;
	}
}

void gview_motion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;

	if(bnstate[1]) {
		float pixw = 4.0f / vp[2];
		pan[0] += pixw * dx / zoom;
		pan[1] -= pixw * dy / zoom;
		redisplay();
	}

	update_hover();
}

static void fit_view(void)
{
	zoom = 2.0f / lvl->height;
	pan[0] = 0.5;
	pan[1] = 0.5;
	update_hover();
}

static void update_hover(void)
{
	float x, y;
	int prev[2];
	prev[0] = hover[0];
	prev[1] = hover[1];

	x = (2.0f * (float)mouse_x / vp[2] - 1.0f) * view_aspect;
	y = 1.0f - 2.0f * (float)mouse_y / vp[3];

	cell_at(x, y, hover, hover + 1);

	if(hover[0] != prev[0] || hover[1] != prev[1]) {
		redisplay();
	}
}
