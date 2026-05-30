#include "../include/gfx.h"
#include "../include/kernel.h"
#include <stdint.h>

#define CELL    5
#define COLS    (200/CELL)   /* 40 */
#define ROWS    (160/CELL)   /* 32 */
#define MAXLEN  256

#define SC_UP    0x48
#define SC_DOWN  0x50
#define SC_LEFT  0x4B
#define SC_RIGHT 0x4D
#define SC_ENTER 0x1C

static int sx[MAXLEN], sy[MAXLEN];
static int slen, sdir, snext;
static int fx, fy;
static int dead;
static int score;
static int frame_skip, frames;

static uint8_t sk[128];

static void poll_keys(void) {
    while (inb(0x64) & 0x01) {
        uint8_t sc = inb(0x60);
        if (sc & 0x80) sk[sc&0x7F]=0;
        else           sk[sc&0x7F]=1;
    }
}

static int rx, ry_s;   /* simple LCG for food */
static uint32_t rseed = 12345;
static int rnd(int n) { rseed=rseed*1664525+1013904223; return (rseed>>16)%n; }

static void place_food(void) {
    fx = rnd(COLS); fy = rnd(ROWS);
}

static void snake_init(int ox, int oy) {
    (void)ox; (void)oy;
    rx=0; ry_s=0;
    slen=4;
    for (int i=0;i<slen;i++) { sx[i]=COLS/2-i; sy[i]=ROWS/2; }
    sdir=1; snext=1;  /* 0=up,1=right,2=down,3=left */
    dead=0; score=0; frames=0; frame_skip=6;
    place_food();
}

static char ibuf[12];
static void itos(int v, char* b) {
    if (v==0){b[0]='0';b[1]='\0';return;}
    char tmp[12]; int i=0;
    while(v){tmp[i++]='0'+v%10;v/=10;}
    for(int j=0;j<i;j++) b[j]=tmp[i-j-1];
    b[i]='\0';
}

void snake_draw(int x, int y, int w, int h) {
    static int inited=0;
    if (!inited) { snake_init(x,y); inited=1; }

    poll_keys();
    frames++;

    /* Input */
    if (sk[SC_UP]    && sdir!=2) snext=0;
    if (sk[SC_RIGHT] && sdir!=3) snext=1;
    if (sk[SC_DOWN]  && sdir!=0) snext=2;
    if (sk[SC_LEFT]  && sdir!=1) snext=3;
    if (dead && sk[SC_ENTER]) { snake_init(x,y); return; }

    /* Update */
    if (!dead && frames >= frame_skip) {
        frames=0;
        sdir = snext;
        int nx=sx[0], ny=sy[0];
        if (sdir==0) ny--;
        if (sdir==1) nx++;
        if (sdir==2) ny++;
        if (sdir==3) nx--;

        /* Wall */
        if (nx<0||nx>=COLS||ny<0||ny>=ROWS) { dead=1; goto draw; }
        /* Self */
        for (int i=1;i<slen-1;i++)
            if (sx[i]==nx&&sy[i]==ny) { dead=1; goto draw; }

        /* Shift */
        for (int i=slen-1;i>0;i--) { sx[i]=sx[i-1]; sy[i]=sy[i-1]; }
        sx[0]=nx; sy[0]=ny;

        /* Eat food */
        if (nx==fx && ny==fy) {
            score++;
            if (slen<MAXLEN) slen++;
            if (frame_skip>2 && score%5==0) frame_skip--;
            place_food();
        }
    }

draw:;
    /* Grid background */
    gfx_fill_rect(x, y, w, h, COL_DARK_GREEN);
    gfx_rect(x, y, w, h, COL_GREEN);

    int ox=x+2, oy=y+2;

    /* Food */
    gfx_fill_circle(ox+fx*CELL+CELL/2, oy+fy*CELL+CELL/2, CELL/2, COL_RED);

    /* Snake */
    for (int i=0;i<slen;i++) {
        uint8_t col = (i==0) ? COL_WHITE : COL_GREEN;
        gfx_fill_rect(ox+sx[i]*CELL+1, oy+sy[i]*CELL+1, CELL-1, CELL-1, col);
    }

    /* Score */
    itos(score, ibuf);
    gfx_puts(x+2, y+h-9, "Score:", COL_WHITE, COL_DARK_GREEN);
    gfx_puts(x+50, y+h-9, ibuf, COL_YELLOW, COL_DARK_GREEN);

    if (dead) {
        gfx_puts_centered(y+h/2-4, "GAME OVER", COL_RED, COL_DARK_GREEN);
        gfx_puts_centered(y+h/2+6, "ENTER=restart", COL_WHITE, COL_DARK_GREEN);
    }
}

void snake_key(char c) { (void)c; }
