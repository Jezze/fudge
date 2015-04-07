#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <block/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct block_interface blockinterface;

static void handleirq(unsigned int irq)
{

    unsigned char status = ide_getstatus(blockinterface.id);
    unsigned char data[512];

    if (status & 1)
        return;

    ide_rblock(blockinterface.id, 1, data);
    block_notify(&blockinterface, 512, 1, data);

}

static unsigned int blockinterface_rdata(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    offset = offset / 512;
    count = count / 512;

    ide_rpio28(blockinterface.id, 0, offset, count);

    return 0;

}

static unsigned int blockinterface_wdata(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return 0;

}

static void driver_init()
{

    block_initinterface(&blockinterface, blockinterface_rdata, blockinterface_wdata);

}

static unsigned int driver_match(unsigned int id)
{

    return id == IDE_ATA;

}

static void driver_attach(unsigned int id)
{

    block_registerinterface(&blockinterface, id);
    pic_setroutine(ide_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(ide_getirq(id));

}

void module_init()
{

    base_initdriver(&driver, "ata", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, IDE_BUS);

}

void module_unregister()
{

    base_unregisterdriver(&driver, IDE_BUS);

}

