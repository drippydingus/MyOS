#include "../include/wm.h"
#include "../include/gfx.h"
#include "../include/mouse.h"
#include "../include/keyboard.h"
#include "../include/kernel.h"
#include <stdint.h>

/* ── Windows ─────────────────────────────────────────────── */

#define TITLE_H   10
#define BORDER     2
#define COL_TITLEBAR_ACTIVE   COL_DARK_BLUE
#define COL_TITLEBAR_INACTIVE COL_DARK_GREY
#define COL_WIN_BG            COL_LIGHT_GREY
#define COL_DESKTOP_TOP       32
#define COL_DESKTOP_BOT       63

static window_t wins[MAX_WINDOWS];
static int      num_wins = 0;
static int      active_win = -1;

/* drag state */
static int drag_id = -1;
static int drag_ox, drag_oy;

/* ── Icons ───────────────────────────────────────────────── */

#define MAX_ICONS 8
typedef struct {
    int x, y, win_w, win_h;
    char     label[32];
    draw_fn_t draw;
    key_fn_t  key;
} icon_t;

static icon_t icons[MAX_ICONS];
static int    num_icons = 0;

void wm_add_icon(int x, int y, const char* label,
                 draw_fn_t draw, key_fn_t key,
                 int win_w, int win_h) {
    if (num_icons >= MAX_ICONS) return;
    icon_t* ic = &icons[num_icons++];
    ic->x = x; ic->y = y;
    ic->win_w = win_w; ic->win_h = win_h;
    ic->draw = draw; ic->key = key;
    int i=0; while (label[i] && i<31) { ic->label[i]=label[i]; i++; }
    ic->label[i]='\0';
}

/* ── Helpers ─────────────────────────────────────────────── */

static int strlen_s(const char* s) { int n=0; while(s[n]) n++; return n; }

void wm_init(void) {
    for (int i=0;i<MAX_WINDOWS;i++) wins[i].active=0;
}

int wm_open(int x, int y, int w, int h,
            const char* title,
            draw_fn_t draw, key_fn_t key) {
    if (num_wins >= MAX_WINDOWS) return -1;
    int id = -1;
    for (int i=0;i<MAX_WINDOWS;i++) if (!wins[i].active) { id=i; break; }
    if (id<0) return -1;
    wins[id].x=x; wins[id].y=y;
    wins[id].w=w; wins[id].h=h;
    wins[id].active=1;
    wins[id].on_draw=draw;
    wins[id].on_key=key;
    int i=0; while(title[i]&&i<31){wins[id].title[i]=title[i];i++;}
    wins[id].title[i]='\0';
    active_win = id;
    if (id==num_wins) num_wins++;
    return id;
}

void wm_close(int id) {
    if (id<0||id>=MAX_WINDOWS) return;
    wins[id].active=0;
    active_win=-1;
    for (int i=num_wins-1;i>=0;i--) if(wins[i].active){active_win=i;break;}
}

/* ── Draw desktop gradient ───────────────────────────────── */

static void draw_desktop(void) {
    for (int y=0;y<SCREEN_H-10;y++) {
        uint8_t c = COL_DESKTOP_TOP + (y * (COL_DESKTOP_BOT-COL_DESKTOP_TOP)) / SCREEN_H;
        gfx_hline(0, SCREEN_W-1, y, c);
    }
    /* taskbar */
    gfx_fill_rect(0, SCREEN_H-10, SCREEN_W, 10, COL_DARK_GREY);
    gfx_hline(0, SCREEN_W-1, SCREEN_H-10, COL_LIGHT_GREY);
    gfx_puts(2, SCREEN_H-9, "MyOS v1.0  |  Made with love by Drippydingus", COL_WHITE, COL_DARK_GREY);
}

/* ── Draw a window ───────────────────────────────────────── */

static void draw_window(int id) {
    window_t* w = &wins[id];
    if (!w->active) return;

    uint8_t tb = (id==active_win) ? COL_TITLEBAR_ACTIVE : COL_TITLEBAR_INACTIVE;

    /* shadow */
    gfx_fill_rect(w->x+3, w->y+3, w->w, w->h+TITLE_H, COL_BLACK);

    /* titlebar */
    gfx_fill_rect(w->x, w->y, w->w, TITLE_H, tb);
    gfx_rect(w->x, w->y, w->w, TITLE_H, COL_WHITE);

    /* title text */
    int tlen = strlen_s(w->title)*8;
    int tx = w->x + (w->w - tlen)/2;
    gfx_puts(tx, w->y+1, w->title, COL_WHITE, tb);

    /* close button */
    gfx_fill_rect(w->x+w->w-10, w->y+1, 8, 8, COL_RED);
    gfx_puts(w->x+w->w-9, w->y+1, "X", COL_WHITE, COL_RED);

    /* client area */
    gfx_fill_rect(w->x, w->y+TITLE_H, w->w, w->h, COL_WIN_BG);
    gfx_rect(w->x, w->y+TITLE_H, w->w, w->h, COL_DARK_GREY);

    /* call app's draw callback */
    if (w->on_draw)
        w->on_draw(w->x, w->y+TITLE_H, w->w, w->h);
}

/* ── Draw icon on desktop ────────────────────────────────── */

static void draw_icon(int id) {
    icon_t* ic = &icons[id];
    /* 32×32 folder-like box */
    gfx_fill_rect(ic->x, ic->y, 32, 28, COL_YELLOW);
    gfx_fill_rect(ic->x, ic->y, 14,  6, COL_ORANGE);
    gfx_rect(ic->x, ic->y, 32, 28, COL_BROWN);
    /* label centered */
    int llen = strlen_s(ic->label)*8;
    int lx = ic->x + (32 - llen)/2;
    gfx_puts(lx, ic->y+30, ic->label, COL_WHITE, COL_TRANSPARENT);
}

/* ── Point-in-rect ───────────────────────────────────────── */
static int in_rect(int px,int py,int rx,int ry,int rw,int rh) {
    return px>=rx && px<rx+rw && py>=ry && py<ry+rh;
}

/* ── Simple delay ────────────────────────────────────────── */
static void delay(int n) { while(n--) __asm__ volatile("nop"); }

/* ── Main loop ───────────────────────────────────────────── */

void wm_run(void) {
    int prev_btn = 0;
    int prev_mx = mouse.x, prev_my = mouse.y;

    while (1) {
        mouse_poll();

        int clicked    = mouse.btn_left && !prev_btn;
        int released   = !mouse.btn_left && prev_btn;
        int mouse_moved= (mouse.x!=prev_mx || mouse.y!=prev_my);

        /* ---- Drag window ---- */
        if (drag_id >= 0) {
            if (mouse.btn_left) {
                wins[drag_id].x = mouse.x - drag_ox;
                wins[drag_id].y = mouse.y - drag_oy;
                /* clamp */
                if (wins[drag_id].x < 0) wins[drag_id].x=0;
                if (wins[drag_id].y < 0) wins[drag_id].y=0;
                if (wins[drag_id].x+wins[drag_id].w > SCREEN_W)
                    wins[drag_id].x = SCREEN_W - wins[drag_id].w;
                if (wins[drag_id].y+wins[drag_id].h+TITLE_H > SCREEN_H-10)
                    wins[drag_id].y = SCREEN_H-10 - wins[drag_id].h - TITLE_H;
            } else {
                drag_id = -1;
            }
        }

        /* ---- Click handling ---- */
        if (clicked) {
            /* Close buttons */
            for (int i=num_wins-1;i>=0;i--) {
                window_t* w=&wins[i];
                if (!w->active) continue;
                if (in_rect(mouse.x,mouse.y, w->x+w->w-10, w->y+1, 8,8)) {
                    wm_close(i);
                    goto done_click;
                }
            }
            /* Titlebar drag / focus */
            for (int i=num_wins-1;i>=0;i--) {
                window_t* w=&wins[i];
                if (!w->active) continue;
                if (in_rect(mouse.x,mouse.y, w->x, w->y, w->w, TITLE_H)) {
                    active_win = i;
                    drag_id = i;
                    drag_ox = mouse.x - w->x;
                    drag_oy = mouse.y - w->y;
                    goto done_click;
                }
            }
            /* Click inside window → focus */
            for (int i=num_wins-1;i>=0;i--) {
                window_t* w=&wins[i];
                if (!w->active) continue;
                if (in_rect(mouse.x,mouse.y, w->x, w->y+TITLE_H, w->w, w->h)) {
                    active_win = i;
                    goto done_click;
                }
            }
            /* Double-click icon (single click here opens it) */
            for (int i=0;i<num_icons;i++) {
                icon_t* ic=&icons[i];
                if (in_rect(mouse.x,mouse.y, ic->x, ic->y, 32,58)) {
                    wm_open(40+i*20, 20+i*10, ic->win_w, ic->win_h,
                            ic->label, ic->draw, ic->key);
                    goto done_click;
                }
            }
        }
        done_click:;

        /* ---- Keyboard → active window ---- */
        /* Poll keyboard (non-blocking: check status port) */
        if (inb(0x64) & 0x01) {
            /* make sure it's not mouse data */
            uint8_t sc = inb(0x60);
            if (!(sc & 0x80)) {        /* key press only */
                /* minimal scancode→ascii for wm-level keys */
                /* (games handle their own polling) */
                (void)sc;
            }
        }

        /* ---- Render ---- */
        if (mouse_moved || clicked || released || drag_id>=0) {
            draw_desktop();
            for (int i=0;i<num_icons;i++) draw_icon(i);
            for (int i=0;i<num_wins;i++)  draw_window(i);
            gfx_draw_cursor(mouse.x, mouse.y);
            gfx_flip();
        }

        prev_btn = mouse.btn_left;
        prev_mx = mouse.x; prev_my = mouse.y;

        delay(5000);
    }
}
