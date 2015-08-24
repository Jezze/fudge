struct mouse_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;

};

void mouse_notify(struct mouse_interface *interface, unsigned int count, void *buffer);
void mouse_notifymousepress(struct mouse_interface *interface, unsigned int button);
void mouse_notifymousemove(struct mouse_interface *interface, char relx, char rely);
void mouse_registerinterface(struct mouse_interface *interface, unsigned int id);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_initinterface(struct mouse_interface *interface);
