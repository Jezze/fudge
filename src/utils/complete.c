#include <fudge.h>
#include <abi.h>

static void complete(struct channel *channel, unsigned int descriptor, void *name, unsigned int length)
{

    struct record record;

    file_open(descriptor);
    channel_reply(channel, EVENT_DATA);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            if (event_avail(&channel->o.header) < record.length + 1)
            {

                channel_place(channel->o.header.target, &channel->o);
                channel_reply(channel, EVENT_DATA);

            }

            event_append(&channel->o.header, record.length, record.name);
            event_append(&channel->o.header, 1, "\n");

        }

        if (!file_step(descriptor))
            break;

    }

    channel_place(channel->o.header.target, &channel->o);
    file_close(descriptor);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    complete(channel, FILE_PW, mdata, msize);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_listen(&channel);

}

