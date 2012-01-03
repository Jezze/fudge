#include <kernel/kernel.h>
#include <kernel/arch/arm/arch.h>
#include <kernel/arch/arm/cpu.h>

static struct arch_arm arm;

#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)
 
void putc(char c)
{

    while (*(volatile unsigned long*)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & (SERIAL_BUFFER_FULL));

    *(volatile unsigned long*)SERIAL_BASE = c;

}
                                                         
void puts(const char *s)
{

    while (*s)
        putc(*s++);

}

static void arch_arm_setup(struct kernel_arch *arch)
{

    struct arch_arm *arm = (struct arch_arm *)arch;

    puts("Fudge Console Text\n\n");

}

void arch_init()
{

    arm.base.setup = arch_arm_setup;
    arm.base.enable_interrupts = cpu_enable_interrupts;
    arm.base.disable_interrupts = cpu_disable_interrupts;

    kernel_register_arch(&arm.base);
    kernel_init();

}

