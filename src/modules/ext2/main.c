#include <fudge/module.h>
#include <base/base.h>
#include <base/block.h>
#include <block/block.h>
#include "ext2.h"

static struct ext2_protocol protocol;

void module_init(void)
{

    ext2_init_protocol(&protocol);

}

void module_register(void)
{

    block_register_protocol(&protocol.base);

}

void module_unregister(void)
{

    block_unregister_protocol(&protocol.base);

}

