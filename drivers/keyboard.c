#include "../include/keyboard.h"
#include "../include/kernel.h"

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

/* US QWERTY scancode set 1 → ASCII (0 = non-printable) */
static const char sc_ascii[] = {
/*00*/  0,   0,  '1','2','3','4','5','6','7','8','9','0','-','=', '\b','\t',
/*10*/ 'q','w','e','r','t','y','u','i','o','p','[',']', '\n', 0, 'a','s',
/*20*/ 'd','f','g','h','j','k','l',';','\'','`',  0, '\\','z','x','c','v',
/*30*/ 'b','n','m',',','.','/',  0, '*',  0, ' ',  0,   0,   0,  0,  0,  0,
/*40*/  0,  0,  0,  0,  0,  0,  0, '7','8','9','-','4','5','6','+','1',
/*50*/ '2','3','0','.'
};

static const char sc_ascii_shift[] = {
/*00*/  0,   0,  '!','@','#','$','%','^','&','*','(',')','_','+', '\b','\t',
/*10*/ 'Q','W','E','R','T','Y','U','I','O','P','{','}', '\n', 0, 'A','S',
/*20*/ 'D','F','G','H','J','K','L',':','"', '~',  0, '|', 'Z','X','C','V',
/*30*/ 'B','N','M','<','>','?',  0, '*',  0, ' ',  0,   0,   0,  0,  0,  0,
/*40*/  0,  0,  0,  0,  0,  0,  0, '7','8','9','-','4','5','6','+','1',
/*50*/ '2','3','0','.'
};

static int shift_held = 0;
static int caps_lock  = 0;

void keyboard_init(void) {
    /* Flush the keyboard buffer */
    while (inb(KBD_STATUS_PORT) & 0x01)
        inb(KBD_DATA_PORT);
}

char keyboard_getchar(void) {
    while (1) {
        /* Wait until the output buffer is full */
        while (!(inb(KBD_STATUS_PORT) & 0x01));

        uint8_t sc = inb(KBD_DATA_PORT);

        /* Key release (bit 7 set) */
        if (sc & 0x80) {
            uint8_t rel = sc & 0x7F;
            if (rel == 0x2A || rel == 0x36) shift_held = 0;
            continue;
        }

        /* Shift keys */
        if (sc == 0x2A || sc == 0x36) { shift_held = 1; continue; }

        /* Caps Lock toggle */
        if (sc == 0x3A) { caps_lock ^= 1; continue; }

        /* Ignore extended / unknown scancodes */
        if (sc >= sizeof(sc_ascii)) continue;

        char c = shift_held ? sc_ascii_shift[sc] : sc_ascii[sc];
        if (c == 0) continue;

        /* Apply caps lock to letters only */
        if (caps_lock) {
            if (c >= 'a' && c <= 'z') c -= 32;
            else if (c >= 'A' && c <= 'Z') c += 32;
        }

        return c;
    }
}
