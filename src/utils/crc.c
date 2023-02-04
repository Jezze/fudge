#include <fudge.h>
#include <abi.h>
#include <hash.h>

static struct crc sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    crc_read(&sum, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int crc = crc_finalize(&sum);

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "%u\n", &crc);
    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    if (!file_walk2(FILE_L0, mdata))
        channel_error("File not found");

    while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
        crc_read(&sum, buffer, count);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);

}

