#include "../include/vga.h"
#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

static size_t terminal_row;
static size_t terminal_col;
static uint8_t terminal_color;
static volatile uint16_t* terminal_buffer;

uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

void terminal_init(void) {
    terminal_row   = 0;
    terminal_col   = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[y * VGA_WIDTH + x] =
                vga_entry(' ', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

void terminal_clear(void) {
    terminal_init();
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

static void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[(y-1) * VGA_WIDTH + x] =
                terminal_buffer[y * VGA_WIDTH + x];

    for (size_t x = 0; x < VGA_WIDTH; x++)
        terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] =
            vga_entry(' ', terminal_color);

    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_col = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
        return;
    }
    if (c == '\r') {
        terminal_col = 0;
        return;
    }
    if (c == '\t') {
        terminal_col = (terminal_col + 8) & ~7;
        if (terminal_col >= VGA_WIDTH) {
            terminal_col = 0;
            if (++terminal_row == VGA_HEIGHT)
                terminal_scroll();
        }
        return;
    }
    if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
            terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
                vga_entry(' ', terminal_color);
        }
        return;
    }

    terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
        vga_entry((unsigned char)c, terminal_color);

    if (++terminal_col == VGA_WIDTH) {
        terminal_col = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    size_t len = 0;
    while (data[len]) len++;
    terminal_write(data, len);
}
