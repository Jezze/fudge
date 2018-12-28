#include <fudge.h>
#include <abi.h>

static unsigned char inputbuffer[FUDGE_BSIZE];
static struct ring input;

static void printprompt(void)
{

    file_writeall(FILE_G1, "$ ", 2);

}

static void printnormal(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, "\b\b  \b\b", 6);
    file_writeall(FILE_G1, buffer, count);
    printprompt();

}

static void printcomplete(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, buffer, count);
    printprompt();

}

static unsigned int runcmd(struct event_channel *channel, char *command, char *data, unsigned int count, unsigned int session)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, command))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_request(channel, EVENT_INIT, session);
        event_place(id, channel);
        event_request(channel, EVENT_DATA, session);
        event_append(&channel->o, count, data);
        event_place(id, channel);
        event_request(channel, EVENT_STOP, session);
        event_place(id, channel);

    }

    return id;

}

static unsigned int interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "cd ", 3))
    {

        command[count - 1] = '\0';

        if (file_walk2(FILE_L0, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static unsigned int interpret(struct event_channel *channel, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    if (count < 2)
        return 0;

    if (interpretbuiltin(count, command))
        return 0;

    return runcmd(channel, "/bin/slang", command, count, 2);

}

static unsigned int complete(struct event_channel *channel, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    return runcmd(channel, "/bin/complete", command, count, 1);

}

static unsigned int onconsoledata(struct event_channel *channel)
{

    struct event_consoledata *consoledata = event_getdata(channel);

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

        file_writeall(FILE_G1, "\b \b", 3);

        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        file_writeall(FILE_G1, &consoledata->data, 1);
        ring_write(&input, &consoledata->data, 1);
        interpret(channel, &input);
        printprompt();

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

    return 0;

}

static unsigned int ondata(struct event_channel *channel)
{

    struct job jobs[32];

    switch (channel->i.header.session)
    {

    case 0:
        printnormal(event_getdata(channel), event_getdatasize(channel));

        break;

    case 1:
        printcomplete(event_getdata(channel), event_getdatasize(channel));

        break;

    case 2:
        job_interpret(jobs, 32, channel, event_getdata(channel), event_getdatasize(channel), 0);

        break;

    }

    return 0;

}

static unsigned int oninit(struct event_channel *channel)
{

    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    printprompt();

    return 0;

}

static unsigned int onkill(struct event_channel *channel)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    if (!file_walk(FILE_G0, FILE_P0, "event"))
        return;

    if (!file_walk(FILE_G1, FILE_P0, "odata"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);

    while (!status)
    {

        switch (event_pick(&channel))
        {

        case EVENT_CONSOLEDATA:
            status = onconsoledata(&channel);

            break;

        case EVENT_DATA:
            status = ondata(&channel);

            break;

        case EVENT_INIT:
            status = oninit(&channel);

            break;

        case EVENT_KILL:
            status = onkill(&channel);

            break;

        }

    }

    file_close(FILE_G1);
    file_close(FILE_G0);

}

