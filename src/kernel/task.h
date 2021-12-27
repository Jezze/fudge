#define TASK_STATE_NORMAL               0
#define TASK_STATE_KILLED               1
#define TASK_STATE_BLOCKED              2

struct task_thread
{

    unsigned int ip;
    unsigned int sp;

};

struct task
{

    struct resource resource;
    struct list_item item;
    struct task_thread thread;
    struct binary_node node;
    struct spinlock spinlock;
    struct binary_format *format;
    unsigned int id;
    unsigned int state;
    unsigned int kicked;

};

void task_setstate(struct task *task, unsigned int state);
void task_register(struct task *task);
void task_unregister(struct task *task);
void task_initthread(struct task_thread *thread);
void task_init(struct task *task, unsigned int id);
