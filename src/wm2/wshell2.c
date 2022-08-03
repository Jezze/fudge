#include <fudge.h>
#include <abi.h>

static char inputdata1[128];
static struct ring input1;
static char inputdata2[128];
static struct ring input2;
static char resultdata[2048];
static struct ring result;

static void update(void)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    count = 0;
    count += buffer_write(buffer, BUFFER_SIZE, "= input1 content \"", 18, count);
    count += ring_readcopy(&input1, buffer + count, BUFFER_SIZE - count);
    count += buffer_write(buffer, BUFFER_SIZE, "\"\n", 2, count);
    count += buffer_write(buffer, BUFFER_SIZE, "= input2 content \"", 18, count);
    count += ring_readcopy(&input2, buffer + count, BUFFER_SIZE - count);
    count += buffer_write(buffer, BUFFER_SIZE, "\"\n", 2, count);

    file_notify(FILE_G0, EVENT_WMRENDERDATA, count, buffer);

    count = 0;
    count += buffer_write(buffer, BUFFER_SIZE, "= result content \"", 18, count);
    count += ring_readcopy(&result, buffer + count, BUFFER_SIZE - count);
    count += buffer_write(buffer, BUFFER_SIZE, "\"\n", 2, count);

    file_notify(FILE_G0, EVENT_WMRENDERDATA, count, buffer);

}

static void print(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ring_avail(&result))
            ring_skip(&result, ring_find(&result, '\n') + 1);

        if (b[i] != '\r')
            ring_write(&result, &b[i], 1);

    }

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void moveleft(unsigned int steps)
{

    char buffer[128];

    ring_writereverse(&input2, buffer, ring_readreverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[128];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

}

static void check(void *mdata, struct job *jobs, unsigned int njobs)
{

    struct event_wmkeypress2 *wmkeypress = mdata;

    switch (wmkeypress->scancode)
    {

    case 0x2E:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            job_send(jobs, njobs, EVENT_TERM, 0, 0);
        else
            job_send(jobs, njobs, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

        break;

    default:
        job_send(jobs, njobs, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

        break;

    }

}

static void runcommand(unsigned int count, void *buffer)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        struct message message;
        struct job jobs[32];
        unsigned int njobs = 0;
        unsigned int tasks;

        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendbufferto(id, EVENT_DATA, count, buffer);
        channel_sendto(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, message.data.buffer, MESSAGE_SIZE)))
        {

            unsigned int n = job_parse(jobs, 32, message.data.buffer, count);

            njobs = job_spawn(jobs, n);

        }

        job_pipe(jobs, njobs);

        tasks = job_run(jobs, njobs);

        while (tasks && channel_pick(&message))
        {

            switch (message.header.event)
            {

            case EVENT_CLOSE:
                tasks = job_close(message.header.source, jobs, njobs);

                break;

            case EVENT_WMKEYPRESS:
                check(message.data.buffer, jobs, njobs);

                break;

            default:
                channel_dispatch(&message);

                break;

            }

        }

    }

}

static void interpret(void)
{

    char buffer[128];
    unsigned int count = ring_read(&input1, buffer, 128);

    printprompt();
    print(buffer, count);

    if (count >= 2)
        runcommand(count, buffer);

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    print(mdata, msize);
    update();

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_CW, mdata))
        file_duplicate(FILE_CW, FILE_L0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Shell\"\n"
        "+ container id \"base\" in \"window\" layout \"maximize\" padding \"16\"\n"
        "+ textbox id \"output\" in \"base\"\n"
        "+ text id \"result\" in \"output\" wrap \"char\"\n"
        "+ text id \"prompt\" in \"output\" wrap \"char\" weight \"bold\" content \"$ \"\n"
        "+ text id \"input1\" in \"output\" wrap \"char\"\n"
        "+ text id \"cursor\" in \"output\" wrap \"char\" mode \"inverted\" content \" \"\n"
        "+ text id \"input2\" in \"output\" wrap \"char\"\n";

    file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data), data);

}

static void onwmkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress2 *wmkeypress = mdata;

    if (!cstring_match(wmkeypress->pressed, "output"))
        return;

    switch (wmkeypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);

        break;

    case 0x0F:
        ring_move(&input1, &input2);
        /* complete(&input1); */

        break;

    case 0x1C:
        ring_move(&input1, &input2);
        ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);
        interpret();

        break;

    case 0x16:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            ring_reset(&input1);
        else
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    case 0x25:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            ring_reset(&input2);
        else
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    case 0x26:
        if (wmkeypress->keymod & KEYMOD_CTRL)
        {

            ring_reset(&result);

        }

        else
        {

            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        }

        break;

    case 0x47:
        moveleft(ring_count(&input1));

        break;

    case 0x48:
        if (wmkeypress->keymod & KEYMOD_SHIFT)
        {
            /*content.offset--;*/
        }

        break;

    case 0x4B:
        moveleft(1);

        break;

    case 0x4D:
        moveright(1);

        break;

    case 0x4F:
        moveright(ring_count(&input2));

        break;

    case 0x50:
        if (wmkeypress->keymod & KEYMOD_SHIFT)
        {
            /*content.offset++;*/
        }

        break;

    default:
        ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    }

    update();

}

void init(void)
{

    ring_init(&input1, 128, inputdata1);
    ring_init(&input2, 128, inputdata2);
    ring_init(&result, 2048, resultdata);

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

