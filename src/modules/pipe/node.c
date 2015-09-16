#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct pipe pipe;

void module_init(void)
{

    pipe_init(&pipe);

}

void module_register(void)
{

    pipe_register(&pipe);

}

void module_unregister(void)
{

    pipe_unregister(&pipe);

}

