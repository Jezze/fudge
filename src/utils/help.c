#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn(FILE_L0, option_getstring("echo"));

    if (id)
    {

        channel_listen(id, EVENT_DATA);
        channel_listen(id, EVENT_CLOSE);
        channel_sendfmt0(id, EVENT_PATH, "/data/help.txt\\0");
        channel_send(id, EVENT_MAIN);
        channel_wait(id, EVENT_CLOSE);
        channel_close();

    }

    else
    {

        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Program not found: %s\n", option_getstring("echo"));

    }

}

void init(void)
{

    option_add("echo", "initrd:/bin/echo");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

