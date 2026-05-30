#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stddef.h>

#define SCREEN_W   320
#define SCREEN_H   200
#define VRAM       ((volatile uint8_t*)0xA0000)

/* Mode 13h 256-colour palette indices we set up ourselves */
#define COL_BLACK        0
#define COL_WHITE        1
#define COL_RED          2
#define COL_GREEN        3
#define COL_BLUE         4
#define COL_CYAN         5
#define COL_MAGENTA      6
#define COL_YELLOW       7
#define COL_DARK_GREY    8
#define COL_LIGHT_GREY   9
#define COL_ORANGE      10
#define COL_PINK        11
#define COL_DARK_BLUE   12
#define COL_DARK_GREEN  13
#define COL_BROWN       14
#define COL_TRANSPARENT 255   /* convention: never written */

/* ---- back-buffer ---- */
extern uint8_t back_buf[SCREEN_H][SCREEN_W];

void gfx_init(void);          /* enter mode 13h, set palette, clear */
void gfx_flip(void);          /* blit back_buf → VRAM               */
void gfx_clear(uint8_t col);

void gfx_putpixel(int x, int y, uint8_t col);
void gfx_hline(int x0, int x1, int y, uint8_t col);
void gfx_vline(int x, int y0, int y1, uint8_t col);
void gfx_rect(int x, int y, int w, int h, uint8_t col);
void gfx_fill_rect(int x, int y, int w, int h, uint8_t col);
void gfx_circle(int cx, int cy, int r, uint8_t col);
void gfx_fill_circle(int cx, int cy, int r, uint8_t col);

/* 8×8 bitmap font */
void gfx_putchar(int x, int y, char c, uint8_t fg, uint8_t bg);
void gfx_puts(int x, int y, const char* s, uint8_t fg, uint8_t bg);
void gfx_puts_centered(int y, const char* s, uint8_t fg, uint8_t bg);

/* cursor sprite */
void gfx_draw_cursor(int x, int y);

#endif
