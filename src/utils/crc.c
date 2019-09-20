#include <fudge.h>
#include <abi.h>

static struct crc s;

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    crc_read(&s, mdata, msize);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        crc_read(&s, buffer, count);

    file_close(file->descriptor);

}

static void onstop(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);
    unsigned char buffer[FUDGE_BSIZE];

    channel_append(channel, ascii_wvalue(buffer, 32, crc_finalize(&s), 10), buffer);
    channel_append(channel, 1, "\n");
    channel_place(channel, id);
    channel_exit(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_STOP, onstop);
    channel_listen(&channel);

}

