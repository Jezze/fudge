#include <memory.h>
#include <runtime.h>

static struct runtime_task tasks[RUNTIME_TASK_SLOTS];

static struct runtime_descriptor *get_descriptor(struct runtime_task *self, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_DESCRIPTOR_SLOTS)
        return 0;

    return &self->descriptors[index];

}

static struct runtime_mount *get_mount(struct runtime_task *self, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_MOUNT_SLOTS)
        return 0;

    return &self->mounts[index];

}

static struct runtime_mount *find_mount(struct runtime_task *self, void *path)
{

    unsigned int i;
    unsigned int max = 0;
    struct runtime_mount *current = 0;

    for (i = 1; i < RUNTIME_TASK_MOUNT_SLOTS - 1; i++)
    {

        if (!self->mounts[i].id)
            continue;

        if (self->mounts[i].count < max)
            continue;

        if (memory_match(self->mounts[i].path, path, self->mounts[i].count))
        {

            current = &self->mounts[i];
            max = self->mounts[i].count;

        }

    }

    return current;

}

struct runtime_task *runtime_get_task(unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_SLOTS)
        return 0;

    return &tasks[index];

}

unsigned int runtime_get_task_slot()
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_SLOTS - 1; i++)
    {

        if (!tasks[i].used)
            return i;

    }

    return 0;

}

void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct runtime_mount *mount)
{

    memory_clear(descriptor, sizeof (struct runtime_descriptor));

    descriptor->id = id;
    descriptor->mount = mount;

}

void runtime_memory_init(struct runtime_memory *memory, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    memory_clear(memory, sizeof (struct runtime_memory));

    memory->paddress = paddress;
    memory->vaddress = vaddress;
    memory->size = size;

}

void runtime_mount_init(struct runtime_mount *mount, unsigned int id, struct modules_filesystem *filesystem, unsigned int count, char *path)
{

    memory_clear(mount, sizeof (struct runtime_mount));

    mount->id = id;
    mount->filesystem = filesystem;
    mount->count = count;
    memory_copy(mount->path, path, mount->count);

}

void runtime_registers_init(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int sb)
{

    memory_clear(registers, sizeof (struct runtime_registers));

    registers->ip = ip;
    registers->sp = sp;
    registers->sb = sb;

}

void runtime_task_init(struct runtime_task *task, unsigned int id)
{

    unsigned int address;

    memory_clear(task, sizeof (struct runtime_task));

    task->id = id;
    task->idle = 1;
    task->get_descriptor = get_descriptor;
    task->get_mount = get_mount;
    task->find_mount = find_mount;

    address = RUNTIME_TASK_ADDRESS_BASE + task->id * RUNTIME_TASK_ADDRESS_SIZE;

    runtime_memory_init(&task->memory, address, address, RUNTIME_TASK_ADDRESS_SIZE);

}

void runtime_task_clone(struct runtime_task *task, struct runtime_task *original, unsigned int id)
{

    unsigned int address;

    memory_copy(task, original, sizeof (struct runtime_task));

    task->id = id;
    task->idle = 1;

    address = RUNTIME_TASK_ADDRESS_BASE + task->id * RUNTIME_TASK_ADDRESS_SIZE;

    runtime_memory_init(&task->memory, address, address, RUNTIME_TASK_ADDRESS_SIZE);

}

struct runtime_task *runtime_schedule()
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_SLOTS - 1; i++)
    {

        if (!tasks[i].idle)
            return &tasks[i];

    }

    return 0;

}

struct runtime_task *runtime_setup()
{

    memory_clear(tasks, sizeof (struct runtime_task) * RUNTIME_TASK_SLOTS);

    return tasks;

}

