#include <lib/cbuffer.h>
#include <lib/io.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/isr.h>
#include <kernel/irq.h>
#include <kernel/kbd.h>

char kbdMapLowerUS[128] =
{
       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,  '*',    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  '-',    0,    0,    0,  '+',    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

char kbdMapUpperUS[128] =
{
       0,   27,  '!',  ' ',  '#',  ' ',  '%',  '&',  '/',  '(',  ')',  '=',  '-',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '[',  ']', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ';', '\'',  '`',    0, '\\',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  ',',  '.',  '/',    0,  '*',    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  '-',    0,    0,    0,  '+',    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

kbd_device_t keyboard;

struct vfs_node vfsKeyboard;

void kbd_handler(struct isr_registers *registers)
{

    unsigned char scancode = inb(KBD_PORT_READ);

    if (scancode == 0x2A)
        keyboard.toggleShift = 1;

    if (scancode == 0xAA)
        keyboard.toggleShift = 0;

    if (scancode & 0x80)
    {

        // Break codes

    }

    else
    {

        // Make codes
        if (keyboard.toggleShift)
            cbuffer_write(&keyboard.cbuffer, kbdMapUpperUS[scancode]);
        else
            cbuffer_write(&keyboard.cbuffer, kbdMapLowerUS[scancode]);

    }

}

unsigned int keyboard_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    ((char *)buffer)[0] = cbuffer_read(&keyboard.cbuffer);

    return 1;

}

struct vfs_node *kbd_get_node()
{

    string_copy(vfsKeyboard.name, "stdin");
    vfsKeyboard.inode = 0;
    vfsKeyboard.length = 0;
    vfsKeyboard.open = 0;
    vfsKeyboard.close = 0;
    vfsKeyboard.read = keyboard_read;
    vfsKeyboard.write = 0;
    vfsKeyboard.walk = 0;

    return &vfsKeyboard;

}

void kbd_init()
{

    keyboard.cbuffer = cbuffer_create(keyboard.buffer, KBD_BUFFER_SIZE);
    keyboard.toggleAlt = 0;
    keyboard.toggleCtrl = 0;
    keyboard.toggleShift = 0;

}

