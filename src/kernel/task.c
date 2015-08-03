#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"

static struct list active;
static struct list inactive;
static struct list blocked;

struct task *task_findactive()
{

    if (!active.tail)
        return 0;

    return active.tail->data;

}

struct task *task_findinactive()
{

    if (!inactive.tail)
        return 0;

    return inactive.tail->data;

}

void task_setstatus(struct task *task, unsigned int status)
{

    switch (status)
    {

    case TASK_STATUS_INACTIVE:
        switch (task->state.status)
        {

        case TASK_STATUS_ACTIVE:
        case TASK_STATUS_UNBLOCKED:
            list_move(&inactive, &active, &task->state.item);

            task->state.status = TASK_STATUS_INACTIVE;

            break;

        }

        break;

    case TASK_STATUS_ACTIVE:
        switch (task->state.status)
        {

        case TASK_STATUS_INACTIVE:
            list_move(&active, &inactive, &task->state.item);

            task->state.status = TASK_STATUS_ACTIVE;

            break;

        case TASK_STATUS_ACTIVE:
        case TASK_STATUS_UNBLOCKED:
            list_move(&active, &active, &task->state.item);

            break;

        case TASK_STATUS_BLOCKED:
            list_move(&active, &blocked, &task->state.item);

            task->state.status = TASK_STATUS_UNBLOCKED;

            break;

        }

        break;

    case TASK_STATUS_BLOCKED:
        switch (task->state.status)
        {

        case TASK_STATUS_ACTIVE:
        case TASK_STATUS_UNBLOCKED:
            list_move(&blocked, &active, &task->state.item);

            task->state.status = TASK_STATUS_BLOCKED;

            break;

        }

        break;

    }

}

void task_copydescriptors(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        target->descriptors[i + 0x00].channel = source->descriptors[i + 0x08].channel;
        target->descriptors[i + 0x00].id = source->descriptors[i + 0x08].id;
        target->descriptors[i + 0x08].channel = source->descriptors[i + 0x08].channel;
        target->descriptors[i + 0x08].id = source->descriptors[i + 0x08].id;
        target->descriptors[i + 0x10].channel = 0;
        target->descriptors[i + 0x10].id = 0;
        target->descriptors[i + 0x18].channel = 0;
        target->descriptors[i + 0x18].id = 0;

    }

}

void task_copybinary(struct task *task)
{

    struct vfs_descriptor *descriptor = &task->descriptors[0x00];
    struct binary_protocol *protocol;

    if (!descriptor->id || !descriptor->channel)
        return;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return;

    protocol->copyprogram(descriptor->channel, descriptor->id);

}

void task_initbinary(struct task *task, unsigned int sp)
{

    struct vfs_descriptor *descriptor = &task->descriptors[0x00];
    struct binary_protocol *protocol;

    if (!descriptor->id || !descriptor->channel)
        return;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return;

    task->state.registers.ip = protocol->findentry(descriptor->channel, descriptor->id);
    task->state.registers.sp = sp;

}

unsigned int task_rmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&task->mailbox.buffer, size * count, buffer);

    if (!count)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    return count;

}

unsigned int task_wmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&task->mailbox.buffer, size * count, buffer);

    if (count)
        task_setstatus(task, TASK_STATUS_ACTIVE);

    return count;

}

void task_register(struct task *task)
{

    list_add(&inactive, &task->state.item);

}

void task_unregister(struct task *task)
{

    list_remove(&inactive, &task->state.item);

}

void task_init(struct task *task)
{

    unsigned int i;

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->state.item, task);
    list_inititem(&task->mailbox.item, task);
    buffer_init(&task->mailbox.buffer, 1, TASK_MAILBOXSIZE, task->mailbox.data);

    for (i = 0; i < TASK_DESCRIPTORS; i++)
        vfs_initdescriptor(&task->descriptors[i]);

}

void task_setup()
{

    list_init(&active);
    list_init(&inactive);
    list_init(&blocked);

}

