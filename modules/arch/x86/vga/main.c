#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "registers.h"
#include "timing.h"

#define VGA_TEXT_LIMIT                  2000
#define VGA_COLORMAP_LIMIT              256

struct vga_character
{

    char character;
    char color;

};

static struct base_driver driver;
static struct base_terminal_interface iterminal;
static struct base_video_interface ivideo;

static struct instance
{

    struct base_device device;
    struct base_terminal_node tnode;
    struct base_video_node vnode;
    struct {unsigned char color; unsigned short offset;} cursor;
    struct {unsigned int x; unsigned int y; unsigned int bpp;} resolution;
    void *taddress;
    void *gaddress;

} instances[2];

static struct instance *find_instance(struct base_bus *bus, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < 2; i++)
    {

        struct instance *instance = &instances[i];

        if (instance->device.bus == bus && instance->device.id == id)
            return instance;

    }

    return 0;

}

/* BIOS mode 0Dh - 320x200x16 */
/*
static const unsigned char g320x200x16[60] = {
    0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80, 0xBF, 0x1F, 0x00, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x14, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x09, 0x0F, 0x00, 0x06,
    0x63
};
*/

/* BIOS mode 0Eh - 640x200x16 */
/*
static const unsigned char g640x200x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0x63
};
*/

/* BIOS mode 10h - 640x350x16 */
/*
static const unsigned char g640x350x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x83, 0x85, 0x5D, 0x28, 0x0F, 0x63, 0xBA, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA3
};
*/

/* BIOS mode 12h - 640x480x16 */
/*
static const unsigned char g640x480x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE3
};
*/

/* BIOS mode 13h - 320x200x256 */
static const unsigned char g320x200x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    0x63
};

/* non-BIOS mode - 320x240x256 */
/*
static const unsigned char g320x240x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0D, 0x3E, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0xAC, 0xDF, 0x28, 0x00, 0xE7, 0x06, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE3
};
*/

/* non-BIOS mode - 320x400x256 */
/*
static const unsigned char g320x400x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0x63
};
*/

/* non-BIOS mode - 360x480x256 */
/*
static const unsigned char g360x480x256[60] = {
    0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0x0D, 0x3E, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0xAC, 0xDF, 0x2D, 0x00, 0xE7, 0x06, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE7
};
*/

/* non BIOS mode - 720x348x2 based on mode 10h */
/*
static const unsigned char g720x348x2[60] = {
    0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x83, 0x85, 0x5D, 0x2D, 0x0F, 0x63, 0xBA, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA7
};
*/

/* non-BIOS mode - 400x300x256 */
/*
static const unsigned char g400x300x256X[60] = {
    0x71, 0x63, 0x64, 0x92, 0x65, 0x82, 0x46, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x31, 0x80, 0x2B, 0x32, 0x00, 0x2F, 0x44, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA7
};
*/

static void clear(struct base_bus *bus, unsigned int id, unsigned int offset)
{

    struct instance *instance = find_instance(bus, id);
    struct vga_character *memory = instance->taddress;
    unsigned int i;

    for (i = offset; i < VGA_TEXT_LIMIT; i++)
    {

        memory[i].character = ' ';
        memory[i].color = instance->cursor.color;

    }

}

static void set_mode(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp)
{

    io_inb(VGA_REGISTER_FCCCTRL);
    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_DISABLE);

    vga_set_registers((unsigned char *)g320x200x256, 0);

    io_inb(VGA_REGISTER_FCCCTRL);
    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_ENABLE);

}

static unsigned int read_terminal_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_terminal_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct instance *instance = find_instance(bus, id);
    struct vga_character *memory = instance->taddress;
    unsigned int bytespp = instance->resolution.bpp / 8;
    unsigned int linesize = instance->resolution.x * bytespp;
    unsigned int fullsize = instance->resolution.y * linesize;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c = ((char *)buffer)[i];

        if (c == '\b')
            instance->cursor.offset--;

        if (c == '\t')
            instance->cursor.offset = (instance->cursor.offset + 8) & ~(8 - 1);

        if (c == '\r')
            instance->cursor.offset -= (instance->cursor.offset % 80);

        if (c == '\n')
            instance->cursor.offset += 80 - (instance->cursor.offset % 80);

        if (c >= ' ')
        {

            memory[instance->cursor.offset].character = c;
            memory[instance->cursor.offset].color = instance->cursor.color;
            instance->cursor.offset++;

        }

        if (instance->cursor.offset >= VGA_TEXT_LIMIT)
        {

            memory_read(instance->taddress, fullsize, instance->taddress, fullsize, linesize);
            clear(bus, id, 80 * 24);
            instance->cursor.offset -= 80;

        }

    }

    outcrt1(VGA_CRTINDEX_CRT0E, instance->cursor.offset >> 8);
    outcrt1(VGA_CRTINDEX_CRT0F, instance->cursor.offset);

    return count;

}

static unsigned int read_video_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct instance *instance = find_instance(bus, id);
    unsigned int bytespp = instance->resolution.bpp / 8;
    unsigned int linesize = instance->resolution.x * bytespp;
    unsigned int fullsize = instance->resolution.y * linesize;

    return memory_read(buffer, count, instance->gaddress, fullsize, offset);

}

static unsigned int write_video_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct instance *instance = find_instance(bus, id);
    unsigned int bytespp = instance->resolution.bpp / 8;
    unsigned int linesize = instance->resolution.x * bytespp;
    unsigned int fullsize = instance->resolution.y * linesize;

    return memory_write(instance->gaddress, fullsize, buffer, count, offset);

}

static unsigned int read_video_colormap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count; i += 3)
    {

        io_outb(VGA_REGISTER_DACRINDEX, i / 3);
        c[i + 0] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 1] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 2] = io_inb(VGA_REGISTER_DACDATA);

    }

    return i - offset;

}

static unsigned int write_video_colormap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count; i += 3)
    {

        io_outb(VGA_REGISTER_DACWINDEX, i / 3);
        io_outb(VGA_REGISTER_DACDATA, c[i + 0]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 1]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 2]);

    }

    return i - offset;

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_bus_inb(bus, id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_bus_inb(bus, id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_DISPLAY_VGA && pci_bus_inb(bus, id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    struct instance *instance = find_instance(0, 0);

    base_init_device(&instance->device, bus, id);
    base_terminal_init_node(&instance->tnode, &instance->device, &iterminal);
    base_terminal_register_node(&instance->tnode);
    base_video_init_node(&instance->vnode, &instance->device, &ivideo);
    base_video_register_node(&instance->vnode);

    instance->taddress = (void *)0x000B8000;
    instance->gaddress = (void *)0x000A0000;
    instance->resolution.x = 80;
    instance->resolution.y = 25;
    instance->resolution.bpp = 16;
    instance->cursor.color = 0x0F;

    clear(bus, id, 0);

}

static void detach(struct base_bus *bus, unsigned int id)
{

}

void init()
{

    memory_clear(instances, sizeof (struct instance) * 2);
    base_terminal_init_interface(&iterminal, read_terminal_data, write_terminal_data);
    base_terminal_register_interface(&iterminal);
    base_video_init_interface(&ivideo, set_mode, read_video_data, write_video_data, read_video_colormap, write_video_colormap);
    base_video_register_interface(&ivideo);
    base_init_driver(&driver, "vga", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_terminal_unregister_interface(&iterminal);
    base_video_unregister_interface(&ivideo);
    base_unregister_driver(&driver);

}

