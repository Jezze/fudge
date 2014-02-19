#include <fudge/ascii.h>
#include <fudge/memory.h>
#include "tar.h"

unsigned int tar_readvalue(char *attribute)
{

    return ascii_read_value(attribute, ascii_length(attribute), 8);

}

unsigned int tar_validate(void *buffer)
{

    struct tar_header *header = buffer;
    unsigned int checksum = tar_readvalue(header->checksum);
    unsigned char *address = buffer;
    unsigned int i = 0;

    for (; i < 148; i++)
        checksum -= address[i];

    for (; i < 156; i++)
        checksum -= 32;

    for (; i < TAR_BLOCK_SIZE; i++)
        checksum -= address[i];

    return !checksum;

}

unsigned int tar_next(struct tar_header *header, unsigned int offset)
{

    unsigned int size = tar_readvalue(header->size);

    return offset + ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 2 : 1)) * TAR_BLOCK_SIZE;

}

