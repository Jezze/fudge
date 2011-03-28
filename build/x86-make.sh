#!/bin/sh

DIR_IMAGE="root"
DIR_IMAGE_BIN="root/bin"
DIR_IMAGE_BOOT="root/boot"
DIR_SOURCE_ARCH_KERNEL="../arch/x86/kernel"
DIR_SOURCE_ARCH_LIB="../arch/x86/lib"
DIR_SOURCE_ARCH_MODULES="../arch/x86/modules"
DIR_SOURCE_BIN="../bin"
DIR_SOURCE_KERNEL="../kernel"
DIR_SOURCE_LIB="../lib"
DIR_SOURCE_MODULES="../modules"

ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"

$GCC $GCCFLAGS $DIR_SOURCE_LIB/file.c -o $DIR_SOURCE_LIB/file.o
$GCC $GCCFLAGS $DIR_SOURCE_LIB/memory.c -o $DIR_SOURCE_LIB/memory.o
$GCC $GCCFLAGS $DIR_SOURCE_LIB/session.c -o $DIR_SOURCE_LIB/session.o
$GCC $GCCFLAGS $DIR_SOURCE_LIB/string.c -o $DIR_SOURCE_LIB/string.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_LIB/calls.s -o $DIR_SOURCE_ARCH_LIB/calls.o

ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"

$GCC $GCCFLAGS $DIR_SOURCE_MODULES/elf/elf.c -o $DIR_SOURCE_MODULES/elf/elf.o
$GCC $GCCFLAGS $DIR_SOURCE_MODULES/tty/tty.c -o $DIR_SOURCE_MODULES/tty/tty.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/ata/ata.c -o $DIR_SOURCE_ARCH_MODULES/ata/ata.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/io/io.c -o $DIR_SOURCE_ARCH_MODULES/io/io.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_MODULES/io/ios.s -o $DIR_SOURCE_ARCH_MODULES/io/ios.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/kbd/kbd.c -o $DIR_SOURCE_ARCH_MODULES/kbd/kbd.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/pci/pci.c -o $DIR_SOURCE_ARCH_MODULES/pci/pci.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/pit/pit.c -o $DIR_SOURCE_ARCH_MODULES/pit/pit.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/rtc/rtc.c -o $DIR_SOURCE_ARCH_MODULES/rtc/rtc.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/serial/serial.c -o $DIR_SOURCE_ARCH_MODULES/serial/serial.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_MODULES/vga/vga.c -o $DIR_SOURCE_ARCH_MODULES/vga/vga.o

ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"

$GCC $GCCFLAGS $DIR_SOURCE_KERNEL/initrd.c -o $DIR_SOURCE_KERNEL/initrd.o
$GCC $GCCFLAGS $DIR_SOURCE_KERNEL/kernel.c -o $DIR_SOURCE_KERNEL/kernel.o
$GCC $GCCFLAGS $DIR_SOURCE_KERNEL/log.c -o $DIR_SOURCE_KERNEL/log.o
$GCC $GCCFLAGS $DIR_SOURCE_KERNEL/modules.c -o $DIR_SOURCE_KERNEL/modules.o
$GCC $GCCFLAGS $DIR_SOURCE_KERNEL/shell.c -o $DIR_SOURCE_KERNEL/shell.o
$GCC $GCCFLAGS $DIR_SOURCE_KERNEL/vfs.c -o $DIR_SOURCE_KERNEL/vfs.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/arch.c -o $DIR_SOURCE_ARCH_KERNEL/arch.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/cpu.s -o $DIR_SOURCE_ARCH_KERNEL/cpu.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/fpu.c -o $DIR_SOURCE_ARCH_KERNEL/fpu.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/fpus.s -o $DIR_SOURCE_ARCH_KERNEL/fpus.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/gdt.c -o $DIR_SOURCE_ARCH_KERNEL/gdt.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/gdts.s -o $DIR_SOURCE_ARCH_KERNEL/gdts.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/idt.c -o $DIR_SOURCE_ARCH_KERNEL/idt.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/idts.s -o $DIR_SOURCE_ARCH_KERNEL/idts.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/init.s -o $DIR_SOURCE_ARCH_KERNEL/init.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/irq.c -o $DIR_SOURCE_ARCH_KERNEL/irq.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/irqs.s -o $DIR_SOURCE_ARCH_KERNEL/irqs.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/isr.c -o $DIR_SOURCE_ARCH_KERNEL/isr.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/isrs.s -o $DIR_SOURCE_ARCH_KERNEL/isrs.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/mmu.c -o $DIR_SOURCE_ARCH_KERNEL/mmu.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/syscall.c -o $DIR_SOURCE_ARCH_KERNEL/syscall.o
$GCC $GCCFLAGS $DIR_SOURCE_ARCH_KERNEL/tss.c -o $DIR_SOURCE_ARCH_KERNEL/tss.o
$ASM $ASMFLAGS $DIR_SOURCE_ARCH_KERNEL/tsss.s -o $DIR_SOURCE_ARCH_KERNEL/tsss.o

LD="ld"
LDFLAGS=-"T./linker-x86.ld"

$LD $LDFLAGS \
    $DIR_SOURCE_KERNEL/initrd.o \
    $DIR_SOURCE_KERNEL/kernel.o \
    $DIR_SOURCE_KERNEL/log.o \
    $DIR_SOURCE_KERNEL/modules.o \
    $DIR_SOURCE_KERNEL/shell.o \
    $DIR_SOURCE_KERNEL/vfs.o \
    $DIR_SOURCE_MODULES/elf/elf.o \
    $DIR_SOURCE_MODULES/tty/tty.o \
    $DIR_SOURCE_ARCH_KERNEL/arch.o \
    $DIR_SOURCE_ARCH_KERNEL/cpu.o \
    $DIR_SOURCE_ARCH_KERNEL/fpu.o \
    $DIR_SOURCE_ARCH_KERNEL/fpus.o \
    $DIR_SOURCE_ARCH_KERNEL/gdt.o \
    $DIR_SOURCE_ARCH_KERNEL/gdts.o \
    $DIR_SOURCE_ARCH_KERNEL/idt.o \
    $DIR_SOURCE_ARCH_KERNEL/idts.o \
    $DIR_SOURCE_ARCH_KERNEL/init.o \
    $DIR_SOURCE_ARCH_KERNEL/irqs.o \
    $DIR_SOURCE_ARCH_KERNEL/isrs.o \
    $DIR_SOURCE_ARCH_KERNEL/irq.o \
    $DIR_SOURCE_ARCH_KERNEL/isr.o \
    $DIR_SOURCE_ARCH_KERNEL/mmu.o \
    $DIR_SOURCE_ARCH_KERNEL/syscall.o \
    $DIR_SOURCE_ARCH_KERNEL/tss.o \
    $DIR_SOURCE_ARCH_KERNEL/tsss.o \
    $DIR_SOURCE_ARCH_MODULES/ata/ata.o \
    $DIR_SOURCE_ARCH_MODULES/io/io.o \
    $DIR_SOURCE_ARCH_MODULES/io/ios.o \
    $DIR_SOURCE_ARCH_MODULES/kbd/kbd.o \
    $DIR_SOURCE_ARCH_MODULES/pci/pci.o \
    $DIR_SOURCE_ARCH_MODULES/pit/pit.o \
    $DIR_SOURCE_ARCH_MODULES/rtc/rtc.o \
    $DIR_SOURCE_ARCH_MODULES/serial/serial.o \
    $DIR_SOURCE_ARCH_MODULES/vga/vga.o \
    $DIR_SOURCE_LIB/memory.o \
    $DIR_SOURCE_LIB/file.o \
    $DIR_SOURCE_LIB/session.o \
    $DIR_SOURCE_LIB/string.o \
    $DIR_SOURCE_ARCH_LIB/calls.o \
    -o root/boot/kernel

ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include/lib -Wall -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"

$GCC $GCCFLAGS $DIR_SOURCE_BIN/cat.c -o $DIR_SOURCE_BIN/cat.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/cd.c -o $DIR_SOURCE_BIN/cd.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/clear.c -o $DIR_SOURCE_BIN/clear.o
$ASM $ASMFLAGS $DIR_SOURCE_BIN/cpu.s -o $DIR_SOURCE_BIN/cpus.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/cpu.c -o $DIR_SOURCE_BIN/cpu.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/date.c -o $DIR_SOURCE_BIN/date.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/echo.c -o $DIR_SOURCE_BIN/echo.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/elf.c -o $DIR_SOURCE_BIN/elf.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/hello.c -o $DIR_SOURCE_BIN/hello.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/ls.c -o $DIR_SOURCE_BIN/ls.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/reboot.c -o $DIR_SOURCE_BIN/reboot.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/shell.c -o $DIR_SOURCE_BIN/shell.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/tar.c -o $DIR_SOURCE_BIN/tar.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/timer.c -o $DIR_SOURCE_BIN/timer.o
$GCC $GCCFLAGS $DIR_SOURCE_BIN/vga.c -o $DIR_SOURCE_BIN/vga.o

LD="ld"
LDFLAGS="-e main"

$LD $LDFLAGS $DIR_SOURCE_BIN/cat.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/cat
$LD $LDFLAGS $DIR_SOURCE_BIN/cd.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/cd
$LD $LDFLAGS $DIR_SOURCE_BIN/clear.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/clear
$LD $LDFLAGS $DIR_SOURCE_BIN/cpu.o $DIR_SOURCE_BIN/cpus.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/cpu
$LD $LDFLAGS $DIR_SOURCE_BIN/date.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/date
$LD $LDFLAGS $DIR_SOURCE_BIN/echo.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/echo
$LD $LDFLAGS $DIR_SOURCE_BIN/elf.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/elf
$LD $LDFLAGS $DIR_SOURCE_BIN/hello.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/hello
$LD $LDFLAGS $DIR_SOURCE_BIN/ls.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/ls
$LD $LDFLAGS $DIR_SOURCE_BIN/reboot.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/reboot
$LD $LDFLAGS $DIR_SOURCE_BIN/shell.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/shell
$LD $LDFLAGS $DIR_SOURCE_BIN/tar.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/tar
$LD $LDFLAGS $DIR_SOURCE_BIN/timer.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/timer
$LD $LDFLAGS $DIR_SOURCE_BIN/vga.o $DIR_SOURCE_LIB/memory.o $DIR_SOURCE_LIB/session.o $DIR_SOURCE_LIB/string.o $DIR_SOURCE_LIB/file.o $DIR_SOURCE_ARCH_LIB/calls.o -o $DIR_IMAGE_BIN/vga

TAR="tar"
TARFLAGS="-cvf"

$TAR $TARFLAGS $DIR_IMAGE_BOOT/initrd.tar $DIR_IMAGE

