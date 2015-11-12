#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

struct client
{

    struct element_text content;
    unsigned int quit;
    unsigned int keymod;
    unsigned char text[FUDGE_BSIZE];
    unsigned int textcount;
    struct box size;

};

static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static void (*handlers[EVENTS])(struct client *client, union event *event);

static void writeelement(unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &element, sizeof (struct element), datacount);

}

static void writetext(unsigned int source, unsigned int z, struct element_text *text, unsigned int count, void *buffer)
{

    writeelement((unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void flush(void)
{

    if (datacount)
    {

        call_write(CALL_PO, datacount, databuffer);

        datacount = 0;

    }

}

static void onkeypress(struct client *client, union event *event)
{

    struct keycode *keycode;

    switch (event->keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        client->keymod |= KEYMOD_SHIFT;

        break;

    case 0x38:
        client->keymod |= KEYMOD_ALT;

        break;

    case 0x0E:
        if (client->textcount)
        {

            client->textcount -= 1;

            writetext(event->header.destination, 1, &client->content, client->textcount, client->text);

        }

        break;

    default:
        keycode = getkeycode(KEYMAP_US, event->keypress.scancode, client->keymod);
        client->textcount += memory_write(client->text, FUDGE_BSIZE, keycode->value, keycode->length, client->textcount);

        writetext(event->header.destination, 1, &client->content, client->textcount, client->text);

        break;
    }

}

static void onkeyrelease(struct client *client, union event *event)
{

    switch (event->keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        client->keymod &= ~KEYMOD_SHIFT;

        break;

    case 0x38:
        client->keymod &= ~KEYMOD_ALT;

        break;

    }

}

static void onwmunmap(struct client *client, union event *event)
{

    writetext(event->header.destination, 0, &client->content, client->textcount, client->text);

    client->quit = 1;

}

static void onwmresize(struct client *client, union event *event)
{

    box_setsize(&client->size, event->wmresize.x, event->wmresize.y, event->wmresize.w, event->wmresize.h);
    box_setsize(&client->content.size, client->size.x + 12, client->size.y + 12, client->size.w - 24, client->size.h - 24);

}

static void onwmshow(struct client *client, union event *event)
{

    writetext(event->header.destination, 1, &client->content, client->textcount, client->text);

}

static void onwmhide(struct client *client, union event *event)
{

    writetext(event->header.destination, 0, &client->content, client->textcount, client->text);

}

static void setup(struct client *client)
{

    element_inittext(&client->content, ELEMENT_TEXTTYPE_NORMAL);

    client->quit = 0;
    client->keymod = KEYMOD_NONE;
    client->textcount = 0;

}

void main(void)
{

    struct client client;
    union event event;
    unsigned int count;

    setup(&client);

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

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

            handlers[event.header.type](&client, &event);
            flush();

        }

        if (client.quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

