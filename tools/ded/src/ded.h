#ifndef DED_H_
#define DED_H_

#include <drawtext.h>
#include "level.h"

enum {
	KEY_F1 = 256, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
	KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
	KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN,
	KEY_HOME, KEY_END, KEY_INS, KEY_PGUP, KEY_PGDOWN
};

struct level *lvl;
struct dtx_font *font;

void redisplay(void);

/* grid view */
int gview_init(void);
void gview_shutdown(void);

void gview_draw(void);
void gview_reshape(int x, int y, int w, int h);

void gview_key(int key, int press);
void gview_mouse(int bn, int press, int x, int y);
void gview_motion(int x, int y);

/* dungeon view */
int dview_init(void);
void dview_shutdown(void);

void dview_draw(void);
void dview_reshape(int x, int y, int w, int h);

void dview_key(int key, int press);
void dview_mouse(int bn, int press, int x, int y);
void dview_motion(int x, int y);

#endif	/* DED_H_ */
