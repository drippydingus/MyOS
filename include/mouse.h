#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

typedef struct {
    int x, y;
    int dx, dy;
    int btn_left, btn_right;
} mouse_state_t;

void mouse_init(void);
void mouse_poll(void);          /* call each frame to update state */
extern mouse_state_t mouse;

#endif
