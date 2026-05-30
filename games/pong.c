#include "../include/gfx.h"
#include "../include/keyboard.h"
#include "../include/kernel.h"
#include <stdint.h>

/* ── Pong ─────────────────────────────────────────────────
   Runs inside a WM window client area.
   Called each frame from a game-loop window's on_draw.
   Key polling done via inb(0x64)/inb(0x60).
   ────────────────────────────────────────────────────────*/

#define PW  4    /* paddle width  */
#define PH  24   /* paddle height */
#define BW  4    /* ball size      */

static int  px_init = 0;
static int  py_init = 0;
static int  pw_init = 0;
static int  ph_init = 0;

static int lpy, rpy;           /* left/right paddle Y */
static int bx, by, bdx, bdy;  /* ball pos + velocity  */
static int lscore, rscore;
static int pong_running;
static int frame;

static void pong_reset_ball(void) {
    bx = px_init + pw_init/2 - BW/2;
    by = py_init + ph_init/2 - BW/2;
    bdx = (frame&1) ? 2 : -2;
    bdy = (frame&2) ? 1 : -1;
}

static void pong_init(int x, int y, int w, int h) {
    px_init=x; py_init=y; pw_init=w; ph_init=h;
    lpy = y + h/2 - PH/2;
    rpy = y + h/2 - PH/2;
    lscore=0; rscore=0;
    frame=0;
    pong_running=1;
    pong_reset_ball();
}

/* Scancode polling (non-blocking) */
static uint8_t keys[128] = {0};

static void poll_keys(void) {
    while (inb(0x64) & 0x01) {
        uint8_t sc = inb(0x60);
        if (sc & 0x80) keys[sc&0x7F]=0;
        else           keys[sc&0x7F]=1;
    }
}

/* SC: W=0x11, S=0x1F, Up=0x48, Down=0x50 */
#define SC_W    0x11
#define SC_S    0x1F
#define SC_UP   0x48
#define SC_DOWN 0x50

static void pong_tick(int x, int y, int w, int h) {
    poll_keys();
    frame++;

    /* Move paddles */
    if (keys[SC_W]    && lpy > y+1)      lpy -= 2;
    if (keys[SC_S]    && lpy < y+h-PH-1) lpy += 2;
    if (keys[SC_UP]   && rpy > y+1)      rpy -= 2;
    if (keys[SC_DOWN] && rpy < y+h-PH-1) rpy += 2;

    /* Move ball */
    bx += bdx; by += bdy;

    /* Top/bottom bounce */
    if (by <= y)        { by=y;        bdy= 1; }
    if (by >= y+h-BW)   { by=y+h-BW;  bdy=-1; }

    /* Left paddle collision */
    int lpx = x+2;
    if (bx <= lpx+PW && bx >= lpx && by+BW >= lpy && by <= lpy+PH) {
        bx=lpx+PW; bdx=2;
        bdy += (by+BW/2 - (lpy+PH/2)) / 8;
        if (bdy==0) bdy=1;
    }
    /* Right paddle collision */
    int rpx = x+w-2-PW;
    if (bx+BW >= rpx && bx+BW <= rpx+PW && by+BW >= rpy && by <= rpy+PH) {
        bx=rpx-BW; bdx=-2;
        bdy += (by+BW/2 - (rpy+PH/2)) / 8;
        if (bdy==0) bdy=1;
    }

    /* Score */
    if (bx < x)     { rscore++; pong_reset_ball(); }
    if (bx > x+w)   { lscore++; pong_reset_ball(); }
    if (lscore>9||rscore>9) { pong_init(x,y,w,h); }
}

static char score_buf[8];
static void itoa2(int v, char* b) {
    if (v>=10){ b[0]='0'+v/10; b[1]='0'+v%10; b[2]='\0'; }
    else { b[0]='0'+v; b[1]='\0'; }
}

void pong_draw(int x, int y, int w, int h) {
    static int inited=0;
    if (!inited) { pong_init(x,y,w,h); inited=1; }

    pong_tick(x, y, w, h);

    /* Background */
    gfx_fill_rect(x, y, w, h, COL_BLACK);

    /* Centre line */
    for (int dy=0; dy<h; dy+=6)
        if ((dy/3)&1) gfx_fill_rect(x+w/2-1, y+dy, 2, 3, COL_DARK_GREY);

    /* Paddles */
    gfx_fill_rect(x+2,       lpy, PW, PH, COL_WHITE);
    gfx_fill_rect(x+w-2-PW,  rpy, PW, PH, COL_WHITE);

    /* Ball */
    gfx_fill_rect(bx, by, BW, BW, COL_WHITE);

    /* Score */
    itoa2(lscore, score_buf);
    gfx_puts(x+w/2-20, y+3, score_buf, COL_WHITE, COL_BLACK);
    itoa2(rscore, score_buf);
    gfx_puts(x+w/2+10, y+3, score_buf, COL_WHITE, COL_BLACK);

    /* Help */
    gfx_puts(x+2, y+h-9, "W/S  Up/Dn", COL_DARK_GREY, COL_BLACK);
}

void pong_key(char c) { (void)c; }
