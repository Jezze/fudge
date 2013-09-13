#include <fudge/module.h>
#include <kernel/vfs.h>
#include "system.h"
#include "backend.h"
#include "protocol.h"

static struct system_backend backend;
static struct vfs_protocol protocol;

void system_group_add(struct system_group *group, struct system_node *node)
{

    struct system_node *current;

    node->parent = &group->node;

    if (!group->children)
    {

        group->children = node;

        return;

    }

    for (current = group->children; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = node;

        return;

    }

}

void system_group_remove(struct system_group *group, struct system_node *node)
{

    struct system_node *current;

    node->parent = 0;

    if (group->children == node)
    {

        group->children = group->children->sibling;

        return;

    }

    for (current = group->children; current; current = current->sibling)
    {

        if (current->sibling != node)
            continue;

        current->sibling = current->sibling->sibling;

        return;

    }

}

void system_register_node(struct system_node *node)
{

    system_group_add(&backend.root, node);

}

void system_unregister_node(struct system_node *node)
{

    system_group_remove(&backend.root, node);

}

static void system_init_node(struct system_node *node, unsigned int type, char *name)
{

    memory_clear(node, sizeof (struct system_node));

    node->type = type;
    node->name = name;

}

static unsigned int open_group(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int close_group(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int read_group(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_group *group = (struct system_group *)self;
    struct system_node *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    for (current = group->children; current; current = current->sibling)
    {

        unsigned int l = string_length(current->name);

        c += memory_read(b + c, count - c, current->name, l, offset);
        offset -= (offset > l) ? l : offset;

        if (current->type == SYSTEM_NODETYPE_GROUP)
        {

            c += memory_read(b + c, count - c, "/\n", 2, offset);
            offset -= (offset > 2) ? 2 : offset;

        }

        else
        {

            c += memory_read(b + c, count - c, "\n", 1, offset);
            offset -= (offset > 1) ? 1 : offset;

        }

    }

    return c;

}

static unsigned int write_group(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void system_init_group(struct system_group *group, char *name)
{

    memory_clear(group, sizeof (struct system_group));
    system_init_node(&group->node, SYSTEM_NODETYPE_GROUP, name);

    group->node.open = open_group;
    group->node.close = close_group;
    group->node.read = read_group;
    group->node.write = write_group;

}

void system_init_stream(struct system_stream *stream, char *name, unsigned int (*open)(struct system_node *self), unsigned int (*close)(struct system_node *self), unsigned int (*read)(struct system_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct system_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(stream, sizeof (struct system_stream));
    system_init_node(&stream->node, SYSTEM_NODETYPE_STREAM, name);

    stream->node.open = open;
    stream->node.close = close;
    stream->node.read = read;
    stream->node.write = write;

}

struct vfs_backend *get_backend()
{

    return &backend.base;

}

void init()
{

    system_init_backend(&backend);
    system_init_protocol(&protocol, &backend);
    vfs_register_protocol(&protocol);

}

void destroy()
{

}

