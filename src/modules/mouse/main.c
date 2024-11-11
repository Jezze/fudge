#include <fudge.h>
#include <kernel.h>
#include "mouse.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_MOUSEINTERFACE)); i++)
        {

            if (i == index)
                return kernel_encodenodelist(&current->sources, 0);

        }

    }

    return 0;

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct event_mousemove mousemove;

    mousemove.relx = relx;
    mousemove.rely = rely;

    kernel_notify(kernel_encodenodelist(&interface->resource.sources, 0), &interface->resource.targets, EVENT_MOUSEMOVE, sizeof (struct event_mousemove), &mousemove);

}

void mouse_notifyscroll(struct mouse_interface *interface, char relz)
{

    struct event_mousescroll mousescroll;

    mousescroll.relz = relz;

    kernel_notify(kernel_encodenodelist(&interface->resource.sources, 0), &interface->resource.targets, EVENT_MOUSESCROLL, sizeof (struct event_mousescroll), &mousescroll);

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct event_mousepress mousepress;

    mousepress.button = button;

    kernel_notify(kernel_encodenodelist(&interface->resource.sources, 0), &interface->resource.targets, EVENT_MOUSEPRESS, sizeof (struct event_mousepress), &mousepress);

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct event_mouserelease mouserelease;

    mouserelease.button = button;

    kernel_notify(kernel_encodenodelist(&interface->resource.sources, 0), &interface->resource.targets, EVENT_MOUSERELEASE, sizeof (struct event_mouserelease), &mouserelease);

}

void mouse_registerinterface(struct mouse_interface *interface)
{

    resource_register(&interface->resource);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);

}

void mouse_initinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);
    kernel_link(&interface->resource.sources, 0, &interface->resource, 0);

    interface->id = id;

}

void module_init(void)
{

    service_init(&service, "mouse", service_match);
    service_register(&service);

}

