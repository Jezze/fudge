#include "memory.h"
#include "list.h"

void list_add(struct list *list, struct list_item *item)
{

    if (!list->head)
    {

        list->head = item;
        list->tail = item;

        return;

    }

    item->prev = list->tail;
    item->next = 0;
    list->tail->next = item;
    list->tail = item;

}

void list_remove(struct list *list, struct list_item *item)
{

    if (!list->head)
        return;

    if (list->head == item)
        list->head = item->next;

    if (list->tail == item)
        list->tail = item->prev;

    if (list->head)
        list->head->prev = 0;

    if (list->tail)
        list->tail->next = 0;

    if (item->next)
        item->next->prev = item->prev;

    if (item->prev)
        item->prev->next = item->next;

    item->next = 0;
    item->prev = 0;

}

void list_move(struct list *out, struct list *in, struct list_item *item)
{

    list_remove(in, item);
    list_add(out, item);

}

unsigned int list_count(struct list *list)
{

    struct list_item *current;
    unsigned int count = 0;

    for (current = list->head; current; current = current->next)
        count++;
 
    return count;

}

void list_inititem(struct list_item *item, void *data)
{

    memory_clear(item, sizeof (struct list_item));

    item->data = data;

}

void list_init(struct list *list)
{

    memory_clear(list, sizeof (struct list));

}

