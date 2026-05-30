#ifndef WM_H
#define WM_H

#include <stdint.h>

#define MAX_WINDOWS 8

typedef void (*draw_fn_t)(int x, int y, int w, int h);
typedef void (*key_fn_t)(char c);

typedef struct {
    int      x, y, w, h;
    char     title[32];
    uint8_t  active;
    draw_fn_t on_draw;
    key_fn_t  on_key;
} window_t;

void wm_init(void);
void wm_run(void);                        /* main event loop */

int  wm_open(int x, int y, int w, int h,
             const char* title,
             draw_fn_t draw, key_fn_t key);
void wm_close(int id);

/* Desktop icon */
void wm_add_icon(int x, int y, const char* label,
                 draw_fn_t app_draw, key_fn_t app_key,
                 int win_w, int win_h);

#endif
