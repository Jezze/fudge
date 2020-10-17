#include <fudge.h>
#include "call.h"
#include "file.h"
#include "channel.h"

static void dispatch(struct channel *channel, struct message_header *header, void *data)
{

    if (header->type < EVENTS)
    {

        if (channel->signals[header->type].callback)
            channel->signals[header->type].callback(channel, header->source, data, message_datasize(header));

        if (channel->signals[EVENT_ANY].callback)
            channel->signals[EVENT_ANY].callback(channel, header->source, data, message_datasize(header));

    }

}

static unsigned int poll(struct channel *channel, struct message_header *header, void *data)
{

    while (channel->poll)
    {

        if (call_pick(header, data))
            return header->type;

    }

    return 0;

}

unsigned int channel_place3(struct channel *channel, unsigned int id, unsigned int type, unsigned int count, void *data)
{

    struct message_header header;

    message_initheader(&header, type, count);

    if (type < EVENTS)
    {

        if (channel->signals[type].redirect)
            id = channel->signals[type].redirect;

        return call_place(id, &header, data);

    }

    return 0;

}

unsigned int channel_place(struct channel *channel, union message *message, unsigned int id)
{

    return channel_place3(channel, id, message->header.type, message->header.length - sizeof (struct message_header), message->data + sizeof (struct message_header));

}

unsigned int channel_poll(struct channel *channel, struct message_header *header, void *data, unsigned int type)
{

    while (poll(channel, header, data))
    {

        if (header->type == type)
            return type;

        dispatch(channel, header, data);

    }

    return 0;

}

void channel_listen(struct channel *channel, void (*oninit)(struct channel *channel), void (*onexit)(struct channel *channel))
{

    struct message_header header;
    char data[FUDGE_BSIZE];

    if (oninit)
        oninit(channel);

    channel->poll = 1;

    while (poll(channel, &header, data))
        dispatch(channel, &header, data);

    if (onexit)
        onexit(channel);

}

void channel_close(struct channel *channel)
{

    channel->poll = 0;

}

void channel_setredirect(struct channel *channel, unsigned int type, unsigned int mode, unsigned int id, unsigned int source)
{

    switch (mode)
    {

    case 1:
        channel->signals[type].redirect = id;

        break;

    case 2:
        channel->signals[type].redirect = source;

        break;

    default:
        channel->signals[type].redirect = 0;

        break;

    }

}

void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, unsigned int source, void *mdata, unsigned int msize))
{

    channel->signals[type].callback = callback;

}

void channel_init(struct channel *channel)
{

    unsigned int i;

    for (i = 0; i < EVENTS; i++)
    {

        channel_setsignal(channel, i, 0);
        channel_setredirect(channel, i, 0, 0, 0);

    }

}

void channel_sendredirect(struct channel *channel, unsigned int id, unsigned int type, unsigned int mode, unsigned int source)
{

    struct event_redirect redirect;

    redirect.type = type;
    redirect.mode = mode;
    redirect.id = source;

    channel_place3(channel, id, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

