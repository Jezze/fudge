#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "element.h"
#include "send.h"

#define CLIENTS                         64
#define VIEWS                           8

static struct client
{

    struct list_item item;
    struct element_window window;
    unsigned int source;

} client[CLIENTS];

static struct view
{

    struct list_item item;
    struct list clients;
    unsigned int center;
    struct element_panel panel;
    struct element_text number;
    char *numberstring;
    struct client *clientfocus;

} view[VIEWS];

static struct element_fill background;
static struct element_mouse mouse;
static struct list clients;
static struct ctrl_videosettings oldsettings;
static struct ctrl_videosettings settings;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static struct view *viewfocus;
static struct box screen;
static struct box menu;
static struct box body;
static unsigned int quit;
static void (*handlers[EVENTS])(union event *event);
static unsigned int actionkey;

static void writeelement(unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &element, sizeof (struct element), datacount);

}

static void writefill(unsigned int source, unsigned int z, struct element_fill *fill)
{

    writeelement((unsigned int)fill, ELEMENT_TYPE_FILL, source, z, sizeof (struct element_fill));

    datacount += memory_write(databuffer, FUDGE_BSIZE, fill, sizeof (struct element_fill), datacount);

}

static void writemouse(unsigned int source, unsigned int z, struct element_mouse *mouse)
{

    writeelement((unsigned int)mouse, ELEMENT_TYPE_MOUSE, source, z, sizeof (struct element_mouse));

    datacount += memory_write(databuffer, FUDGE_BSIZE, mouse, sizeof (struct element_mouse), datacount);

}

static void writepanel(unsigned int source, unsigned int z, struct element_panel *panel)
{

    writeelement((unsigned int)panel, ELEMENT_TYPE_PANEL, source, z, sizeof (struct element_panel));

    datacount += memory_write(databuffer, FUDGE_BSIZE, panel, sizeof (struct element_panel), datacount);

}

static void writetext(unsigned int source, unsigned int z, struct element_text *text, unsigned int count, void *buffer)
{

    writeelement((unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void writewindow(unsigned int source, unsigned int z, struct element_window *window)
{

    writeelement((unsigned int)window, ELEMENT_TYPE_WINDOW, source, z, sizeof (struct element_window));

    datacount += memory_write(databuffer, FUDGE_BSIZE, window, sizeof (struct element_window), datacount);

}

static void flush(void)
{

    if (datacount)
    {

        call_write(CALL_PO, datacount, databuffer);

        datacount = 0;

    }

}

static void arrangeclients(unsigned int source, struct view *view)
{

    struct list_item *current;
    struct client *client;
    unsigned int y;
    unsigned int h;

    switch (view->clients.count)
    {

    case 0:
        break;

    case 1:
        client = view->clients.tail->data;

        box_setsize(&client->window.size, body.x, body.y, body.w, body.h);
        writewindow(source, 1, &client->window);
        send_wmresize(CALL_L2, client->source, client->window.size.x, client->window.size.y, client->window.size.w, client->window.size.h);

        break;

    default:
        y = body.y;
        h = body.h / (view->clients.count - 1);
        client = view->clients.tail->data;

        box_setsize(&client->window.size, body.x, body.y, view->center, body.h);
        writewindow(source, 1, &client->window);
        send_wmresize(CALL_L2, client->source, client->window.size.x, client->window.size.y, client->window.size.w, client->window.size.h);

        for (current = view->clients.tail->prev; current; current = current->prev)
        {

            client = current->data;

            box_setsize(&client->window.size, body.x + view->center, y, body.w - view->center, h);
            writewindow(source, 1, &client->window);
            send_wmresize(CALL_L2, client->source, client->window.size.x, client->window.size.y, client->window.size.w, client->window.size.h);

            y += h;

        }

        break;

    }

}

static struct client *focusclient(unsigned int source, struct client *focus, struct client *new)
{

    if (focus == new)
        return new;

    if (focus)
    {

        focus->window.active = 0;

        writewindow(source, 1, &focus->window);

    }

    if (new)
    {

        new->window.active = 1;

        writewindow(source, 1, &new->window);

    }

    return new;

}

static struct client *nextclient(unsigned int source, struct view *view)
{

    struct client *client = view->clientfocus;

    if (!client)
        return 0;

    return focusclient(source, client, (client->item.next) ? client->item.next->data : view->clients.head);

}

static struct client *prevclient(unsigned int source, struct view *view)
{

    struct client *client = view->clientfocus;

    if (!client)
        return 0;

    return focusclient(source, client, (client->item.prev) ? client->item.prev->data : view->clients.tail);

}

static struct client *findclient(struct view *view, unsigned int x, unsigned int y)
{

    struct list_item *current;

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        if (box_isinside(&client->window.size, x, y))
            return client;

    }

    return 0;

}

static void mapclient(unsigned int source, struct view *view, unsigned int newsource)
{

    struct client *client;

    if (!clients.head)
        return;

    client = clients.head->data;
    client->source = newsource;

    list_move(&view->clients, &clients, &client->item);

    view->clientfocus = focusclient(source, view->clientfocus, client);

}

static void unmapclient(unsigned int source, struct view *view)
{

    if (!view->clientfocus)
        return;

    list_move(&clients, &view->clients, &view->clientfocus->item);

    view->clientfocus = focusclient(source, view->clientfocus, (view->clients.tail) ? view->clients.tail->data : 0);

}

static void activateview(unsigned int source, struct view *view)
{

    struct list_item *current;

    view->panel.active = 1;
    view->number.type = ELEMENT_TEXTTYPE_HIGHLIGHT;

    writepanel(source, 1, &view->panel);
    writetext(source, 1, &view->number, 1, view->numberstring);

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        writewindow(source, 1, &client->window);

    }

}

static void deactivateview(unsigned int source, struct view *view)
{

    struct list_item *current;

    view->panel.active = 0;
    view->number.type = ELEMENT_TEXTTYPE_NORMAL;

    writepanel(source, 1, &view->panel);
    writetext(source, 1, &view->number, 1, view->numberstring);

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        writewindow(source, 0, &client->window);

    }

}

static struct view *focusview(unsigned int source, struct view *focus, struct view *new)
{

    if (focus == new)
        return new;

    if (focus)
        deactivateview(source, focus);

    if (new)
        activateview(source, new);

    return new;

}

static struct view *findview(unsigned int x, unsigned int y)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        if (box_isinside(&view[i].panel.size, x, y))
            return &view[i];

    }

    return 0;

}

static void onkeypress(union event *event)
{

    switch (event->keypress.scancode)
    {

    case 0x38:
        actionkey = 1;

        break;

    }

    if (!actionkey)
    {

        if (viewfocus->clientfocus)
            send_keypress(CALL_L2, viewfocus->clientfocus->source, event->keypress.scancode);

        return;

    }

    switch (event->keypress.scancode)
    {

    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
        viewfocus = focusview(event->header.destination, viewfocus, &view[event->keypress.scancode - 0x02]);

        break;

    case 0x10:
        if (viewfocus->clientfocus)
        {

            send_wmunmap(CALL_L2, viewfocus->clientfocus->source);
            unmapclient(event->header.destination, viewfocus);
            arrangeclients(event->header.destination, viewfocus);

        }

        break;

    case 0x19:
        call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
        call_spawn();

        break;

    case 0x1C:
        if (viewfocus->clientfocus)
        {

            list_move(&viewfocus->clients, &viewfocus->clients, &viewfocus->clientfocus->item);
            arrangeclients(event->header.destination, viewfocus);

        }

        break;

    case 0x23:
        viewfocus->center -= (body.w / 32);

        arrangeclients(event->header.destination, viewfocus);

        break;

    case 0x24:
        viewfocus->clientfocus = nextclient(event->header.destination, viewfocus);

        break;

    case 0x25:
        viewfocus->clientfocus = prevclient(event->header.destination, viewfocus);

        break;

    case 0x26:
        viewfocus->center += (body.w / 32);

        arrangeclients(event->header.destination, viewfocus);

        break;

    case 0x2C:
        send_wmunmap(CALL_L2, 0);

        break;

    }

}

static void onkeyrelease(union event *event)
{

    switch (event->keyrelease.scancode)
    {

    case 0x38:
        actionkey = 0;

        break;

    }

    if (!actionkey)
    {

        if (viewfocus->clientfocus)
            send_keyrelease(CALL_L2, viewfocus->clientfocus->source, event->keyrelease.scancode);

        return;

    }

}

static void onmousepress(union event *event)
{

    struct view *view = findview(mouse.x, mouse.y);
    struct client *client = findclient(viewfocus, mouse.x, mouse.y);

    switch (event->mousepress.button)
    {

    case 0x01:
        if (view)
            viewfocus = focusview(event->header.destination, viewfocus, view);

        if (client)
            viewfocus->clientfocus = focusclient(event->header.destination, viewfocus->clientfocus, client);

        break;

    }

}

static void onmousemove(union event *event)
{

    mouse.x += event->mousemove.relx;
    mouse.y -= event->mousemove.rely;

    if (event->mousemove.relx > 0 && mouse.x >= screen.w)
        mouse.x = screen.w - 1;

    if (event->mousemove.relx < 0 && mouse.x >= screen.w)
        mouse.x = 0;

    if (event->mousemove.rely < 0 && mouse.y >= screen.h)
        mouse.y = screen.h - 1;

    if (event->mousemove.rely > 0 && mouse.y >= screen.h)
        mouse.y = 0;

    writemouse(event->header.destination, 3, &mouse);

}

static void onwmmap(union event *event)
{

    if (event->header.source == event->header.destination)
    {

        ctrl_setvideosettings(&settings, 1920, 1080, 32);
        video_getmode(CALL_L0, &oldsettings);
        video_setmode(CALL_L0, &settings);
        video_getmode(CALL_L0, &settings);
        send_wmresize(CALL_L2, event->header.destination, 0, 0, settings.w, settings.h);

    }
            
    else
    {

        mapclient(event->header.destination, viewfocus, event->header.source);
        arrangeclients(event->header.destination, viewfocus);

    }

}

static void onwmunmap(union event *event)
{

    unsigned int i;

    if (event->header.source == event->header.destination)
        video_setmode(CALL_L0, &oldsettings);

    for (i = 0; i < VIEWS; i++)
    {

        while (view[i].clients.count)
        {

            struct client *client = view[i].clients.head->data;

            send_wmunmap(CALL_L2, client->source);
            list_move(&clients, &view[i].clients, &client->item);

        }

    }

    quit = 1;

}

static void onwmresize(union event *event)
{

    unsigned int i;

    box_setsize(&screen, event->wmresize.x, event->wmresize.y, event->wmresize.w, event->wmresize.h);
    box_setsize(&menu, screen.x, screen.y, screen.w, 32);
    box_setsize(&body, screen.x, screen.y + menu.h, screen.w, screen.h - menu.h);
    box_setsize(&background.size, screen.x, screen.y, screen.w, screen.h);
    writefill(event->header.destination, 1, &background);

    for (i = 0; i < VIEWS; i++)
    {

        view[i].center = body.w / 2;

        box_setsize(&view[i].panel.size, menu.x + i * menu.w / VIEWS, menu.y, menu.w / VIEWS, menu.h);
        writepanel(event->header.destination, 1, &view[i].panel);
        box_setsize(&view[i].number.size, view[i].panel.size.x + menu.h / 4, view[i].panel.size.y + menu.h / 4, view[i].panel.size.w - menu.h / 2, view[i].panel.size.h - menu.h / 2);
        writetext(event->header.destination, 1, &view[i].number, 1, view[i].numberstring);
        arrangeclients(event->header.destination, &view[i]);

    }

    mouse.x = screen.x + screen.w / 4;
    mouse.y = screen.y + screen.h / 4;

    writemouse(event->header.destination, 3, &mouse);

}

static void setup(void)
{

    unsigned int i;

    element_initfill(&background, 2);
    element_initmouse(&mouse);

    for (i = 0; i < CLIENTS; i++)
    {

        list_inititem(&client[i].item, &client[i]);
        element_initwindow(&client[i].window);
        list_add(&clients, &client[i].item);

    }

    for (i = 0; i < VIEWS; i++)
    {

        list_inititem(&view[i].item, &view[i]);
        element_initpanel(&view[i].panel);
        element_inittext(&view[i].number, ELEMENT_TEXTTYPE_NORMAL);

        view[i].numberstring = "12345678" + i;
        view[i].clientfocus = 0;

    }

    viewfocus = &view[0];
    viewfocus->panel.active = 1;
    viewfocus->number.type = ELEMENT_TEXTTYPE_HIGHLIGHT;

}

void main(void)
{

    union event event;
    unsigned int count;

    setup();

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_MOUSEPRESS] = onmousepress;
    handlers[EVENT_MOUSEMOVE] = onmousemove;
    handlers[EVENT_WMMAP] = onwmmap;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;

    call_open(CALL_PO);
    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);
    send_wmmap(CALL_L2, 0);

    while ((count = call_read(CALL_L1, sizeof (struct event_header), &event.header)))
    {

        if (event.header.count)
            call_read(CALL_L1, event.header.count, event.data + sizeof (struct event_header));

        if (handlers[event.header.type])
        {

            handlers[event.header.type](&event);
            flush();

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

