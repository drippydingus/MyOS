#include "../include/vga.h"
#include "../include/kernel.h"
#include "../include/keyboard.h"
#include "../include/gfx.h"
#include "../include/mouse.h"
#include "../include/wm.h"
#include "../include/games.h"
#include <stdint.h>
#include <stddef.h>

/* ── String utils ─────────────────────────────────────────── */
size_t strlen(const char* s){size_t n=0;while(s[n])n++;return n;}
int strcmp(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return(unsigned char)*a-(unsigned char)*b;}
int strncmp(const char*a,const char*b,size_t n){while(n--&&*a&&*a==*b){a++;b++;}return n==(size_t)-1?0:(unsigned char)*a-(unsigned char)*b;}
void utoa(uint32_t v,char*buf,int base){static const char d[]="0123456789ABCDEF";char tmp[32];int i=0;if(!v){buf[0]='0';buf[1]='\0';return;}while(v){tmp[i++]=d[v%base];v/=base;}for(int j=0;j<i;j++)buf[j]=tmp[i-j-1];buf[i]='\0';}
void itoa(int v,char*buf,int base){if(v<0&&base==10){*buf++='-';utoa((uint32_t)(-v),buf,base);}else utoa((uint32_t)v,buf,base);}

/* ── Multiboot ────────────────────────────────────────────── */
typedef struct { uint32_t flags,mem_lower,mem_upper; } multiboot_info_t;
#define MULTIBOOT_MAGIC 0x2BADB002

/* ── Boot splash (text mode) then switch to GUI ──────────── */

static void boot_splash(uint32_t magic, multiboot_info_t* mbi) {
    terminal_init();

    uint8_t cyan  = vga_entry_color(VGA_COLOR_LIGHT_CYAN,  VGA_COLOR_BLACK);
    uint8_t white = vga_entry_color(VGA_COLOR_WHITE,       VGA_COLOR_BLACK);
    uint8_t pink  = vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    uint8_t yell  = vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
    uint8_t green = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    uint8_t red   = vga_entry_color(VGA_COLOR_LIGHT_RED,   VGA_COLOR_BLACK);

    terminal_setcolor(cyan);
    terminal_writestring(
        "  __  __       ___  ____  \n"
        " |  \\/  |_   _/ _ \\/ ___| \n"
        " | |\\/| | | | | | |\\___ \\ \n"
        " | |  | | |_| | |_| |___) |\n"
        " |_|  |_|\\__, |\\___/|____/ \n"
        "          |___/             \n\n"
    );
    terminal_setcolor(white);
    terminal_writestring(" MyOS v1.0  --  A minimal x86 kernel\n");
    terminal_setcolor(yell);
    terminal_writestring("============================================\n");
    terminal_setcolor(pink);
    terminal_writestring("       Made with love by Drippydingus\n");
    terminal_setcolor(yell);
    terminal_writestring("============================================\n\n");

    terminal_setcolor(white);
    if (magic == MULTIBOOT_MAGIC) {
        terminal_setcolor(green);
        terminal_writestring("[OK] Multiboot bootloader\n");
    } else {
        terminal_setcolor(red);
        terminal_writestring("[!!] Unknown bootloader\n");
    }
    terminal_setcolor(white);

    if (mbi && (mbi->flags & 0x1)) {
        char buf[16];
        terminal_writestring("[MEM] ~");
        utoa(mbi->mem_upper/1024, buf, 10);
        terminal_writestring(buf);
        terminal_writestring(" MB\n");
    }

    uint32_t eax,ebx,ecx,edx;
    char vendor[13]={0};
    __asm__ volatile("cpuid":"=a"(eax),"=b"(ebx),"=c"(ecx),"=d"(edx):"a"(0));
    ((uint32_t*)vendor)[0]=ebx;((uint32_t*)vendor)[1]=edx;((uint32_t*)vendor)[2]=ecx;
    terminal_writestring("[CPU] ");
    terminal_writestring(vendor);
    terminal_putchar('\n');

    keyboard_init();
    terminal_setcolor(green);
    terminal_writestring("[OK] Keyboard\n");
    terminal_setcolor(green);
    terminal_writestring("[OK] Mouse init...\n");
    terminal_setcolor(cyan);
    terminal_writestring("\nStarting GUI in 1 second...\n");

    /* Busy-wait ~1s */
    for (volatile int i=0;i<80000000;i++);
}

/* ── kernel_main ──────────────────────────────────────────── */

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    boot_splash(magic, mbi);

    /* Switch to VGA mode 13h graphical mode */
    gfx_init();
    mouse_init();
    wm_init();

    /* Register desktop icons */
    wm_add_icon( 8, 10, "Pong",     pong_draw,     pong_key,     160, 120);
    wm_add_icon( 8, 60, "Snake",    snake_draw,     snake_key,    220, 180);
    wm_add_icon( 8,110, "Breakout", breakout_draw,  breakout_key, 180, 160);

    /* Enter the window manager main loop — never returns */
    wm_run();

    for(;;) __asm__ volatile("hlt");
}
