#include <stdio.h>
#include <math.h>
#include <assert.h>
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
static void click_cell(int cx, int cy);
static void del_cell(int cx, int cy);

static void cell_pos(int cx, int cy, float *x, float *y);
static struct gridcell *cell_at(float x, float y, int *cxptr, int *cyptr);
static struct gridcell *cell_at_pixel(int px, int py, int *cxptr, int *cyptr);


int gview_init(void)
{
	fit_view();
	return 0;
}

void gview_shutdown(void)
{
}

void gview_draw(void)
{
	int i, j, k;
	float x, y, hsz;
	struct gridcell *gptr;
	float aspect = (float)lvl->width / (float)lvl->height;
	float pixw = 2.0f / vp[2];

	hsz = zoom * 0.5f;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f / view_aspect, 1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for(k=0; k<2; k++) {

		gptr = lvl->cells;

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

				gptr++;
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
	int cx, cy;
	struct gridcell *cell;

	mouse_x = x;
	mouse_y = y;

	if(bn < 8) bnstate[bn] = press;

	switch(bn) {
	case 0:
		if(press) {
			if(cell_at_pixel(x, y, &cx, &cy)) {
				click_cell(cx, cy);
				redisplay();
			}
		}
		break;

	case 2:
		if(press) {
			if(cell_at_pixel(x, y, &cx, &cy)) {
				del_cell(cx, cy);
				redisplay();
			}
		}
		break;

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

	update_hover();

	if(bnstate[0]) {
		click_cell(hover[0], hover[1]);
		redisplay();
	}
	if(bnstate[2]) {
		del_cell(hover[0], hover[1]);
		redisplay();
	}

	if(bnstate[1]) {
		float pixw = 4.0f / vp[2];
		pan[0] += pixw * dx / zoom;
		pan[1] -= pixw * dy / zoom;
		redisplay();
	}
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

	cell_at_pixel(mouse_x, mouse_y, hover, hover + 1);

	if(hover[0] != prev[0] || hover[1] != prev[1]) {
		redisplay();
	}
}

static struct gridcell *get_cell(int cx, int cy)
{
	if(cx < 0 || cx >= lvl->width) return 0;
	if(cy < 0 || cy >= lvl->height) return 0;

	return lvl->cells + cy * lvl->width + cx;
}

static int cell_used(int cx, int cy)
{
	struct gridcell *cell = get_cell(cx, cy);
	return (cell && (cell->desc & DESC_USED)) ? 1 : 0;
}

static void click_cell(int cx, int cy)
{
	struct gridcell *cell, *adj;

	if(!(cell = get_cell(cx, cy))) {
		return;
	}

	if(cell->desc & DESC_USED) return;	/* nothing to do */

	cell->desc = DESC_USED | (1 << DESC_FLOOR_SHIFT) | (1 << DESC_CEIL_SHIFT);

	if((adj = get_cell(cx - 1, cy)) && (adj->desc & DESC_USED)) {
		adj->desc &= ~DESC_E_MASK;	/* drop any walls facing our way */
	} else {
		cell->desc |= 1 << DESC_W_SHIFT;	/* nothing there, add wall */
	}
	if((adj = get_cell(cx + 1, cy)) && (adj->desc & DESC_USED)) {
		adj->desc &= ~DESC_W_MASK;
	} else {
		cell->desc |= 1 << DESC_E_SHIFT;
	}
	if((adj = get_cell(cx, cy - 1)) && (adj->desc & DESC_USED)) {
		adj->desc &= ~DESC_S_MASK;
	} else {
		cell->desc |= 1 << DESC_N_SHIFT;
	}
	if((adj = get_cell(cx, cy + 1)) && (adj->desc & DESC_USED)) {
		adj->desc &= ~DESC_N_MASK;
	} else {
		cell->desc |= 1 << DESC_S_SHIFT;
	}
}

static void del_cell(int cx, int cy)
{
	struct gridcell *cell;

	if(!(cell = get_cell(cx, cy))) {
		return;
	}

	if(!(cell->desc & DESC_USED)) return;	/* nothing to do */

	cell->desc = 0;

	if((cell = get_cell(cx - 1, cy)) && (cell->desc & DESC_USED)) {
		cell->desc |= 1 << DESC_E_SHIFT;
	}
	if((cell = get_cell(cx + 1, cy)) && (cell->desc & DESC_USED)) {
		cell->desc |= 1 << DESC_W_SHIFT;
	}
	if((cell = get_cell(cx, cy - 1)) && (cell->desc & DESC_USED)) {
		cell->desc |= 1 << DESC_S_SHIFT;
	}
	if((cell = get_cell(cx, cy + 1)) && (cell->desc & DESC_USED)) {
		cell->desc |= 1 << DESC_N_SHIFT;
	}
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

static struct gridcell *cell_at_pixel(int px, int py, int *cxptr, int *cyptr)
{
	float x = (2.0f * (float)px / vp[2] - 1.0f) * view_aspect;
	float y = 1.0f - 2.0f * (float)py / vp[3];

	return cell_at(x, y, cxptr, cyptr);
}

