#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = option_getdecimal("service");
    unsigned int id = fsp_walk(service, 0, "data/help.txt");

    if (id)
    {

        char buffer[MESSAGE_SIZE];
        unsigned int count;
        unsigned int offset;

        for (offset = 0; (count = fsp_read(service, id, buffer, MESSAGE_SIZE, offset)); offset += count)
            channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    option_add("service", "90");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

