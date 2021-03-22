#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

static int init(int argc, char **argv);
static void cleanup(void);
static void disp(void);
static void reshape(int x, int y);
static void keypress(unsigned char key, int x, int y);
static void keyrel(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1280, 800);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("dungeon editor");

	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keypress);
	glutKeyboardUpFunc(keyrel);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	if(init(argc, argv) == -1) {
		return 1;
	}
	atexit(cleanup);

	glutMainLoop();
	return 0;
}


static int init(int argc, char **argv)
{
	return 0;
}

static void cleanup(void)
{
}

static void disp(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
}

static void keypress(unsigned char key, int x, int y)
{
	if(key == 27) {
		exit(0);
	}
}

static void keyrel(unsigned char key, int x, int y)
{
}

static void mouse(int bn, int st, int x, int y)
{
}

static void motion(int x, int y)
{
}
