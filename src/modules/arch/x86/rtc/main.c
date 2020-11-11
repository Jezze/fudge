#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/clock/clock.h>
#include <modules/arch/x86/platform/platform.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>

#define R_COMMAND                       0x0000
#define R_DATA                          0x0001
#define C_COMMAND_SECONDS               0x00
#define C_COMMAND_MINUTES               0x02
#define C_COMMAND_HOURS                 0x04
#define C_COMMAND_WEEKDAY               0x06
#define C_COMMAND_DAY                   0x07
#define C_COMMAND_MONTH                 0x08
#define C_COMMAND_YEAR                  0x09

static struct base_driver driver;
static struct clock_interface clockinterface;
static unsigned short io;

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char read(unsigned int type)
{

    io_outb(io + R_COMMAND, type);

    return convert(io_inb(io + R_DATA));

}

static void handleirq(unsigned int irq)
{

}

static unsigned int clockinterface_readctrl(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_clocksettings settings;

    settings.seconds = read(C_COMMAND_SECONDS);
    settings.minutes = read(C_COMMAND_MINUTES);
    settings.hours = read(C_COMMAND_HOURS);
    settings.weekday = read(C_COMMAND_WEEKDAY);
    settings.day = read(C_COMMAND_DAY);
    settings.month = read(C_COMMAND_MONTH);
    settings.year = 2000 + read(C_COMMAND_YEAR);

    return memory_read(buffer, count, &settings, sizeof (struct ctrl_clocksettings), offset);

}

static void driver_init(unsigned int id)
{

    clock_initinterface(&clockinterface, id);

    clockinterface.ctrl.operations.read = clockinterface_readctrl;

}

static unsigned int driver_match(unsigned int id)
{

    return id == PLATFORM_RTC;

}

static void driver_reset(unsigned int id)
{

    io = platform_getbase(id);

}

static void driver_attach(unsigned int id)
{

    clock_registerinterface(&clockinterface);
    pic_setroutine(platform_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    clock_unregisterinterface(&clockinterface);
    pic_unsetroutine(platform_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "rtc", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PLATFORM_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PLATFORM_BUS);

}

