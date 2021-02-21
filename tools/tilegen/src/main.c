#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "tileset.h"
#include "rend.h"

static int init(void);
static void cleanup(void);
static void display(void);
static void reshape(int x, int y);
static void keyboard(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);
static int parse_args(int argc, char **argv);

static const char *tset_fname;

static int win_width, win_height;
static float win_aspect;

static int bnstate[8];
static int mouse_x, mouse_y;

static float cam_theta, cam_phi = 15, cam_dist = 8;

static int show_grid = 1;
static int user_view = 1;
static float vfov = 60;


int regrend_gl(void);
int regrend_ray(void);


int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

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
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glClearColor(0.2, 0.2, 0.2, 1);

	regrend_gl();
	regrend_ray();

	if(!rendlist) {
		fprintf(stderr, "no renderers\n");
		return -1;
	}

	if(load_tileset(tset_fname) == -1) {
		return -1;
	}

	return 0;
}

static void cleanup(void)
{
	destroy_tileset();

	while(rendlist) {
		rendlist->cleanup();
		rendlist = rendlist->next;
	}
	rendlist = 0;
}

static void display(void)
{
	float lpos[] = {0, 1, 0, 1};
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(vfov, win_aspect, 0.5, 500.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(user_view) {
		glTranslatef(0, 0, -cam_dist);
		glRotatef(cam_phi, 1, 0, 0);
		glRotatef(cam_theta, 0, 1, 0);
	} else {
		glTranslatef(0, -1, -1);
	}

	if(show_grid) {
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(2);
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(-1000, 0, 0);
		glVertex3f(1000, 0, 0);
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, -1000);
		glVertex3f(0, 0, 1000);
		glEnd();
		glPopAttrib();
	}

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	draw_cell(0x80001102);

	glutSwapBuffers();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;

	glViewport(0, 0, x, y);
}

static void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);

	case 'g':
		show_grid ^= 1;
		glutPostRedisplay();
		break;

	case '\t':
		user_view ^= 1;
		glutPostRedisplay();
		break;
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

	if(!(dx | dy) || !user_view) {
		return;
	}

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

static void print_usage(const char *argv0)
{
	printf("Usage: %s [options] <tileset>\n", argv0);
	printf("Options:\n");
	printf("  -h: print usage and exit\n");
}

static int parse_args(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] == 0) {
				switch(argv[i][1]) {
				case 'h':
					print_usage(argv[0]);
					exit(0);

				default:
					fprintf(stderr, "invalid option: %s\n", argv[i]);
					return -1;
				}
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return -1;
			}
		} else {
			if(tset_fname) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				return -1;
			}
			tset_fname = argv[i];
		}
	}

	if(!tset_fname) {
		fprintf(stderr, "pass a tileset descriptor filename\n");
		return -1;
	}

	return 0;
}
