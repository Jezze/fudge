enum resource_type
{

    RESOURCE_TYPE_ALL                   = 1,
    RESOURCE_TYPE_CONTAINER             = 2,
    RESOURCE_TYPE_TASK                  = 3,
    RESOURCE_TYPE_VFS                   = 5,
    RESOURCE_TYPE_VFSBACKEND            = 6,
    RESOURCE_TYPE_VFSPROTOCOL           = 7,
    RESOURCE_TYPE_BINARY                = 8,
    RESOURCE_TYPE_BINARYPROTOCOL        = 9

};

struct resource_id
{

    enum resource_type type;
    unsigned int size;
    const char *text;

};

struct resource_item
{

    struct list_item item;
    struct resource_id id;

};

struct resource_iterator
{

    struct list_item item;
    unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer);

};

struct resource_item *resource_find_item(enum resource_type type);
struct resource_iterator *resource_find_iterator(enum resource_type type);
void resource_register_item(struct resource_item *item);
void resource_register_iterator(struct resource_iterator *iterator);
void resource_init_item(struct resource_item *item, void *data, enum resource_type type, unsigned int size, const char *text);
void resource_init_iterator(struct resource_iterator *iterator, unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer));
void resource_setup();
