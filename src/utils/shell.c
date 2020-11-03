#include <fudge.h>
#include <abi.h>

#define MODE_NORMAL                     0
#define MODE_WAITING                    1

static unsigned int keymod = KEYMOD_NONE;
static char inputbuffer[FUDGE_BSIZE];
static struct ring input;
static unsigned int mode = MODE_NORMAL;
static unsigned int desc = FILE_G0;

static void printprompt(void)
{

    file_writeall(FILE_G1, "$ ", 2);

}

static unsigned int interpretbuiltin(unsigned int count, char *data)
{

    if (count < 2)
        return 1;

    if (memory_match(data, "cd ", 3))
    {

        data[count - 1] = '\0';

        if (file_walk2(FILE_L0, data + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static void interpret(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_MSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_MSIZE);
    unsigned int id;

    if (interpretbuiltin(count, data))
        return;

    id = job_simple(channel, "/bin/slang", count, data);

    if (id)
    {

        struct message_header header;
        struct job_status status;
        struct job jobs[32];
        unsigned int njobs = 0;

        while (channel_pollsource(channel, id, &header, data))
        {

            if (header.type == EVENT_CLOSE)
                break;

            if (header.type == EVENT_DATA)
            {

                status.start = data;
                status.end = status.start + message_datasize(&header);

                while (status.start < status.end)
                {

                    njobs = job_parse(&status, jobs, 32);

                    job_run(channel, jobs, njobs);

                }

            }

        }

        mode = MODE_WAITING;

        job_wait(channel, jobs, njobs);

        mode = MODE_NORMAL;

    }

}

static void complete(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_MSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_MSIZE);
    unsigned int id;

    id = job_simple(channel, "/bin/complete", count, data);

    if (id)
    {

        struct message_header header;

        while (channel_pollsource(channel, id, &header, data))
        {

            if (header.type == EVENT_CLOSE)
                break;

            if (header.type == EVENT_DATA)
                file_writeall(FILE_G1, data, message_datasize(&header));

        }

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
        if (mode == MODE_NORMAL)
        {

            complete(channel, &input);
            printprompt();

        }

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

        if (mode == MODE_NORMAL)
        {

            interpret(channel, &input);
            printprompt();

        }

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    printprompt();

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_writeall(FILE_G1, mdata, msize);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(desc, mdata))
        return;

    file_open(desc);

    desc++;

}

static void onkeypress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keypress *keypress = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, keymod);

    keymod = keymap_modkey(keypress->scancode, keymod);

    switch (keypress->scancode)
    {

    case 0x0E:
        if (!ring_skipreverse(&input, 1))
            break;

        file_writeall(FILE_G1, "\b \b", 3);

        break;

    case 0x0F:
        if (mode == MODE_NORMAL)
        {

            complete(channel, &input);
            printprompt();

        }

        break;

    case 0x1C:
        file_writeall(FILE_G1, keycode->value, keycode->length);
        ring_write(&input, keycode->value, keycode->length);

        if (mode == MODE_NORMAL)
        {

            interpret(channel, &input);
            printprompt();

        }

        break;

    default:
        ring_write(&input, keycode->value, keycode->length);
        file_writeall(FILE_G1, keycode->value, keycode->length);

        break;

    }

}

static void onkeyrelease(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keymod = keymap_modkey(keyrelease->scancode, keymod);

}

static void oninit(struct channel *channel)
{

    ring_init(&input, FUDGE_BSIZE, inputbuffer);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_CONSOLEDATA, onconsoledata);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_KEYPRESS, onkeypress);
    channel_setsignal(&channel, EVENT_KEYRELEASE, onkeyrelease);
    channel_listen(&channel, oninit, 0);

}

