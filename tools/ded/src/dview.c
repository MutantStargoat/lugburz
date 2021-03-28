#include <GL/gl.h>
#include "ded.h"

int dview_init(void)
{
	return 0;
}

void dview_shutdown(void)
{
}

void dview_draw(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3f(0.2, 0.1, 0.1);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glEnd();
}

void dview_reshape(int x, int y, int w, int h)
{
}

void dview_key(int key, int st)
{
}

void dview_mouse(int bn, int st, int x, int y)
{
}

void dview_motion(int x, int y)
{
}
