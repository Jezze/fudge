#include <fudge.h>
#include "option.h"

#define NUMOPTIONS                      32

static struct option options[NUMOPTIONS];

static struct option *find(char *key)
{

    unsigned int i;

    for (i = 0; i < NUMOPTIONS; i++)
    {

        struct option *option = &options[i];

        if (option->key && cstring_match(option->key, key))
            return option;

    }

    return 0;

}

static struct option *findfree(char *key)
{

    unsigned int i;

    for (i = 0; i < NUMOPTIONS; i++)
    {

        struct option *option = &options[i];

        if (!option->key)
            return option;

    }

    return 0;

}

int option_getdecimal(char *key)
{

    struct option *option = find(key);

    return (option) ? cstring_readvalue(option->value, cstring_length(option->value), 10) : 0;

}

char *option_getstring(char *key)
{

    struct option *option = find(key);

    return (option) ? option->value : 0;

}

unsigned int option_set(char *key, char *value)
{

    struct option *option = find(key);

    return (option) ? cstring_write(option->value, 64, value, 0) : 0;

}

void option_add(char *key, char *value)
{

    struct option *option = findfree(key);

    if (option)
        option_init(option, key, value);

}

void option_init(struct option *option, char *key, char *value)
{

    option->key = key;

    cstring_write(option->value, 64, value, 0);

}

