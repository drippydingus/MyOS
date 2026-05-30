#include "../include/mouse.h"
#include "../include/kernel.h"
#include "../include/gfx.h"

mouse_state_t mouse = {SCREEN_W/2, SCREEN_H/2, 0, 0, 0, 0};

#define PS2_DATA   0x60
#define PS2_STATUS 0x64

static void ps2_wait_write(void) {
    int t = 100000;
    while ((inb(PS2_STATUS) & 0x02) && t--);
}
static void ps2_wait_read(void) {
    int t = 100000;
    while (!(inb(PS2_STATUS) & 0x01) && t--);
}
static void ps2_write(uint8_t port, uint8_t val) {
    ps2_wait_write();
    outb(port, val);
}
static uint8_t ps2_read(void) {
    ps2_wait_read();
    return inb(PS2_DATA);
}

void mouse_init(void) {
    /* Enable auxiliary device (mouse) */
    ps2_write(PS2_STATUS, 0xA8);

    /* Enable mouse interrupts (not really used — we poll) */
    ps2_write(PS2_STATUS, 0x20);
    uint8_t status = ps2_read() | 0x02;
    ps2_write(PS2_STATUS, 0x60);
    ps2_write(PS2_DATA, status);

    /* Send "reset" to mouse */
    ps2_write(PS2_STATUS, 0xD4);
    ps2_write(PS2_DATA, 0xFF);
    ps2_read(); /* ACK */
    ps2_read(); /* 0xAA */
    ps2_read(); /* device ID */

    /* Set default settings */
    ps2_write(PS2_STATUS, 0xD4);
    ps2_write(PS2_DATA, 0xF6);
    ps2_read();

    /* Enable data reporting */
    ps2_write(PS2_STATUS, 0xD4);
    ps2_write(PS2_DATA, 0xF4);
    ps2_read();
}

void mouse_poll(void) {
    /* Check if output buffer has data */
    if (!(inb(PS2_STATUS) & 0x01)) return;
    /* Check it's from the mouse (bit 5 of status) */
    uint8_t s = inb(PS2_STATUS);
    if (!(s & 0x20)) {
        inb(PS2_DATA); /* discard keyboard data */
        return;
    }

    /* Read 3-byte packet (non-blocking) */
    if (!(inb(PS2_STATUS) & 0x01)) return;
    uint8_t b0 = inb(PS2_DATA);

    /* Quick check: bit 3 must always be set in byte 0 */
    if (!(b0 & 0x08)) return;

    /* Wait for bytes 1 and 2 with timeout */
    int t;
    t = 50000; while (!(inb(PS2_STATUS) & 0x01) && t--);
    if (!t) return;
    uint8_t b1 = inb(PS2_DATA);

    t = 50000; while (!(inb(PS2_STATUS) & 0x01) && t--);
    if (!t) return;
    uint8_t b2 = inb(PS2_DATA);

    mouse.btn_left  = b0 & 0x01;
    mouse.btn_right = b0 & 0x02;

    /* Sign-extend movement bytes */
    mouse.dx = (int)b1 - ((b0 & 0x10) ? 256 : 0);
    mouse.dy = (int)b2 - ((b0 & 0x20) ? 256 : 0);

    mouse.x += mouse.dx;
    mouse.y -= mouse.dy;   /* VGA y is inverted */

    if (mouse.x < 0) mouse.x = 0;
    if (mouse.y < 0) mouse.y = 0;
    if (mouse.x >= SCREEN_W) mouse.x = SCREEN_W - 1;
    if (mouse.y >= SCREEN_H) mouse.y = SCREEN_H - 1;
}
