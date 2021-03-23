#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int win_width, win_height;
float win_aspect;

static int bnstate[8];
static int mouse_x, mouse_y;

static float cam_theta, cam_phi = 15, cam_dist = 8;

static int show_grid = 1;
static int user_view = 1;
static float vfov = 60;
static int rwidth = 320;
static int rheight = 128;
static int nrows = 4;
static int ncols = 5;

static int frame;
static unsigned int *cell_desc;
static int num_cell_desc;

static struct renderer *rend;

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
	int i, x, y, num_ceil, num_floor, num_sides;
	unsigned int *dptr;

	glewInit();

	glEnable(GL_CULL_FACE);

	regrend_gl();
	//regrend_ray();

	if(!rendlist) {
		fprintf(stderr, "no renderers\n");
		return -1;
	}
	rend = rendlist;	/* TODO */
	if(rend->prepare(rwidth, rheight, vfov) == -1) {
		fprintf(stderr, "failed to initialize %s renderer\n", rend->name);
		return -1;
	}

	if(load_tileset(tset_fname) == -1) {
		return -1;
	}

	/* build mask of valid tile descriptors */
	num_ceil = tileset_num_ceilings();
	num_floor = tileset_num_floors();
	num_sides = tileset_num_sides();

	/* num_sides * 3 because we need all variants for both left/right and far walls
	 * this is an upper bound, since just below we'll emit descriptors for left
	 * and right walls at the same time only for the center tile column.
	 */
	num_cell_desc = ncols * nrows * (num_ceil + num_floor + 3 * num_sides);
	if(!(cell_desc = malloc(num_cell_desc * sizeof *cell_desc))) {
		perror("malloc failed");
		return -1;
	}
	dptr = cell_desc;

	for(y=0; y<nrows; y++) {
		uint32_t rowbits = y << 28;
		for(x=0; x<ncols; x++) {
			uint32_t colbits = x << 24;
			for(i=0; i<num_ceil; i++) {
				*dptr++ = rowbits | colbits | (i << DESC_CEIL_SHIFT);
			}
			for(i=0; i<num_floor; i++) {
				*dptr++ = rowbits | colbits | (i << DESC_FLOOR_SHIFT);
			}
			for(i=0; i<num_sides; i++) {
				/* left-side or center: emit the west wall variants */
				if(x <= ncols/2) {
					*dptr++ = rowbits | colbits | (i << DESC_W_SHIFT);
				}
				/* right-side or center: emit the east wall variants */
				if(x >= ncols/2) {
					*dptr++ = rowbits | colbits | (i << DESC_E_SHIFT);
				}
			}
			for(i=0; i<num_sides; i++) {
				*dptr++ = rowbits | colbits | (i << DESC_N_SHIFT);
			}
		}
	}
	num_cell_desc = dptr - cell_desc;	/* original num was an upper bound, see above */

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
	int row, col;
	uint32_t desc;

	glClearColor(0.12, 0.12, 0.12, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if(frame < num_cell_desc) {
		desc = cell_desc[frame++];
		row = (desc >> 28) & 0xf;
		col = (desc >> 24) & 0xf;
		desc &= DESC_TILE_MASK;

		printf("rendering %08x at (%d,%d)\n", desc, col, row);
		rend->render(col - ncols / 2, row, REND_CLEAR | desc);
		rend->show();
		save_stamp(rend, "out", col, row, desc);
		glutPostRedisplay();
	}

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
