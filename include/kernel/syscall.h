#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TABLE_SIZE 256

extern void syscall_interrupt();
extern unsigned int syscall_handler(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx, unsigned int edi);
extern void syscall_init();

#endif

