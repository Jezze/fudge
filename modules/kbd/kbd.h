struct kbd_interface
{

    struct base_driver *driver;
    unsigned int (*read_data)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned char keymap[256];

};

struct kbd_protocol
{

    char *name;

};

void kbd_register_interface(struct kbd_interface *interface);
void kbd_register_protocol(struct kbd_protocol *protocol);
void kbd_unregister_interface(struct kbd_interface *interface);
void kbd_unregister_protocol(struct kbd_protocol *protocol);
void kbd_init_interface(struct kbd_interface *interface, struct base_driver *driver, unsigned int (*read_data)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer));
void kbd_init_protocol(struct kbd_protocol *protocol, char *name);
