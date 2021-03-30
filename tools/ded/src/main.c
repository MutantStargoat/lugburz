#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "ded.h"
#include "level.h"
#include "bgtiles.h"

static int init(int argc, char **argv);
static void cleanup(void);
static void disp(void);
static void reshape(int x, int y);
static void keypress(unsigned char key, int x, int y);
static void keyrel(unsigned char key, int x, int y);
static void skeypress(int key, int x, int y);
static void skeyrel(int key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);
static int parse_args(int argc, char **argv);

static int win_width, win_height;
static int split;

static struct bgtileset *def_tileset;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(1280, 1024);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("dungeon editor");

	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keypress);
	glutKeyboardUpFunc(keyrel);
	glutSpecialFunc(skeypress);
	glutSpecialUpFunc(skeyrel);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	if(init(argc, argv) == -1) {
		return 1;
	}
	atexit(cleanup);

	glutMainLoop();
	return 0;
}

void redisplay(void)
{
	glutPostRedisplay();
}

static int init(int argc, char **argv)
{
	if(parse_args(argc, argv) == -1) {
		return -1;
	}

	if(!(font = dtx_open_font("font", 0))) {
		fprintf(stderr, "failed to open font\n");
		return -1;
	}
	dtx_prepare_range(font, 24, 32, 127);
	dtx_use_font(font, 24);

	if(gview_init() == -1) {
		return -1;
	}
	if(dview_init() == -1) {
		return -1;
	}

	return 0;
}

static void cleanup(void)
{
	gview_shutdown();
	dview_shutdown();
}

static void disp(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, win_width, split);
	gview_draw();
	glViewport(0, split, win_width, win_height - split);
	dview_draw();

	glutSwapBuffers();
}

static void reshape(int x, int y)
{
	split = win_height ? ((float)split / win_height) * y : y / 2;

	win_width = x;
	win_height = y;

	dview_reshape(0, 0, win_width, split);
	gview_reshape(0, split, win_width, win_height - split);
}

static void keypress(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);
	}

	if(y >= split) {
		gview_key(key, 1);
	} else {
		dview_key(key, 1);
	}
}

static void keyrel(unsigned char key, int x, int y)
{
	if(y >= split) {
		gview_key(key, 0);
	} else {
		dview_key(key, 0);
	}
}

static int translate_skey(int key)
{
	switch(key) {
	case GLUT_KEY_LEFT: return KEY_LEFT;
	case GLUT_KEY_RIGHT: return KEY_RIGHT;
	case GLUT_KEY_UP: return KEY_UP;
	case GLUT_KEY_DOWN: return KEY_DOWN;
	case GLUT_KEY_HOME: return KEY_HOME;
	case GLUT_KEY_END: return KEY_END;
	case GLUT_KEY_INSERT: return KEY_INS;
	case GLUT_KEY_PAGE_UP: return KEY_PGUP;
	case GLUT_KEY_PAGE_DOWN: return KEY_PGDOWN;
	default:
		if(key >= GLUT_KEY_F1 && key <= GLUT_KEY_F12) {
			return (key - GLUT_KEY_F1) + KEY_F1;
		}
	}
	return 0;
}

static void skeypress(int key, int x, int y)
{
	key = translate_skey(key);
	if(y >= split) {
		gview_key(key, 1);
	} else {
		dview_key(key, 1);
	}
}

static void skeyrel(int key, int x, int y)
{
	key = translate_skey(key);
	if(y >= split) {
		gview_key(key, 0);
	} else {
		dview_key(key, 0);
	}
}

static int press_x = -1, press_y;

static void mouse(int bn, int st, int x, int y)
{
	int bidx = bn - GLUT_LEFT_BUTTON;
	int press = st == GLUT_DOWN ? 1 : 0;

	if(st == GLUT_DOWN) {
		press_x = x;
		press_y = y;
	} else {
		press_x = -1;
	}

	if(press_y >= split) {
		y -= split;
		if(y < 0) y = 0;
		gview_mouse(bidx, press, x, y);
	} else {
		if(y >= split) y = split - 1;
		dview_mouse(bidx, press, x, y);
	}
}

static void motion(int x, int y)
{
	int sely;

	if(press_x >= 0) {
		sely = press_y;
	} else {
		sely = y;
	}

	if(sely >= split) {
		y -= split;
		if(y < 0) y = 0;
		gview_motion(x, y);
	} else {
		if(y >= split) y = split - 1;
		dview_motion(x, y);
	}
}

static int parse_args(int argc, char **argv)
{
	int i, ncols = 32, nrows = 32;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] == 0) {
				switch(argv[i][1]) {
				case 's':
					if(sscanf(argv[++i], "%dx%d", &ncols, &nrows) < 2) {
						fprintf(stderr, "-n must be followed by COLxROW\n");
						return -1;
					}
					break;

				case 't':
					if(!(def_tileset = get_bgtileset(argv[++i]))) {
						return -1;
					}
					break;

				default:
					fprintf(stderr, "invalid option: %s\n", argv[i]);
					return -1;
				}
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return -1;
			}

		} else {
			if(lvl) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				return -1;
			}
			if(!(lvl = malloc(sizeof *lvl)) || load_level(lvl, argv[i]) == -1) {
				free(lvl);
				fprintf(stderr, "failed to load level: %s\n", argv[i]);
				return -1;
			}
		}
	}

	if(!lvl) {
		if(!(lvl = malloc(sizeof *lvl)) || init_level(lvl, ncols, nrows) == -1) {
			free(lvl);
			fprintf(stderr, "failed to create level (%dx%d)\n", ncols, nrows);
			return -1;
		}
		lvl->bgset = def_tileset;
	}
	return 0;
}
