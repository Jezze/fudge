#include <fudge.h>
#include <abi.h>

static void list(struct channel *channel, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);
    channel_reply(channel, EVENT_DATA);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char num[FUDGE_NSIZE];

        if (event_avail(&channel->o.header) < record.length + 3 + 16)
        {

            channel_place(channel->o.header.target, &channel->o);
            event_reset(&channel->o.header);

        }

        event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, record.id, 16, 8, 0), num);
        event_append(&channel->o.header, 1, " ");
        event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, record.size, 16, 8, 0), num);
        event_append(&channel->o.header, 1, " ");
        event_append(&channel->o.header, record.length, record.name);
        event_append(&channel->o.header, 1, "\n");

        if (!file_step(descriptor))
            break;

    }

    channel_place(channel->o.header.target, &channel->o);
    file_close(descriptor);

}

static void onempty(struct channel *channel, void *mdata, unsigned int msize)
{

    list(channel, FILE_PW);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;

    list(channel, file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

