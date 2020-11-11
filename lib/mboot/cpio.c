#include <fudge.h>
#include <kernel.h>
#include "cpio.h"

static struct service_protocol protocol;
static unsigned int address;
static unsigned int limit;

static unsigned int read(void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, (void *)address, limit, offset);

}

static unsigned int write(void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write((void *)address, limit, buffer, count, offset);

}

static unsigned int map(unsigned int offset, unsigned int count)
{

    return address + offset;

}

static struct cpio_header *mapheader(unsigned int id)
{

    struct cpio_header *header = (struct cpio_header *)map(id, sizeof (struct cpio_header));

    return (cpio_validate(header)) ? header : 0;

}

static char *mapname(struct cpio_header *header, unsigned int id)
{

    return (char *)map(id + sizeof (struct cpio_header), header->namesize);

}

static unsigned int root(void)
{

    struct cpio_header *eheader;
    unsigned int id = 0;
    unsigned int current = 0;

    do
    {

        eheader = mapheader(current);

        if (!eheader)
            break;

        if ((eheader->mode & 0xF000) != 0x4000)
            continue;

        id = current;

    } while ((current = cpio_next(eheader, current)));

    return id;

}

static unsigned int parent(struct cpio_header *header, unsigned int id)
{

    unsigned int length = memory_findlastbyte(mapname(header, id), header->namesize - 1, '/');
    struct cpio_header *eheader;
    unsigned int current = id;

    do
    {

        eheader = mapheader(current);

        if (!eheader)
            break;

        if ((eheader->mode & 0xF000) != 0x4000)
            continue;

        if (eheader->namesize == length + 1)
            return current;

    } while ((current = cpio_next(eheader, current)));

    return id;

}

static unsigned int child(struct cpio_header *header, unsigned int id, char *path, unsigned int length)
{

    struct cpio_header *eheader;
    unsigned int current = 0;

    do
    {

        char *name;

        eheader = mapheader(current);

        if (!eheader)
            break;

        if (eheader->namesize != header->namesize + length + 1)
            continue;

        name = mapname(eheader, current);

        if (!name)
            break;

        if (memory_match(name + header->namesize, path, length))
            return current;

    } while ((current = cpio_next(eheader, current)));

    return id;

}

static unsigned int protocol_root(void)
{

    return root();

}

static unsigned int protocol_parent(unsigned int id)
{

    struct cpio_header *header = mapheader(id);

    if (!header)
        return 0;

    return parent(header, id);

}

static unsigned int protocol_child(unsigned int id, char *path, unsigned int length)
{

    struct cpio_header *header = mapheader(id);

    if (!header)
        return 0;

    return child(header, id, path, length);

}

static unsigned int protocol_create(unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int stepdirectory(unsigned int id, unsigned int current)
{

    struct cpio_header *eheader = mapheader(current);

    if (!eheader)
        return 0;

    while ((current = cpio_next(eheader, current)))
    {

        if (current == id)
            break;

        eheader = mapheader(current);

        if (!eheader)
            break;

        if (parent(eheader, current) == id)
            return current;

    }

    return 0;

}

static unsigned int protocol_step(unsigned int id, unsigned int current)
{

    struct cpio_header *header = mapheader(id);

    if (!header)
        return 0;

    switch (header->mode & 0xF000)
    {

    case 0x4000:
        return stepdirectory(id, (id == current) ? 0 : current);

    }

    return 0;

}

static unsigned int protocol_open(struct service_state *state, unsigned int id)
{

    return id;

}

static unsigned int protocol_close(struct service_state *state, unsigned int id)
{

    return id;

}

static unsigned int readfile(struct service_state *state, void *buffer, unsigned int count, unsigned int offset, unsigned int id, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - offset;
    unsigned int o = id + cpio_filedata(header) + offset;

    return read(buffer, (count > s) ? s : count, o);

}

static unsigned int readdirectory(struct service_state *state, void *buffer, unsigned int count, unsigned int offset, unsigned int current, struct cpio_header *header)
{

    struct record record;
    struct cpio_header *eheader;
    char *name;

    if (!current)
        return 0;

    eheader = mapheader(current);

    if (!eheader)
        return 0;

    name = mapname(eheader, current);

    if (!name)
        return 0;

    record.id = current;
    record.size = cpio_filesize(eheader);
    record.length = memory_read(record.name, RECORD_NAMESIZE, name, eheader->namesize - 1, header->namesize);

    return memory_read(buffer, count, &record, sizeof (struct record), offset);

}

static unsigned int protocol_read(struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = mapheader(id);

    if (!header)
        return 0;

    switch (header->mode & 0xF000)
    {

    case 0x8000:
        return readfile(state, buffer, count, offset, id, header);

    case 0x4000:
        return readdirectory(state, buffer, count, offset, current, header);

    }

    return 0;

}

static unsigned int writefile(struct service_state *state, void *buffer, unsigned int count, unsigned int offset, unsigned int id, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - offset;
    unsigned int o = id + cpio_filedata(header) + offset;

    return write(buffer, (count > s) ? s : count, o);

}

static unsigned int protocol_write(struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = mapheader(id);

    if (!header)
        return 0;

    switch (header->mode & 0xF000)
    {

    case 0x8000:
        return writefile(state, buffer, count, offset, id, header);

    }

    return 0;

}

static unsigned int protocol_seek(unsigned int id, unsigned int offset)
{

    return offset;

}

static unsigned int protocol_map(unsigned int id)
{

    struct cpio_header *header = mapheader(id);

    if (!header)
        return 0;

    return map(id + cpio_filedata(header), cpio_filesize(header));

}

void cpio_setup(unsigned int addr, unsigned int lim)
{

    address = addr;
    limit = lim;

    service_initprotocol(&protocol, 1000, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

