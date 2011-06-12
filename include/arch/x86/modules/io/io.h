#ifndef ARCH_IO_H
#define ARCH_IO_H

struct modules_io_device
{

    struct modules_module base;
    unsigned int (*read)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(char *buffer, unsigned int count, unsigned int offset);
    struct file_node node;

};

extern unsigned char io_inb(unsigned short port);
extern unsigned short io_inw(unsigned short port);
extern unsigned int io_ind(unsigned short port);
extern void io_outb(unsigned short port, unsigned char data);
extern void io_outw(unsigned short port, unsigned short data);
extern void io_outd(unsigned short port, unsigned int data);
extern void io_init();

#endif

