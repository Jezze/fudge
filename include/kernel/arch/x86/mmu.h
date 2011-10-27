#ifndef ARCH_MMU_H
#define ARCH_MMU_H

#define MMU_ERROR_PRESENT  1 << 0
#define MMU_ERROR_RW       1 << 1
#define MMU_ERROR_USER     1 << 2
#define MMU_ERROR_RESERVED 1 << 3
#define MMU_ERROR_FETCH    1 << 4

#define MMU_DIRECTORY_SIZE    1024

#define MMU_TABLE_SIZE              1024
#define MMU_TABLE_FLAG_PRESENT      1 << 0
#define MMU_TABLE_FLAG_WRITEABLE    1 << 1
#define MMU_TABLE_FLAG_USERMODE     1 << 2
#define MMU_TABLE_FLAG_CACHEWRITE   1 << 3
#define MMU_TABLE_FLAG_CACHEDISABLE 1 << 4
#define MMU_TABLE_FLAG_ACCESSED     1 << 5
#define MMU_TABLE_FLAG_LARGE        1 << 6
#define MMU_TABLE_FLAG_IGNORED      1 << 7

#define MMU_PAGE_SIZE              4096
#define MMU_PAGE_FLAG_PRESENT      1 << 0
#define MMU_PAGE_FLAG_WRITEABLE    1 << 1
#define MMU_PAGE_FLAG_USERMODE     1 << 2
#define MMU_PAGE_FLAG_CACHEWRITE   1 << 3
#define MMU_PAGE_FLAG_CACHEDISABLE 1 << 4
#define MMU_PAGE_FLAG_ACCESSED     1 << 5
#define MMU_PAGE_FLAG_DIRTY        1 << 6
#define MMU_PAGE_FLAG_GLOBAL       1 << 7

struct isr_registers;

struct mmu_table
{

    unsigned int entries[MMU_TABLE_SIZE];

} __attribute__((aligned(MMU_PAGE_SIZE)));

struct mmu_directory
{

    struct mmu_table *tables[MMU_DIRECTORY_SIZE];

} __attribute__((aligned(MMU_PAGE_SIZE)));

struct mmu_header
{

    void *address;
    struct mmu_directory directory;
    struct mmu_table table;

};

extern void mmu_init();
extern void mmu_set_directory(void *paddress);
extern void mmu_map(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags);
extern void mmu_enable();
extern struct mmu_header *mmu_get_header(void *paddress);
extern void *mmu_get_paddress(unsigned int pid);

#endif

