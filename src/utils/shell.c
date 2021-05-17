#include <fudge.h>
#include <abi.h>

static char inputbuffer[BUFFER_SIZE];
static struct ring input;

static void print(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, buffer, count);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void check(struct channel *channel, void *mdata, struct job *jobs, unsigned int njobs)
{

    struct event_consoledata *consoledata = mdata;

    job_send(channel, jobs, njobs, EVENT_CONSOLEDATA, 1, &consoledata->data);

    switch (consoledata->data)
    {

    case 'q':
        job_send(channel, jobs, njobs, EVENT_TERM, 0, 0);

        break;

    case 'Q':
        job_kill(channel, jobs, njobs);

        break;

    }

}

static void runcommand(struct channel *channel, unsigned int count, void *buffer)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        struct message_header header;
        struct message_data data;
        struct job jobs[32];
        unsigned int njobs = 0;
        unsigned int tasks;
        unsigned int c;

        channel_sendredirectback(channel, id, EVENT_DATA);
        channel_sendredirectback(channel, id, EVENT_CLOSE);
        channel_sendbuffer(channel, id, EVENT_DATA, count, buffer);
        channel_send(channel, id, EVENT_MAIN);

        while ((c = channel_readsource(channel, id, data.buffer, MESSAGE_SIZE)))
        {

            unsigned int n = job_parse(jobs, 32, data.buffer, c);

            njobs = job_spawn(channel, jobs, n);

        }

        job_pipe(channel, jobs, njobs);

        tasks = job_run(channel, jobs, njobs);

        while (tasks && channel_poll(channel, &header, &data))
        {

            switch (header.event)
            {

            case EVENT_CLOSE:
                tasks = job_close(channel, header.source, jobs, njobs);

                break;

            case EVENT_CONSOLEDATA:
                check(channel, data.buffer, jobs, njobs);

                break;

            default:
                channel_dispatch(channel, &header, &data);

                break;

            }

        }

    }

}

static void interpret(struct channel *channel, struct ring *ring)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = ring_read(ring, buffer, BUFFER_SIZE);

    if (count >= 2)
        runcommand(channel, count, buffer);

    printprompt();

}

static void complete(struct channel *channel, struct ring *ring)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = ring_read(ring, buffer, BUFFER_SIZE);
    unsigned int id = file_spawn("/bin/complete");

    if (id)
    {

        channel_sendredirectback(channel, id, EVENT_DATA);
        channel_sendredirectback(channel, id, EVENT_CLOSE);
        channel_sendbuffer(channel, id, EVENT_DATA, count, buffer);
        channel_send(channel, id, EVENT_MAIN);

        while (channel_readsource(channel, id, buffer, BUFFER_SIZE));

    }

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(channel, &input);

        break;

    case '\b':
    case 0x7F:
        if (!ring_skipreverse(&input, 1))
            break;

        print("\b \b", 3);

        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        print(&consoledata->data, 1);
        ring_write(&input, &consoledata->data, 1);
        interpret(channel, &input);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        print(&consoledata->data, 1);

        break;

    }

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    print(mdata, msize);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    printprompt();
    file_link(FILE_G0);

    while (channel_process(channel));

    file_unlink(FILE_G0);

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "console"))
    {

        if (file_walk2(FILE_L0, value))
        {

            file_walk(FILE_G0, FILE_L0, "event");
            file_walk(FILE_G1, FILE_L0, "transmit");

        }

    }

}

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        file_duplicate(FILE_PW, FILE_L0);
        file_duplicate(FILE_CW, FILE_L0);

    }

}

void init(struct channel *channel)
{

    ring_init(&input, BUFFER_SIZE, inputbuffer);
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);
    channel_setcallback(channel, EVENT_PATH, onpath);

}

