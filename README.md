A minimal x86 operating system written in **C and Assembly**, designed for learning about low-level programming, computer architecture, and operating system development.

MyOS provides a simple text-based shell, hardware information utilities, and basic system control commands while keeping the codebase lightweight and easy to understand.

---

## Features

- Written in **C + x86 Assembly**
- Interactive command-line shell
- VGA text mode output
- Multiboot-compatible boot process
- CPU vendor and brand string detection
- Memory information from Multiboot
- VGA color demonstration
- Basic uptime counter using the CPU Time Stamp Counter (TSC)
- Software reboot support
- CPU halt support
- Small and educational codebase

---

## Command List

| Command | Description |
|----------|-------------|
| `help` | Lists all available commands |
| `hello` | Displays a friendly greeting |
| `about` | Shows information about MyOS and credits |
| `meminfo` | Displays available RAM reported by Multiboot |
| `cpuinfo` | Shows CPU vendor and full processor brand string (if supported) |
| `clear` | Clears the screen |
| `color` | Displays all 16 VGA text colors |
| `echo <text>` | Prints the supplied text |
| `uptime` | Displays the current TSC tick counter |
| `reboot` | Reboots the machine |
| `halt` | Halts the CPU |

---
## Project Goals

MyOS exists primarily as a learning project focused on:

- x86 architecture
- Bootloaders
- Protected Mode
- Kernel development
- VGA programming
- Low-level hardware interaction
- Assembly and C interoperability

---

## Building

### Requirements

- GCC Cross Compiler (`i686-elf-gcc`)
- NASM
- GNU Make
- GRUB tools
- QEMU (recommended)

### Build

```bash
make
```

### Run in QEMU

```bash
qemu-system-i386 -kernel myos.bin
```

or

```bash
qemu-system-i386 -cdrom myos.iso
```

depending on your build configuration.

---

## Credits

### Drippydingus

Creator of MyOS.

---

## Disclaimer

MyOS is an educational hobby operating system. It is not intended for production use and may lack features commonly found in modern operating systems.

---

## License

This project is released for educational purposes. Feel free to study, modify, and experiment with the source code.
