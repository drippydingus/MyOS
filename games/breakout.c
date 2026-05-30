#include "../include/gfx.h"
#include "../include/kernel.h"
#include <stdint.h>

#define BR_COLS   10
#define BR_ROWS    5
#define BRK_W     16
#define BRK_H      5
#define BRK_PAD    1

#define PAD_W     28
#define PAD_H      4
#define BALL_R     3

static int bricks[BR_ROWS][BR_COLS];
static int pad_x, ball_x, ball_y, bdx, bdy;
static int lives, br_score, br_dead, br_won;
static int bfx, bfy;    /* field origin */
static int bfw, bfh;

static uint8_t brkeys[128];

static void poll_brkeys(void) {
    while (inb(0x64) & 0x01) {
        uint8_t sc = inb(0x60);
        if (sc & 0x80) brkeys[sc&0x7F]=0;
        else           brkeys[sc&0x7F]=1;
    }
}

#define SC_LEFT  0x4B
#define SC_RIGHT 0x4D
#define SC_ENTER 0x1C

static void br_init(int x, int y, int w, int h) {
    bfx=x; bfy=y; bfw=w; bfh=h;
    br_dead=0; br_won=0; lives=3; br_score=0;
    pad_x = x + w/2 - PAD_W/2;
    ball_x = x + w/2;
    ball_y = y + h - 20;
    bdx=1; bdy=-2;
    /* Init bricks */
    for (int r=0;r<BR_ROWS;r++)
        for (int c=0;c<BR_COLS;c++)
            bricks[r][c]=1;
}

static int abs_i(int v) { return v<0?-v:v; }

static char bsbuf[12];
static void bitos(int v, char* b) {
    if (v==0){b[0]='0';b[1]='\0';return;}
    char tmp[12]; int i=0;
    while(v){tmp[i++]='0'+v%10;v/=10;}
    for(int j=0;j<i;j++) b[j]=tmp[i-j-1];
    b[i]='\0';
}

static const uint8_t row_colors[BR_ROWS] = {
    COL_RED, COL_ORANGE, COL_YELLOW, COL_GREEN, COL_CYAN
};

void breakout_draw(int x, int y, int w, int h) {
    static int inited=0;
    if (!inited) { br_init(x,y,w,h); inited=1; }

    poll_brkeys();

    if (brkeys[SC_ENTER] && (br_dead||br_won)) { br_init(x,y,w,h); return; }

    if (!br_dead && !br_won) {
        /* Paddle movement */
        if (brkeys[SC_LEFT]  && pad_x > x+1)        pad_x -= 3;
        if (brkeys[SC_RIGHT] && pad_x+PAD_W < x+w-1) pad_x += 3;

        /* Ball movement */
        ball_x += bdx; ball_y += bdy;

        /* Wall bounce */
        if (ball_x-BALL_R <= x)      { ball_x=x+BALL_R;      bdx= abs_i(bdx); }
        if (ball_x+BALL_R >= x+w)    { ball_x=x+w-BALL_R;    bdx=-abs_i(bdx); }
        if (ball_y-BALL_R <= y)      { ball_y=y+BALL_R;       bdy= abs_i(bdy); }

        /* Paddle bounce */
        int py = y+h-PAD_H-4;
        if (ball_y+BALL_R >= py && ball_y < py+PAD_H &&
            ball_x >= pad_x && ball_x <= pad_x+PAD_W) {
            bdy = -abs_i(bdy);
            ball_y = py-BALL_R;
            /* Angle based on hit position */
            int hit = ball_x - (pad_x + PAD_W/2);
            bdx = hit / 5;
            if (bdx==0) bdx=(ball_x<pad_x+PAD_W/2)?-1:1;
        }

        /* Bottom — lose life */
        if (ball_y+BALL_R > y+h) {
            lives--;
            ball_x=x+w/2; ball_y=y+h-20;
            bdx=1; bdy=-2;
            if (lives<=0) br_dead=1;
        }

        /* Brick collision */
        int brow_top = y+8;
        int brow_h   = BR_ROWS*(BRK_H+BRK_PAD);
        int all_gone = 1;
        for (int r=0;r<BR_ROWS;r++) {
            for (int c=0;c<BR_COLS;c++) {
                if (!bricks[r][c]) continue;
                all_gone=0;
                int bx2 = x + c*(BRK_W+BRK_PAD);
                int by2 = brow_top + r*(BRK_H+BRK_PAD);
                /* Simple AABB */
                if (ball_x+BALL_R >= bx2 && ball_x-BALL_R <= bx2+BRK_W &&
                    ball_y+BALL_R >= by2 && ball_y-BALL_R <= by2+BRK_H) {
                    bricks[r][c]=0;
                    br_score += 10;
                    /* Determine bounce axis */
                    int ov_x = (ball_x<bx2+BRK_W/2) ? (ball_x+BALL_R-bx2) : (bx2+BRK_W-(ball_x-BALL_R));
                    int ov_y = (ball_y<by2+BRK_H/2) ? (ball_y+BALL_R-by2) : (by2+BRK_H-(ball_y-BALL_R));
                    if (ov_x < ov_y) bdx=-bdx; else bdy=-bdy;
                }
            }
        }
        if (all_gone) br_won=1;
        (void)brow_h;
    }

    /* ---- Draw ---- */
    gfx_fill_rect(x, y, w, h, COL_BLACK);

    /* Bricks */
    int brow_top = y+8;
    for (int r=0;r<BR_ROWS;r++)
        for (int c=0;c<BR_COLS;c++) {
            if (!bricks[r][c]) continue;
            int bx2=x+c*(BRK_W+BRK_PAD);
            int by2=brow_top+r*(BRK_H+BRK_PAD);
            gfx_fill_rect(bx2, by2, BRK_W, BRK_H, row_colors[r]);
            gfx_rect(bx2, by2, BRK_W, BRK_H, COL_BLACK);
        }

    /* Paddle */
    int py=y+h-PAD_H-4;
    gfx_fill_rect(pad_x, py, PAD_W, PAD_H, COL_LIGHT_GREY);
    gfx_rect(pad_x, py, PAD_W, PAD_H, COL_WHITE);

    /* Ball */
    gfx_fill_circle(ball_x, ball_y, BALL_R, COL_WHITE);

    /* HUD */
    bitos(br_score, bsbuf);
    gfx_puts(x+2, y+h-9, "Score:", COL_WHITE, COL_BLACK);
    gfx_puts(x+50, y+h-9, bsbuf, COL_YELLOW, COL_BLACK);
    gfx_puts(x+w-30, y+h-9, "Lives:", COL_WHITE, COL_BLACK);
    char lbuf[4]; lbuf[0]='0'+lives; lbuf[1]='\0';
    gfx_puts(x+w-4, y+h-9, lbuf, COL_RED, COL_BLACK);

    if (br_dead) {
        gfx_puts_centered(y+h/2-4, "GAME OVER", COL_RED,   COL_BLACK);
        gfx_puts_centered(y+h/2+6, "ENTER=restart", COL_WHITE, COL_BLACK);
    }
    if (br_won) {
        gfx_puts_centered(y+h/2-4, "YOU WIN!", COL_YELLOW, COL_BLACK);
        gfx_puts_centered(y+h/2+6, "ENTER=restart", COL_WHITE, COL_BLACK);
    }
}

void breakout_key(char c) { (void)c; }
