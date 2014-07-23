struct base_mouse_interface
{

    struct base_interface base;
    char vx;
    char vy;
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

void base_mouse_connect_interface(struct base_mouse_interface *interface, struct base_bus *bus, unsigned int id);
void base_mouse_register_interface(struct base_mouse_interface *interface);
void base_mouse_unregister_interface(struct base_mouse_interface *interface);
void base_mouse_init_interface(struct base_mouse_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_mouse_setup();
