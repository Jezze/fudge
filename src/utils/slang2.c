#include <abi.h>
#include <fudge.h>

#define TOKENSKIP                       1
#define TOKENEND                        2
#define TOKENIDENT                      3
#define TOKENIN                         4
#define TOKENOUT                        5
#define TOKENPIPE                       6

struct token
{

    unsigned int type;
    char *str;

};

struct tokenlist
{

    unsigned int head;
    unsigned int size;
    struct token *table;

};

static char stringdata[FUDGE_BSIZE];
static struct ring stringtable;
static struct token infixdata[1024];
static struct token postfixdata[1024];
static struct token stackdata[8];
static struct tokenlist infix;
static struct tokenlist postfix;
static struct tokenlist stack;

static void tokenlist_init(struct tokenlist *list, unsigned int size, struct token *table)
{

    list->head = 0;
    list->size = size;
    list->table = table;

}

static void tokenlist_push(struct tokenlist *list, struct token *token)
{

    list->table[list->head].type = token->type;
    list->table[list->head].str = token->str;

    if (list->head < list->size)
        list->head++;

}

static struct token *tokenlist_pop(struct tokenlist *list)
{

    if (!list->head)
        return 0;

    list->head--;

    return &list->table[list->head];

}

static void tokenlist_add(struct tokenlist *list, unsigned int type, char *str)
{

    struct token token;

    token.type = type;
    token.str = str;

    tokenlist_push(list, &token);

}

static unsigned int precedence(struct token *token)
{

    switch (token->type)
    {

    case TOKENEND:
        return 1;

    case TOKENPIPE:
        return 2;

    case TOKENIN:
    case TOKENOUT:
        return 3;

    }

    return 0;

}

static unsigned int tokenize(char c)
{

    switch (c)
    {

    case ' ':
    case '\t':
        return TOKENSKIP;

    case '<':
        return TOKENIN;

    case '>':
        return TOKENOUT;

    case '|':
        return TOKENPIPE;

    case ';':
    case '\n':
        return TOKENEND;

    }

    return TOKENIDENT;

}

static unsigned int getidentlength(unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(buffer[i]);

        if (token != TOKENIDENT)
            break;

    }

    return i;

}

static void tokenizebuffer(struct tokenlist *infix, struct ring *stringtable, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;
    unsigned int c;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(data[i]);

        switch (token)
        {

        case TOKENSKIP:
            continue;

        case TOKENIDENT:
            tokenlist_add(infix, token, stringtable->buffer + ring_count(stringtable));

            c = getidentlength(count - i, data + i);

            ring_write(stringtable, &data[i], c);
            ring_write(stringtable, "\0", 1);

            i += c - 1;

            break;

        default:
            tokenlist_add(infix, token, 0);

            break;

        }

    }

}

static void translate(struct tokenlist *postfix, struct tokenlist *infix, struct tokenlist *stack)
{

    unsigned int i;

    for (i = 0; i < infix->head; i++)
    {

        struct token *token = &infix->table[i];
        struct token *t;

        if (token->type == TOKENIDENT)
        {

            tokenlist_push(postfix, token);

            continue;

        }

        if (token->type == TOKENEND || token->type == TOKENPIPE)
        {

            while ((t = tokenlist_pop(stack)))
                tokenlist_push(postfix, t);

            tokenlist_push(postfix, token);

            continue;

        }

        while ((t = tokenlist_pop(stack)))
        {

            if (precedence(token) > precedence(t))
            {

                tokenlist_push(stack, t);

                break;

            }

            tokenlist_push(postfix, t);

        }

        tokenlist_push(stack, token);

    }

}

static void parse(struct event_header *iheader, struct event_header *oheader, struct tokenlist *postfix, struct tokenlist *stack, unsigned int session)
{

    unsigned int i;

    event_replydatapipe(oheader, iheader, session);

    for (i = 0; i < postfix->head; i++)
    {

        struct token *token = &postfix->table[i];
        struct token *t;

        switch (token->type)
        {

        case TOKENIDENT:
            tokenlist_push(stack, token);

            break;

        case TOKENIN:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            event_appenddata(oheader, 1, "I");
            event_appenddata(oheader, ascii_length(t->str) + 1, t->str);

            break;

        case TOKENOUT:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            event_appenddata(oheader, 1, "O");
            event_appenddata(oheader, ascii_length(t->str) + 1, t->str);

            break;

        case TOKENPIPE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            event_appenddata(oheader, 1, "P");
            event_appenddata(oheader, ascii_length(t->str) + 1, t->str);

            break;

        case TOKENEND:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            event_appenddata(oheader, 1, "E");
            event_appenddata(oheader, ascii_length(t->str) + 1, t->str);
            event_send(oheader);
            event_replydatapipe(oheader, iheader, session);

            break;

        }

    }

}

static unsigned int ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!datafile->descriptor)
        return 0;

    file_open(datafile->descriptor);

    while ((count = file_read(datafile->descriptor, buffer, FUDGE_BSIZE)))
        tokenizebuffer(&infix, &stringtable, count, buffer);

    file_close(datafile->descriptor);
    translate(&postfix, &infix, &stack);
    parse(iheader, oheader, &postfix, &stack, datafile->session);

    return 0;

}

static unsigned int ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    if (!datapipe->count)
        return 0;

    tokenizebuffer(&infix, &stringtable, datapipe->count, datapipe + 1);
    translate(&postfix, &infix, &stack);
    parse(iheader, oheader, &postfix, &stack, datapipe->session);

    return 0;

}

static unsigned int ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datastop *datastop = event_getdata(iheader);

    event_replydatastop(oheader, iheader, datastop->session);
    event_send(oheader);

    return 1;

}

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    ring_init(&stringtable, FUDGE_BSIZE, stringdata);
    tokenlist_init(&infix, 1024, infixdata);
    tokenlist_init(&postfix, 1024, postfixdata);
    tokenlist_init(&stack, 8, stackdata);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;

    event_open();

    while (!status)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_DATAFILE:
            status = ondatafile(iheader, oheader);

            break;

        case EVENT_DATAPIPE:
            status = ondatapipe(iheader, oheader);

            break;

        case EVENT_DATASTOP:
            status = ondatastop(iheader, oheader);

            break;

        case EVENT_INIT:
            status = oninit(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();

}
