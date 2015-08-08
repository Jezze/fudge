#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "container.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(struct container *container, struct task *task, void *stack);

static struct vfs_descriptor *getdescriptor(struct task *task, unsigned int descriptor)
{

    return &task->descriptors[descriptor & (TASK_DESCRIPTORS - 1)];

}

static struct vfs_channel *getchannel(struct container *container, unsigned int channel)
{

    return &container->channels[channel & (CONTAINER_CHANNELS - 1)];

}

static struct vfs_mount *getmount(struct container *container, unsigned int mount)
{

    return &container->mounts[mount & (CONTAINER_MOUNTS - 1)];

}

static unsigned int auth(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int channel; unsigned int backend;} *args = stack;
    struct vfs_channel *channel = getchannel(container, args->channel);

    channel->backend = vfs_findbackend(args->backend);

    if (!channel->backend)
        return 0;

    channel->protocol = vfs_findprotocol(channel->backend);

    if (!channel->protocol)
        return 0;

    return 1;

}

static unsigned int walk(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; unsigned int count; char *path;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    unsigned int offset;
    unsigned int count;

    if (!pdescriptor->id || !pdescriptor->channel)
        return 0;

    descriptor->channel = pdescriptor->channel;
    descriptor->id = pdescriptor->id;

    for (offset = 0; (count = memory_findbyte(args->path + offset, args->count - offset, '/')); offset += count)
    {

        char *path = args->path + offset;
        unsigned int i;

        if (count == 3 && memory_match(path, "../", 3))
        {

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct vfs_mount *mount = &container->mounts[i];

                if (descriptor->channel == mount->child.channel && descriptor->id == mount->child.id)
                {

                    descriptor->channel = mount->parent.channel;
                    descriptor->id = mount->parent.id;

                    break;

                }

            }

            descriptor->id = descriptor->channel->protocol->parent(descriptor->channel->backend, descriptor->id);

            if (!descriptor->id)
                return 0;

        }

        else
        {

            descriptor->id = descriptor->channel->protocol->child(descriptor->channel->backend, descriptor->id, count, args->path + offset);

            if (!descriptor->id)
                return 0;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct vfs_mount *mount = &container->mounts[i];

                if (descriptor->channel == mount->parent.channel && descriptor->id == mount->parent.id)
                {

                    descriptor->channel = mount->child.channel;
                    descriptor->id = mount->child.id;

                    break;

                }

            }

        }

    }

    return descriptor->id;

}

static unsigned int create(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int count; char *name;} *args = stack;
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);

    if (!pdescriptor->id || !pdescriptor->channel)
        return 0;

    return pdescriptor->channel->protocol->create(pdescriptor->channel->backend, pdescriptor->id, args->count, args->name);

}

static unsigned int destroy(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int count; char *name;} *args = stack;
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);

    if (!pdescriptor->id || !pdescriptor->channel)
        return 0;

    return pdescriptor->channel->protocol->destroy(pdescriptor->channel->backend, pdescriptor->id, args->count, args->name);

}

static unsigned int open(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->id = descriptor->channel->protocol->open(descriptor->channel->backend, descriptor->id);

}

static unsigned int close(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->id = descriptor->channel->protocol->close(descriptor->channel->backend, descriptor->id);

}

static unsigned int read(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset; unsigned int size; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->channel->protocol->read(descriptor->channel->backend, descriptor->id, args->offset, args->size, args->count, args->buffer);

}

static unsigned int write(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset; unsigned int size; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->channel->protocol->write(descriptor->channel->backend, descriptor->id, args->offset, args->size, args->count, args->buffer);

}

static unsigned int mount(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int channel; unsigned int descriptor;} *args = stack;
    struct vfs_mount *mount = getmount(container, args->mount);
    struct vfs_channel *channel = getchannel(container, args->channel);
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->descriptor);

    if (!channel->backend || !channel->protocol || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = channel;
    mount->child.id = channel->protocol->root(channel->backend);

    return 1;

}

static unsigned int bind(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct vfs_mount *mount = getmount(container, args->mount);
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    struct vfs_descriptor *cdescriptor = getdescriptor(task, args->cdescriptor);

    if (!pdescriptor->id || !pdescriptor->channel || !cdescriptor->id || !cdescriptor->channel)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = cdescriptor->channel;
    mount->child.id = cdescriptor->id;

    return 1;

}

static unsigned int load(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct binary_protocol *protocol;
    unsigned long physical;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!descriptor->id || !descriptor->channel || !descriptor->channel->protocol->getphysical)
        return 0;

    /* Physical should be replaced with known address later on */
    physical = descriptor->channel->protocol->getphysical(descriptor->channel->backend, descriptor->id);

    if (!physical)
        return 0;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol || !protocol->relocate(descriptor->channel, descriptor->id, physical))
        return 0;

    module_init = (void (*)(void))(protocol->findsymbol(descriptor->channel, descriptor->id, 11, "module_init"));

    if (module_init)
        module_init();

    module_register = (void (*)(void))(protocol->findsymbol(descriptor->channel, descriptor->id, 15, "module_register"));

    if (module_register)
        module_register();

    return 1;

}

static unsigned int unload(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct binary_protocol *protocol;
    void (*module_unregister)(void);

    if (!descriptor->id || !descriptor->channel)
        return 0;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    module_unregister = (void (*)(void))(protocol->findsymbol(descriptor->channel, descriptor->id, 17, "module_unregister"));

    if (module_unregister)
        module_unregister();

    return 1;

}

static unsigned int undefined(struct container *container, struct task *task, void *stack)
{

    return 0;

}

unsigned int abi_call(unsigned int index, struct container *container, struct task *task, void *stack)
{

    return calls[index & (CALLS - 1)](container, task, stack);

}

void abi_setup(unsigned int (*spawn)(struct container *container, struct task *task, void *stack), unsigned int (*despawn)(struct container *container, struct task *task, void *stack))
{

    calls[0x00] = auth;
    calls[0x01] = walk;
    calls[0x02] = create;
    calls[0x03] = destroy;
    calls[0x04] = open;
    calls[0x05] = close;
    calls[0x06] = read;
    calls[0x07] = write;
    calls[0x08] = mount;
    calls[0x09] = bind;
    calls[0x0A] = load;
    calls[0x0B] = unload;
    calls[0x0C] = spawn;
    calls[0x0D] = despawn;
    calls[0x0E] = undefined;
    calls[0x0F] = undefined;

}

