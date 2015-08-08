#include <fudge.h>
#include "resource.h"

static struct list resources;

struct resource *resource_find(struct resource *resource)
{

    struct list_item *current = (resource) ? resource->item.next : resources.head;

    if (!current)
        return 0;

    return current->data;

}

struct resource *resource_findtype(struct resource *resource, unsigned int type)
{

    struct resource *current = resource;

    while ((current = resource_find(current)))
    {

        if (current->type == type)
            return current;

    }

    return 0;

}

void resource_register(struct resource *resource)
{

    list_add(&resources, &resource->item);

}

void resource_unregister(struct resource *resource)
{

    list_remove(&resources, &resource->item);

}

void resource_init(struct resource *resource, unsigned int type, void *data)
{

    list_inititem(&resource->item, resource);

    resource->type = type;
    resource->data = data;

}

void resource_setup(void)
{

    list_init(&resources);

}

