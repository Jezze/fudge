#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/arch/x86/pci/pci.h>

#define REG_COMMAND                     0x00
#define REG_STATUS                      0x04
#define REG_INTERRUPT                   0x08
#define REG_FRAMEINDEX                  0x0C
#define REG_SEGMENT                     0x10
#define REG_LISTBASE                    0x14
#define REG_LISTNEXT                    0x18
#define REG_FLAG                        0x40

static struct base_driver driver;

static void driver_init(unsigned int id)
{

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_SERIAL_USB && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x20;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

}

static void driver_detach(unsigned int id)
{

}

void module_init(void)
{

    base_initdriver(&driver, "ehci", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

