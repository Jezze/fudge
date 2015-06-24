struct keyboard_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;

};

void keyboard_notify(struct keyboard_interface *interface, unsigned int size, unsigned int count, void *buffer);
void keyboard_notifykeypress(struct keyboard_interface *interface, unsigned char scancode);
void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id);
void keyboard_unregisterinterface(struct keyboard_interface *interface);
void keyboard_initinterface(struct keyboard_interface *interface);
