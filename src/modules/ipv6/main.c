#include <fudge.h>
#include <net/ipv6.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>

static struct ethernet_protocol ethernetprotocol;

static void ethernetprotocol_addinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_removeinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    system_write(&ethernetprotocol.data, count, buffer);

}

void module_init(void)
{

    ethernet_initprotocol(&ethernetprotocol, "ipv6", 0x86DD, ethernetprotocol_addinterface, ethernetprotocol_removeinterface, ethernetprotocol_notify);

}

void module_register(void)
{

    ethernet_registerprotocol(&ethernetprotocol);

}

void module_unregister(void)
{

    ethernet_unregisterprotocol(&ethernetprotocol);

}

