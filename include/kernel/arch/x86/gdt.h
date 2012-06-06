#ifndef ARCH_GDT_H
#define ARCH_GDT_H

#define GDT_TABLE_SLOTS       6
#define GDT_ENTRY_NULL        0
#define GDT_ENTRY_KERNEL_CODE 1
#define GDT_ENTRY_KERNEL_DATA 2
#define GDT_ENTRY_USER_CODE   3
#define GDT_ENTRY_USER_DATA   4
#define GDT_ENTRY_TSS         5

struct gdt_entry
{

    unsigned short limitLow;
    unsigned short baseLow;
    unsigned char baseMiddle;
    unsigned char access;
    unsigned char granularity;
    unsigned char baseHigh;

} __attribute__((packed));

struct gdt_pointer
{

    unsigned short limit;
    struct gdt_entry *base;

} __attribute__((packed));

void gdt_init();
void gdt_set_gate(unsigned int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity);

#endif

