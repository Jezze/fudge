#include <lib/file.h>
#include <kernel/modules.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/pit/pit.h>

void pit_handler(struct isr_registers *registers)
{

}

void pit_wait(int ticks)
{

}

void pit_init()
{

    unsigned short divisor = PIT_HERTZ / PIT_FREQUENCY;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(divisor & 0xFF));
    io_outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));

    irq_register_handler(IRQ_ROUTINE_PIT, pit_handler);

}

