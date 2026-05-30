#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

/* String utilities */
size_t strlen(const char* str);
void   itoa(int value, char* buf, int base);
void   utoa(uint32_t value, char* buf, int base);

/* I/O ports */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Multiboot magic */
#define MULTIBOOT_MAGIC 0x2BADB002

#endif
