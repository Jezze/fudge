#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "video.h"

static struct system_node root;

void video_notifymode(struct video_interface *interface, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct {struct event_header header; struct event_videomode videomode;} message;

    message.videomode.w = w;
    message.videomode.h = h;
    message.videomode.bpp = bpp;

    event_initheader(&message.header, EVENT_VIDEOMODE, EVENT_ADDR_SELF, EVENT_ADDR_BROADCAST, sizeof (struct event_videomode));
    kernel_multicastevent(&interface->event.states, &message.header);

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->colormap);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->colormap);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void video_initinterface(struct video_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap");
    system_initnode(&interface->event, SYSTEM_NODETYPE_MAILBOX, "event");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "video");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

