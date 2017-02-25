#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "container.h"
#include "task.h"
#include "service.h"
#include "kernel.h"

static struct task_descriptor *getdescriptor(struct container *container, struct task *task, unsigned int descriptor)
{

    return &container->descriptors[task->id * TASK_DESCRIPTORS + descriptor];

}

static void copydescriptor(struct task *target, struct task_descriptor *tdescriptor, struct task_descriptor *sdescriptor)
{

    tdescriptor->server = (sdescriptor) ? sdescriptor->server : 0;
    tdescriptor->id = (sdescriptor) ? sdescriptor->id : 0;

}

void kernel_copydescriptors(struct container *container, struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        copydescriptor(target, getdescriptor(container, target, i + 0x00), getdescriptor(container, source, i + 0x08));
        copydescriptor(target, getdescriptor(container, target, i + 0x08), getdescriptor(container, source, i + 0x08));
        copydescriptor(target, getdescriptor(container, target, i + 0x10), 0);
        copydescriptor(target, getdescriptor(container, target, i + 0x18), 0);

    }

}

void kernel_multicast(struct list *links, void *buffer, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
    {

        struct task_descriptor *descriptor = current->data;

        task_write(descriptor->task, buffer, count);

    }

}

unsigned int kernel_setupbinary(struct container *container, struct task *task, unsigned int sp)
{

    struct task_descriptor *descriptor = getdescriptor(container, task, 0);

    task->node.physical = descriptor->server->protocol->map(descriptor);

    if (!task->node.physical)
        return 0;

    task->format = binary_findformat(&task->node);

    if (!task->format)
        return 0;

    task_resume(task, task->format->findentry(&task->node), sp);

    return 1;

}

void kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct container_server *server = &container->servers[0x00];
    struct container_mount *mount = &container->mounts[0x00];
    struct task_descriptor *init = getdescriptor(container, task, 8);
    struct task_descriptor *root = getdescriptor(container, task, 9);

    server->backend = backend;
    server->protocol = service_findprotocol(backend);
    mount->parent.server = server;
    mount->parent.id = server->protocol->root(backend);
    mount->child.server = server;
    mount->child.id = server->protocol->root(backend);

    root->server = mount->parent.server;
    root->id = mount->parent.id;
    init->server = mount->parent.server;
    init->id = mount->parent.id;

    init->server->protocol->child(init, "bin/", 4);
    init->server->protocol->child(init, "init", 4);

    container->nservers++;
    container->nmounts++;

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

