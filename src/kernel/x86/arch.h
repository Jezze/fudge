unsigned short arch_generalfault(void *stack);
unsigned short arch_pagefault(void *stack);
unsigned short arch_syscall(void *stack);
void arch_setinterrupt(unsigned char index, void (*callback)());
void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int count);
unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_setup(struct vfs_backend *backend);
