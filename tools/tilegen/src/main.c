#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

static int init(void);
static void cleanup(void);
static void display(void);
static void reshape(int x, int y);
static void keyboard(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static int bnstate[8];
static int mouse_x, mouse_y;

static float cam_theta, cam_phi = 15, cam_dist = 8;


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(1024, 768);
	glutCreateWindow("tilegen");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	if(init() == -1) {
		return 1;
	}
	atexit(cleanup);

	glutMainLoop();
	return 0;
}

static int init(void)
{
	glewInit();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.2, 0.2, 0.2, 1);

	return 0;
}

static void cleanup(void)
{
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(-1000, 0, 0);
	glVertex3f(1000, 0, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, -1000);
	glVertex3f(0, 0, 1000);
	glEnd();

	glutSwapBuffers();
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(50.0, (float)x / (float)y, 0.5, 500.0);
}

static void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);
	}
}

static void mouse(int bn, int st, int x, int y)
{
	bnstate[bn - GLUT_LEFT] = st == GLUT_DOWN;
	mouse_x = x;
	mouse_y = y;
}

static void motion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;

	if(!(dx | dy)) return;

	if(bnstate[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;
		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
		glutPostRedisplay();
	}

	if(bnstate[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
		glutPostRedisplay();
	}
}
