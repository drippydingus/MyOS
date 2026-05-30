CC      = gcc
AS      = nasm
LD      = ld

CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra -nostdlib \
          -fno-builtin -fno-stack-protector -fno-pic \
          -I./include -lm

ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld --oformat elf32-i386

OBJDIR  = build
ISO_DIR = isodir

OBJS = $(OBJDIR)/boot.o \
       $(OBJDIR)/kernel.o \
       $(OBJDIR)/vga_text.o \
       $(OBJDIR)/keyboard.o \
       $(OBJDIR)/mouse.o \
       $(OBJDIR)/gfx.o \
       $(OBJDIR)/wm.o \
       $(OBJDIR)/pong.o \
       $(OBJDIR)/snake.o \
       $(OBJDIR)/breakout.o

all: myos.iso

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/boot.o: boot/boot.asm | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/kernel.o: kernel/kernel.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/vga_text.o: drivers/vga.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/keyboard.o: drivers/keyboard.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/mouse.o: drivers/mouse.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/gfx.o: gui/gfx.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/wm.o: gui/wm.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/pong.o: games/pong.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/snake.o: games/snake.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/breakout.o: games/breakout.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

myos.iso: $(OBJDIR)/kernel.bin
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(OBJDIR)/kernel.bin $(ISO_DIR)/boot/kernel.bin
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso $(ISO_DIR) 2>/dev/null

clean:
	rm -rf $(OBJDIR) $(ISO_DIR) myos.iso

.PHONY: all clean
